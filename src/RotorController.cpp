#include <RotorController.h>
#include <Preferences.h>

namespace Rotor {

    // PREFS instances
    Preferences cal_prefs;

    // *****************************
    // Define Rotation class members
    // *****************************
    Rotation::Rotation() {

    }

    // Initialisation, called from setup()
    void Rotation::init() {
        // Init rotation pins
        pinMode(rot_pins[0], OUTPUT);
        pinMode(rot_pins[1], OUTPUT);
        digitalWrite(rot_pins[0], HIGH);
        digitalWrite(rot_pins[1], HIGH);

        // Init speed pin
        dacWrite(speed_pin, 0);

        // Init ADS1115
        adc.setGain(GAIN_ONE); 
        if (!adc.begin()) {
            Serial.println("Failed to initialize ADS1115.");
        }

        // Init calibration factors
        loadCalibration();
    }

    // Set calibration factors and apply and save
    void Rotation::calibrate(const float &u1, const float &u2,
                             const float &a1, const float &a2) {
        calibration.u1 = u1;
        calibration.u2 = u2;
        calibration.a1 = a1;
        calibration.a2 = a2;
        applyCalibration();
        saveCalibration();
    }

    // Calculate gradient and offset from calibration factors
    void Rotation::applyCalibration() {
        // Gradient, Y-Offset u_0
        calibration.d_grad = (calibration.a2 - calibration.a1) / (calibration.u2 - calibration.u1);
        calibration.u_0 = calibration.a1 - ((calibration.a2 - calibration.a1) / (calibration.u2 - calibration.u1) * calibration.u1);
    }

    // Save calibration factors to PREFS
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

    // Load calibration factors from PREFS and apply, create storage if it doesnt already exist.
    void Rotation::loadCalibration() {
        bool prefs_exists = cal_prefs.begin("calPrefs", true);
        calibration.u1 = cal_prefs.getFloat("u1", 0.317 );
        calibration.u2 = cal_prefs.getFloat("u2", 4.095);
        calibration.a1 = cal_prefs.getFloat("a1", 30.0);
        calibration.a2 = cal_prefs.getFloat("a2", 445.0);
        calibration.offset = cal_prefs.getFloat("offset", 0.0);
        cal_prefs.end();
        applyCalibration();
        if (!prefs_exists) {
            saveCalibration();
        }
    }

    // Set constant angle-offset and save
    void Rotation::setAngleOffset(const float &offset) {
        calibration.offset = offset;
        saveCalibration();
    }

    // Get raw ADC value
    int Rotation::getADCValue() {
        last_adc_value = adc.readADC_SingleEnded(adc_channel);
        return last_adc_value;
    }

    // Get ADC voltage
    float Rotation::getADCVolts() {
        last_adc_volts =  adc.computeVolts(getADCValue());
        return last_adc_volts;
    }

    // Get rotor's azimuth angle
    float Rotation::getAngle() {
        getADCVolts();
        last_angle = calibration.d_grad * last_adc_volts * calibration.volt_div_factor
                   + calibration.u_0 + calibration.offset;
        return last_angle;
    }

    // Update the last_... values
    void Rotation::update() {
        getADCValue();
        getAngle();
    }

    // Start rotation in given direction
    void Rotation::startRotation(const int &dir) {
        digitalWrite(rot_pins[dir], LOW);
    }

    // Stop rotor
    void Rotation::stopRotor() {
        digitalWrite(rot_pins[0], HIGH);
        digitalWrite(rot_pins[1], HIGH);
    }

    // Set rotation speed
    void Rotation::setSpeedDAC(const int &speed) {
        if (speed == 0) {
            dacDisable(speed_pin);
        } else {
            dacWrite(speed_pin, (uint8_t) std::round(speed * 2.55));
        }
    }


    // ************************
    // Define Messenger members
    // ************************
    Messenger::Messenger() {
        // Set buffer size on the heap
        msg_buffer.reserve(100);
    }

    // Set rotation msg into buffer
    void Messenger::setLastRotationMsg(const bool &with_angle) {
        if (rotor_ptr != NULL) {
            // Buffers
            msg_buffer = "ROTOR|";
            StaticJsonDocument<100> doc;

            // Angle
            if (with_angle) {
                doc["adc_v"] = round(rotor_ptr->rotor.last_adc_volts * 1000.0) / 1000.0;
                doc["angle"] = round(rotor_ptr->rotor.last_angle * 100.0) / 100.0;
            }

            // Rotation & direction
            if (!rotor_ptr->is_rotating) {
                doc["rotation"] = 0;
            } else if (rotor_ptr->direction) {
                doc["rotation"] = 1;
            } else {
                doc["rotation"] = -1;
            }

            // Send message
            serializeJson(doc, msg_buffer);
        }
    }

    // Send last rotation values over web socket
    void Messenger::sendLastRotationMsg(const bool &with_angle) {
        setLastRotationMsg(with_angle);
        socket.textAll(msg_buffer);
    }

    // Send new rotation values over web socket
    void Messenger::sendNewRotationMsg(const bool &with_angle) {
        rotor_ptr->rotor.update();
        sendLastRotationMsg(with_angle);
    }

    // Send speed of rotation over web socket
    void Messenger::sendSpeed() {
        msg_buffer = "ROTOR|";
        StaticJsonDocument<20> doc;
        doc["speed"] = rotor_ptr->speed;
        serializeJson(doc, msg_buffer);
        socket.textAll(msg_buffer);
    }
    

    // ******************************
    // Define RotorController members
    // ******************************
    void RotorController::init() {
        rotor.init();
        // Messenger instance gets ptr to this Controller instance
        messenger.rotor_ptr = this;
    }

    // Start rotating in given direction
    void RotorController::startRotation(const int dir) {
        if (!is_rotating) {
            direction = dir;
            is_rotating = true;
            rotor.startRotation(direction);
            messenger.sendLastRotationMsg(false);
            if (verbose) { Serial.println("[ROTOR] Started rotation (" + directions[dir]  + ")"); }
        } else {
            if (verbose) { Serial.println("[ROTOR] Can't start rotation, already rotating."); }
        }
    }

    // Stop rotor
    void RotorController::stop() {
        if (is_rotating) {
            rotor.stopRotor();
            is_rotating = false;
            messenger.sendLastRotationMsg(false);
            if (verbose) { Serial.println("[ROTOR] Stopped rotation."); }
        } else {
            if (verbose) { Serial.println("[ROTOR] Is already stationary."); }
        }
    }

    // Set rotor speed
    void RotorController::setSpeed(const int spd) {
        speed = spd;
        rotor.setSpeedDAC(speed);
        messenger.sendSpeed();
        if (verbose) { 
            Serial.print("[ROTOR] Set speed (");
            Serial.print(speed);
            Serial.println(" %)"); 
        }
    }

    // Set calibration
    void RotorController::setCalibration(const float u1, const float u2,
                                         const float a1, const float a2) {
        rotor.calibrate(u1, u2, a1, a2);
        if (verbose) {
            Serial.print("[ROTOR] Set calibration: ");
            Serial.print(u1, 4); Serial.print(" V | ");
            Serial.print(u2, 4); Serial.print(" V | ");
            Serial.print(a1, 2); Serial.print("° | ");
            Serial.print(a2, 2); Serial.println("°");
        }
    }

    // Set angle-offset
    void RotorController::setAngleOffset(const int offset) {
        rotor.setAngleOffset(offset);
        if (verbose) {
            Serial.print("[ROTOR] Set angle offset: ");
            Serial.print(offset);
            Serial.println("°");
        }
    }
}
