#ifndef ROTATION_H
#define ROTATION_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <Preferences.h>


namespace Rotor {

    // Rotor Rotation Class
    // ********************
    // Handles the I/O stuff and calibration of the real rotor
    class Rotation {
    private:
        // ADC configuration
        Adafruit_ADS1115 adc;
        bool ads_failed = false;

        // PREFS for calibration parameters
        Preferences cal_prefs;

        // => Save calibration factors to PREFS
        void saveCalibration();        

        // => Load calibration factors from PREFS and apply
        void loadCalibration();

        // => Calculate gradient and offset from calibration factors
        void applyCalibration();

    public:
        // Last rotor values
        unsigned long last_ms = 0.0;
        uint16_t last_adc_value = 0;
        float last_adc_volts = 0.0;
        float last_angle = 0.0;
        
        // Calibration parameters
        struct {
            float u1, u2, a1, a2;
            float d_grad, u_0;
            const float volt_div_factor = 1.5f;
            float offset;
        } calibration;

        // Default construcutor
        Rotation() {};

        // => Initialistion, call from setup()
        bool init();

        // => Set calibration factors and apply and save
        void calibrate(const float &u1, const float &u2,
                       const float &a1, const float &a2);

        // => Set constant angle-offset and save calibration
        void setAngleOffset(const float &offset);

        // => Return ADC status
        bool getADCStatus() { return !ads_failed; }

        // => Get current raw ADC value
        uint16_t getADCValue();

        // => Get current ADC voltage
        float getADCVolts();

        // => Get current rotor's azimuth angle, a new value is pulled from ADC
        float getAngle();

        // => Start rotation in given direction
        void startRotation(const uint8_t &dir) const;

        // => Stop rotor
        void stopRotor() const;

        // => Set DAC voltage on speed pin
        void setSpeedDAC(const uint8_t &speed) const;

        // => Read ADC and update last rotor position values
        void update();        
    };
}

#endif //ROTATION_H
