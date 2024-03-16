#include <Arduino.h>
#include <ArduinoJson.h>
#include <math.h>

#include <globals.h>
#include <RotorController.h>
#include <Adafruit_ADS1X15.h>
#include <Timer.h>
#include <ESPAsyncWebServer.h>
#include <RotorSocket.h>        // Exposes Global: websocket


namespace Rotor {
    const String directions[2] = {"CCW", "CW"};

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
        if (!adc.begin(0x48)) {
            Serial.println("[Rotor] Failed to initialise ADS1115!");
            ads_failed = true;
        }

        // Load calibration factors
        loadCalibration();

        return !ads_failed;
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

    // => Calculate gradient and offset from calibration factors
    void Rotation::applyCalibration() {
        calibration.d_grad = (calibration.a2 - calibration.a1) / (calibration.u2 - calibration.u1);
        calibration.u_0 = calibration.a1 - ((calibration.a2 - calibration.a1) / (calibration.u2 - calibration.u1) * calibration.u1);
    }

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

    // => Get rotor's current azimuth angle
    float Rotation::getAngle() {
        update();
        return last_angle;
    }

    // => Read ADC and update last rotor position values
    void Rotation::update() {
        if (!ads_failed) {
            // Read ADC and compute volts
            last_adc_value = adc.readADC_SingleEnded(adc_channel);
            last_ms = millis();
            last_adc_volts = adc.computeVolts(last_adc_value);

            // Calculate angle using calibration
            last_angle = calibration.d_grad * last_adc_volts * calibration.volt_div_factor
                       + calibration.u_0 + calibration.offset;
            last_angle_rad = last_angle * deg_to_rad_factor;
        }
    }    






    // ************************
    // Define Messenger members
    // ************************

    Messenger::Messenger() {
        // Set message buffer size on the heap
        msg_buffer.reserve(100);
    }

    // => Create JSON message from last rotor values and save in msg_buffer
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

            // Target | only included in msg if angle is not included and rotor is on auto-rotating
            if (!with_angle && rotor_ptr->is_auto_rotating) {
                doc["target"] = round(rotor_ptr->auto_rotation_target * 100.0) / 100.0;
            }

            // Angle | only included in msg if specified
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

    // => Send last rotation values
    void Messenger::sendLastRotation(const bool &with_angle) {
        setLastRotationMsg(with_angle);
        websocket.textAll(msg_buffer);
    }

    // => Send new rotation values from ADC, always includes angle
    void Messenger::sendNewRotation() {
        rotor_ptr->rotor.update();
        sendLastRotation(true);
    }

    // => Send max speed
    void Messenger::sendSpeed() {
        msg_buffer = "ROTOR|";
        StaticJsonDocument<16> doc;
        doc["speed"] = rotor_ptr->max_speed;
        serializeJson(doc, msg_buffer);
        websocket.textAll(msg_buffer);
    }

    // => Send current calibration parameters
    void Messenger::sendCalibration() {
        msg_buffer = "CALIBRATION|";
        StaticJsonDocument<96> doc;
        doc["a1"] = round(rotor_ptr->rotor.calibration.a1 * 10000.0) / 10000.0;
        doc["u1"] = round(rotor_ptr->rotor.calibration.u1 * 10000.0) / 10000.0;
        doc["a2"] = round(rotor_ptr->rotor.calibration.a2 * 10000.0) / 10000.0;
        doc["u2"] = round(rotor_ptr->rotor.calibration.u2 * 10000.0) / 10000.0;
        doc["offset"] = rotor_ptr->rotor.calibration.offset;
        serializeJson(doc, msg_buffer);
        websocket.textAll(msg_buffer);
    }

    // => Send auto rotation target
    void Messenger::sendTarget() {
        msg_buffer = "ROTOR|";
        StaticJsonDocument<32> doc;
        doc["target"] = round(rotor_ptr->auto_rotation_target * 100.0) / 100.0;
        serializeJson(doc, msg_buffer);
        websocket.textAll(msg_buffer);
    }    






    // ******************************
    // Define RotorController members
    // ******************************

    // => Initialisation, called from setup()
    bool RotorController::init() {
        bool rotorInitSuccess = rotor.init();
        rotor.update();

        // Init variables for calculating angular speed
        previous.last_angle = rotor.last_angle;
        previous.last_ms = rotor.last_ms;
        auto_rot.timer = new Timer(auto_rot.timeout);
        auto_rot.counterTimer = new Timer(auto_rot.counterInterval);

        messenger.rotor_ptr = this;     // Messenger gets pointer to this instance
        return rotorInitSuccess;
    }

    // --------
    // Commands
    // --------

    // => Start rotating in given direction, distribute new state to clients
    void RotorController::startRotation(const int dir) {
        if (!is_rotating) {
            direction = dir;
            is_rotating = true;
            rotor.startRotation(direction);
            messenger.sendLastRotation(false);
            if (verbose) {
                Serial.print("[Rotor] Started rotation (");
                Serial.print(directions[dir]);
                Serial.println(").");
            }
        } else {
            if (verbose) { Serial.println("[Rotor] Can't start rotation, already rotating."); }
        }
    }

    // => Stop rotor, distribute new state to clients
    void RotorController::stop() {
        rotor.stopRotor();
        if (is_rotating) {
            is_rotating = false;
            is_auto_rotating = false;

            // If active, disable speed ramp and reset speed DAC
            if (smooth_speed_active) {
                smooth_speed_active = false;
                setCurrentSpeed(max_speed);
            }

            messenger.sendLastRotation(false);
            if (verbose) { Serial.println("[Rotor] Stopped rotation."); }
        } else {
            if (verbose) { Serial.println("[Rotor] Is already stationary."); }
        }
    }

    // => Set max rotor speed, distribute new state to clients
    void RotorController::setMaxSpeed(const int spd) {
        max_speed = spd;
        messenger.sendSpeed();

        // Only set DAC if smooth speed is currently not active
        if (!smooth_speed_active) {
            rotor.setSpeedDAC(max_speed);
        }

        if (verbose) { 
            Serial.print("[Rotor] Set max speed (");
            Serial.print(max_speed);
            Serial.println(" %)."); 
        }
    }

    // => Set current rotor speed, do not distribute
    void RotorController::setCurrentSpeed(const int spd) {
        current_speed = spd;
        rotor.setSpeedDAC(current_speed);
    }

    // => Set calibration, distribute new state to clients
    void RotorController::setCalibration(const float u1, const float u2,
                                         const float a1, const float a2) {
        rotor.calibrate(u1, u2, a1, a2);
        messenger.sendCalibration();
        if (verbose) {
            Serial.print("[Rotor] Set calibration: ");
            Serial.print(u1, 4); Serial.print(" V | ");
            Serial.print(u2, 4); Serial.print(" V | ");
            Serial.print(a1, 2); Serial.print("° | ");
            Serial.print(a2, 2); Serial.println("°.");
        }
    }

    // => Set angle-offset, distribute new state to clients
    void RotorController::setAngleOffset(const int offset) {
        rotor.setAngleOffset(offset);
        messenger.sendCalibration();
        if (verbose) {
            Serial.print("[Rotor] Set angle offset: ");
            Serial.print(offset);
            Serial.println("°.");
        }
    }

    // => Auto-rotate to to given angle.
    // If use_overlap is true -> choose shortest path to angle, include overlap region.
    // If use_smooth_speed is true -> ramp up/down speed when starting/stopping auto-rotating
    // **************************************************************************************
    void RotorController::rotateTo(const float target_angle,
                                   const bool use_overlap,
                                   const bool use_smooth_speed) {
        // Stop previous rotation
        stop();

        // Determine shortest direction to target
        int overlap_border = auto_rot.max_angle - 360;
        float current_angle = rotor.getAngle();
        float distance = target_angle - current_angle;
        
        // Target angle is in overlap region (from 0° to overlapBorder)
        // -> this means it's possible to reach it with +360°, too.
        if (use_overlap && target_angle <= overlap_border) {
            // If distance is less than -180° (absolut distance > 180) -> flip it.
            if (distance < -180.0f) {
                distance += 360.0f;
            }
        }

        // Absolute distance
        float abs_distance = abs(distance);

        // Do nothing if distance to target is too small
        if (abs_distance < auto_rot.min_distance) {
            if (verbose) {
                Serial.print("[Rotor] Auto-rotation request denied. Target too close to current position.");
            }
            return;
        }
    	
        // Choose rotation direction
        int target_dir = (distance > 0) ? +1 : 0;

        // Set up speed ramp if enabled
        if (use_smooth_speed) {
            smooth_speed_active = true;
            speed_ramp.start_angle = current_angle;

            if (abs_distance < speed_ramp.min_distance) {
                // Scale down max speed for shorter distances using a sin-function
                speed_ramp.speed_distance_factor = sin(abs_distance / speed_ramp.min_distance * M_PI_2);
                speed_ramp.ramp_distance = abs_distance / 2.0f;
            } else {
                // Use max speed
                speed_ramp.speed_distance_factor = 1.0f;
                speed_ramp.ramp_distance = speed_ramp.min_distance / 2.0f;
            }
        } else {
            smooth_speed_active = false;
        }

        // Serial output
        if (verbose) {
            Serial.print("[Rotor] Auto-rotation request: Target: ");
            Serial.print(target_angle, 1);
            Serial.print("° | Computed target: " );
            Serial.print(current_angle + distance);
            Serial.print("° | Direction: ");
            Serial.print(directions[target_dir]);
            Serial.print(" | Use OL: ");
            Serial.print(use_overlap);
            Serial.print(" | Smooth speed: ");
            Serial.print(use_smooth_speed);
            if (use_smooth_speed) {
                Serial.print(" | Max speed: ");
                Serial.print(speed_ramp.speed_distance_factor * 100);
                Serial.print("%");
            }
            Serial.println();
        }

        // Start auto rotation
        auto_rotation_target = current_angle + distance;
        auto_rotation_target_rad = auto_rotation_target * deg_to_rad_factor;
        auto_rot.timer->reset();
        auto_rot.timer->start();
        auto_rot.timeoutCounter = 0;
        is_auto_rotating = true;
        startRotation(target_dir);
    }

    // => Stop auto rotation if target has been reached.
    // To be called continously from main loop during auto rotation.
    // If angular-speed is zero 3s into auto-rotation, start 3s timeout.
    // *****************************************************************
    void RotorController::watchAutoRotation() {
        // Target reached
        if ((direction == 0 && rotor.last_angle <= auto_rotation_target + auto_rot.tolerance) ||
            (direction == 1 && rotor.last_angle >= auto_rotation_target - auto_rot.tolerance)) {
            stop();
            if (verbose) {
                Serial.print("[Rotor] Auto-rotation target (");
                Serial.print(auto_rotation_target);
                Serial.print("°) reached with: ");
                Serial.print(rotor.getAngle());
                Serial.println("°.");
            }

        // Initially, wait 4s -> then check if rotor stopped before reaching target.
        } else if (!angular_speed && (auto_rot.timer->passed() || (auto_rot.timeoutCounter && auto_rot.counterTimer->passed()))) {
            auto_rot.timeoutCounter++;

            // Stop rotation if rotor was checked to be stationary 4-times in a row
            if (auto_rot.timeoutCounter >= 4) {
                stop();
                if (verbose) {
                    Serial.println("[Rotor] Auto-rotation aborted. Rotor stopped before reaching target.");
                }
            }

        // Reset stationary counter if angular speed is not zero  
        } else if (angular_speed && auto_rot.timeoutCounter > 0) {
            auto_rot.timeoutCounter = 0;
        }
    }

    // => Set smooth speed to DAC.
    // To be called continously from main loop if speed ramp is active
    // ***************************************************************
    void RotorController::watchSmoothSpeedRamp() {
        int new_speed = getSmoothSpeed();

        if (new_speed != current_speed) {
            setCurrentSpeed(new_speed);

            // Serial output
            if (verbose) {
                Serial.print("[Rotor] Speed (");
                Serial.printf("%3d", new_speed);
                Serial.print("%) | Distances: ");
                Serial.printf("%5.1f", abs(rotor.last_angle - speed_ramp.start_angle));
                Serial.print(" <-+-> ");
                Serial.printf("%5.1f\n\r", abs(rotor.last_angle - auto_rotation_target));
            }
        }    
    }

    // => Get current speed when ramping up/down speed
    // ***********************************************
    int RotorController::getSmoothSpeed() {
        // Max speed is already 0
        if (max_speed == 0) { return max_speed; }

        // Distances
        float distance_to_start = abs(rotor.last_angle - speed_ramp.start_angle);
        float distance_to_target = abs(rotor.last_angle - auto_rotation_target);

        // Set speed multiplication factor
        float speed_ramp_factor;
        if (distance_to_start == 0.0f || distance_to_target == 0.0f) {
            // If any distance = 0 => speed = 0
            speed_ramp_factor = 0.0f;
        } else if (distance_to_start < speed_ramp.ramp_distance) {
            // Ramping up
            float x = distance_to_start / speed_ramp.ramp_distance;
            speed_ramp_factor = getSpeedRampFactor(x, speed_ramp.gradient);
        } else if (distance_to_target < speed_ramp.ramp_distance) {
            // Ramping down
            float x = distance_to_target / speed_ramp.ramp_distance;
            speed_ramp_factor = getSpeedRampFactor(x, speed_ramp.gradient);
        } else {
            // Constant speed
            speed_ramp_factor = 1.0f;
        }
        return (int) (max_speed * speed_ramp_factor * speed_ramp.speed_distance_factor);
    }

    // => Get smooth speed scaling factor, using tanh
    // https://math.stackexchange.com/questions/846743/example-of-a-smooth-step-function-that-is-constant-below-0-and-constant-above/846747#846747
    float RotorController::getSpeedRampFactor(const float x, const float gradient) {
        return (0.5f * (1.0f + std::tanh(((2 * x - 1.0f) * gradient) / (std::sqrt((1.0f - x) * x)))));
    }

    // => Update rotor values from ADC and calculate angular speed
    // ***********************************************************
    void RotorController::update(bool with_angular_speed) {
        rotor.update();

        // Calculate angular speed
        if (with_angular_speed && rotor.getADCStatus()) {
            float new_angular_speed = (rotor.last_angle - previous.last_angle) 
                                    / (rotor.last_ms - previous.last_ms) * 1000.0f;
            
            // Exponential moving average
            new_angular_speed = (0.5f * new_angular_speed) + (0.5f * angular_speed);

            if (abs(new_angular_speed) <= 0.1f) {
                angular_speed = 0.0f;
            } else {
                angular_speed = new_angular_speed;
            }

            // Set previous values for next calculation
            previous.last_angle = rotor.last_angle;
            previous.last_ms = rotor.last_ms;
        }
    }
}

Rotor::RotorController rotor_ctrl;
