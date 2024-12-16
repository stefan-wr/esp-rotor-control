#include <Arduino.h>

#include <globals.h>
#include <Rotation.h>
#include <Adafruit_ADS1X15.h>
#include <Timer.h>

#define ADC_ADDRESS 0x48
#define ADC_CHANNEL 0


namespace Rotor {

    // *****************************
    // Define Rotation class members
    // *****************************

    // => Initialisation
    bool Rotation::init() {
        // Init rotation pins
        pinMode(rot_pins[0], OUTPUT);
        pinMode(rot_pins[1], OUTPUT);
        stopRotor();

        // Init speed pin
        dacWrite(speed_pin, 0);

        // Init ADS1115
        // 16bit = 32768 values
        // GAIN_ONE: +/- 4.096V
        // -> 0.125 mV per ADC value
        adc.setGain(GAIN_ONE);
        if (!adc.begin(ADC_ADDRESS)) {
            Serial.println("[Rotor] Failed to initialise ADS1115!");
            ads_failed = true;
        }

        // Load calibration factors
        loadCalibration();

        return !ads_failed;
    }

    // Calibration
    // ===========

    // => Save calibration factors to PREFS
    void Rotation::saveCalibration() {
        cal_prefs.begin("calPrefs", false);
        cal_prefs.clear();
        cal_prefs.putFloat("u1", calibration.u1);
        cal_prefs.putFloat("u2", calibration.u2);
        cal_prefs.putFloat("a1", calibration.a1);
        cal_prefs.putFloat("a2", calibration.a2);
        cal_prefs.putFloat("offset", calibration.offset);
        cal_prefs.end();
    }

    // => Load calibration factors from PREFS and apply, create storage if it doesn't already exist.
    void Rotation::loadCalibration() {
        bool prefs_exists = cal_prefs.begin("calPrefs", true);
        calibration.u1 = cal_prefs.getFloat("u1", 0.317f);
        calibration.u2 = cal_prefs.getFloat("u2", 4.095f);
        calibration.a1 = cal_prefs.getFloat("a1", 30.0f);
        calibration.a2 = cal_prefs.getFloat("a2", 445.0f);
        calibration.offset = cal_prefs.getFloat("offset", 0.0f);
        cal_prefs.end();
        applyCalibration();
        if (!prefs_exists) {
            saveCalibration();
        }
    }

    // => Calculate gradient and offset from calibration factors
    void Rotation::applyCalibration() {
        calibration.d_grad = (calibration.a2 - calibration.a1) / (calibration.u2 - calibration.u1);
        calibration.u_0 = calibration.a1 - ((calibration.a2 - calibration.a1) / (calibration.u2 - calibration.u1) * calibration.u1);
    }

    // => Set calibration factors and apply and save
    void Rotation::calibrate(const float &u1, const float &u2,
                             const float &a1, const float &a2) {
        calibration.u1 = u1;
        calibration.u2 = u2;
        calibration.a1 = a1;
        calibration.a2 = a2;
        applyCalibration();
        saveCalibration();
    }


    // Values
    // ======

    // => Set constant angle-offset and save calibration
    void Rotation::setAngleOffset(const float &offset) {
        calibration.offset = offset;
        saveCalibration();
    }

    // => Start rotation in given direction
    void Rotation::startRotation(const int &dir) {
        digitalWrite(rot_pins[dir], LOW);
    }

    // => Stop rotor
    void Rotation::stopRotor() {
        digitalWrite(rot_pins[0], HIGH);
        digitalWrite(rot_pins[1], HIGH);
    }

    // => Set DAC voltage on speed pin
    void Rotation::setSpeedDAC(const int &speed) {
        if (speed == 0) {
            dacDisable(speed_pin);
        } else {
            dacWrite(speed_pin, (uint8_t) std::round(speed * 2.55f));
        }
    }

    // => Get current raw ADC value
    int Rotation::getADCValue() {
        update();
        return last_adc_value;
    }

    // => Get current ADC voltage
    float Rotation::getADCVolts() {
        update();
        return last_adc_volts;
    }

    // => Get rotor's current azimuth angle, a new value is pulled from ADC
    float Rotation::getAngle() {
        update();
        return last_angle;
    }

    // => Read ADC and update last rotor position values
    void Rotation::update() {
        if (!ads_failed) {
            // Read ADC and compute volts
            last_adc_value = adc.readADC_SingleEnded(ADC_CHANNEL);
            last_ms = millis();
            last_adc_volts = adc.computeVolts(last_adc_value);

            // Calculate angle using calibration
            last_angle = calibration.d_grad * last_adc_volts * calibration.volt_div_factor
                       + calibration.u_0 + calibration.offset;
            last_angle_rad = last_angle * DEG_TO_RAD;
        }
    }
}
