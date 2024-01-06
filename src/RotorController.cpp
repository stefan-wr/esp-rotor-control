#include <RotorController.h>
#include <Preferences.h>

namespace Rotor {
    const String directions[2] = {"CCW", "CW"};

    // PREFS instances
    Preferences cal_prefs;

    // *****************************
    // Define Rotation class members
    // *****************************

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
        if (!adc.begin(0x48)) {
            Serial.println("[ROTOR] Failed to initialize ADS1115!");
            ads_failed = true;
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

    // Read ADC and update rotor position values (ADC value -> ADC volts -> angle)
    void Rotation::update() {
        if (ads_failed) {
            last_ms = 0.0;
            last_adc_value = 0;
            last_adc_volts = 0;
            last_angle = 0;
        } else {
            last_ms = millis();
            last_adc_value = adc.readADC_SingleEnded(adc_channel);
            last_adc_volts = adc.computeVolts(last_adc_value);
            last_angle = calibration.d_grad * last_adc_volts * calibration.volt_div_factor
                       + calibration.u_0 + calibration.offset;
        }
    }

    // Get current raw ADC value
    int Rotation::getADCValue() {
        update();
        return last_adc_value;
    }

    // Get current ADC voltage
    float Rotation::getADCVolts() {
        update();
        return last_adc_volts;
    }

    // Get current rotor's azimuth angle
    float Rotation::getAngle() {
        update();
        return last_angle;
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
        // Set message buffer size on the heap
        msg_buffer.reserve(100);
    }

    // Set last rotation msg into buffer
    void Messenger::setLastRotationMsg(const bool &with_angle) {
        if (rotor_ptr != NULL) {
            // Buffers
            msg_buffer = "ROTOR|";
            StaticJsonDocument<100> doc;

            // Rotation & direction | always included in msg
            if (!rotor_ptr->is_rotating) {
                doc["rotation"] = 0;
            } else if (rotor_ptr->direction) {
                doc["rotation"] = 1;
            } else {
                doc["rotation"] = -1;
            }

            // Target | only included in msg if angle is not included and rotor is on auto-rotation
            if (!with_angle && rotor_ptr->is_auto_rotating) {
                doc["target"] = round(rotor_ptr->auto_rotation_target * 100.0) / 100.0;
            }

            // Angle | only included in msg if needed (specified with argument)
            if (with_angle) {
                doc["adc_v"] = round(rotor_ptr->rotor.last_adc_volts
                                     * rotor_ptr->rotor.calibration.volt_div_factor
                                     * 1000.0) / 1000.0;
                doc["angle"] = round(rotor_ptr->rotor.last_angle * 100.0) / 100.0;
            }

            // Set msg into buffer
            serializeJson(doc, msg_buffer);
        }
    }

    // Send last rotation values over web socket
    void Messenger::sendLastRotation(const bool &with_angle) {
        setLastRotationMsg(with_angle);
        socket.textAll(msg_buffer);
    }

    // Send new rotation values over web socket, always with angle
    void Messenger::sendNewRotation() {
        rotor_ptr->rotor.update();
        sendLastRotation(true);
    }

    // Send speed of rotation over web socket
    void Messenger::sendSpeed() {
        msg_buffer = "ROTOR|";
        StaticJsonDocument<20> doc;
        doc["speed"] = rotor_ptr->speed;
        serializeJson(doc, msg_buffer);
        socket.textAll(msg_buffer);
    }

    // Send current calibration parameters over web socket
    void Messenger::sendCalibration() {
        msg_buffer = "CALIBRATION|";
        StaticJsonDocument<100> doc;
        doc["a1"] = round(rotor_ptr->rotor.calibration.a1 * 10000.0) / 10000.0;
        doc["u1"] = round(rotor_ptr->rotor.calibration.u1 * 10000.0) / 10000.0;
        doc["a2"] = round(rotor_ptr->rotor.calibration.a2 * 10000.0) / 10000.0;
        doc["u2"] = round(rotor_ptr->rotor.calibration.u2 * 10000.0) / 10000.0;
        doc["offset"] = rotor_ptr->rotor.calibration.offset;
        serializeJson(doc, msg_buffer);
        socket.textAll(msg_buffer);
    }

    // Send auto rotation target over web socket
    void Messenger::sendTarget() {
        msg_buffer = "ROTOR|";
        StaticJsonDocument<40> doc;
        doc["target"] = round(rotor_ptr->auto_rotation_target * 100.0) / 100.0;
        serializeJson(doc, msg_buffer);
        socket.textAll(msg_buffer);
    }
    





    // ******************************
    // Define RotorController members
    // ******************************
    void RotorController::init() {
        // Init Rotor instance
        rotor.init();
        rotor.update();
        previous.last_angle = rotor.last_angle;
        previous.last_ms = rotor.last_ms;
        auto_rot.timer_ptr = new Timer(auto_rot.timeout);

        // Messenger instance gets ptr to this Controller instance
        messenger.rotor_ptr = this;
    }

    // Start rotating in given direction
    void RotorController::startRotation(const int dir) {
        if (!is_rotating) {
            direction = dir;
            is_rotating = true;
            rotor.startRotation(direction);
            messenger.sendLastRotation(false);
            if (verbose) {
                Serial.print("[ROTOR] Started rotation (");
                Serial.print(directions[dir]);
                Serial.println(").");
            }
        } else {
            if (verbose) { Serial.println("[ROTOR] Can't start rotation, already rotating."); }
        }
    }

    // Stop rotor
    void RotorController::stop() {
        if (is_rotating) {
            rotor.stopRotor();
            is_rotating = false;
            is_auto_rotating = false;
            messenger.sendLastRotation(false);
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
            Serial.println(" %)."); 
        }
    }

    // Set calibration
    void RotorController::setCalibration(const float u1, const float u2,
                                         const float a1, const float a2) {
        rotor.calibrate(u1, u2, a1, a2);
        messenger.sendCalibration();
        if (verbose) {
            Serial.print("[ROTOR] Set calibration: ");
            Serial.print(u1, 4); Serial.print(" V | ");
            Serial.print(u2, 4); Serial.print(" V | ");
            Serial.print(a1, 2); Serial.print("° | ");
            Serial.print(a2, 2); Serial.println("°.");
        }
    }

    // Set angle-offset
    void RotorController::setAngleOffset(const int offset) {
        rotor.setAngleOffset(offset);
        messenger.sendCalibration();
        if (verbose) {
            Serial.print("[ROTOR] Set angle offset: ");
            Serial.print(offset);
            Serial.println("°.");
        }
    }

    // Auto-rotate to given angle
    void RotorController::rotateTo(const float target_angle, const bool use_overlap) {
        // Stop previous rotation
        stop();

        // Determine shortest direction to target
        int overlap_border = auto_rot.max_angle - 360;
        float current_angle = rotor.getAngle();
        float distance = target_angle - current_angle;
        
        // Target angle is in overlap region (from 0° to overlapBorder)
        // -> this means it's possible to reach it with +360°, too.
        if (use_overlap && target_angle <= overlap_border) {
            // If distance is less than -180° -> flip it.
            if (distance < -180) {
                distance += 360;
            }
        }

        // Do nothing if distance to target is too small
        if (abs(distance) < auto_rot.min_distance) {
            if (verbose) {
                Serial.print("[ROTOR] Auto-rotation request denied. Target too close to current position.");
            }
            return;
        }
    	
        // Direction
        int target_dir = (distance > 0) ? +1 : 0;

        // Serial output
        if (verbose) {
            Serial.print("[ROTOR] Auto-rotation request: Target: ");
            Serial.print(target_angle, 1);
            Serial.print("° | Computed target: " );
            Serial.print(current_angle + distance);
            Serial.print("° | Direction: ");
            Serial.println(directions[target_dir]);
        }

        // Start auto rotation
        auto_rotation_target = current_angle + distance;
        auto_rot.timer_ptr->reset();
        auto_rot.timer_ptr->start();
        is_auto_rotating = true;
        startRotation(target_dir);
    }

    // Stop auto rotation if target has been reached.
    // To be called continously from main loop during auto rotation.
    // If angular-speed is zero 3s into auto-rotation, start 3s timeout.
    void RotorController::watchAutoRotation() {
        float current_angle = rotor.getAngle();

        // Target reached
        if ((direction == 0 && current_angle <= auto_rotation_target + auto_rot.tolerance) ||
            (direction == 1 && current_angle >= auto_rotation_target - auto_rot.tolerance)) {
            stop();
            if (verbose) {
                Serial.print("[ROTOR] Auto-rotation target (");
                Serial.print(auto_rotation_target);
                Serial.print("°) reached with: ");
                Serial.print(rotor.getAngle());
                Serial.println("°.");
            }

        // Wait 3s -> check if rotor stopped before reaching target
        // -> wait 3s again -> abort auto roation if rotor is still stationary.
        } else if (auto_rot.timer_ptr->passed() && !angular_speed) {
            if (auto_rot.timer_ptr->n_passed >= 2) {
                stop();
                if (verbose) {
                    Serial.println("[ROTOR] Auto-rotation aborted. Rotor stopped before reaching target.");
                }
            }
        }
    }

    // Update rotor values from ADC and calculate angular speed
    void RotorController::update() {
        // Update rotor values
        rotor.update();

        // Calculate angular speed
        float current_angular_speed = (rotor.last_angle - previous.last_angle) 
                                    / (rotor.last_ms - previous.last_ms) * 1000.0;

        if (abs(current_angular_speed) < 0.1) {
            angular_speed = 0;
        } else {
            angular_speed = current_angular_speed;
        }

        // Set previous values
        previous.last_angle = rotor.last_angle;
        previous.last_ms = rotor.last_ms;
    }
}
