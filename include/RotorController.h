#ifndef ROTORCONTROLLER_H
#define ROTORCONTROLLER_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <ESPAsyncWebServer.h>
#include <Timer.h>
#include <Preferences.h>


namespace Rotor {

    class Rotation;
    class Messenger;
    class RotorController;

    // Rotor Rotation Class
    // ********************
    // Handles the I/O stuff and calibration of the real rotor
    class Rotation {
    private:
        // ADC configuration
        Adafruit_ADS1115 adc;
        const int adc_channel = 0;
        bool ads_failed = false;
        Preferences cal_prefs;  // PREFS for calibration parameters

        // => Calculate gradient and offset from calibration factors
        void applyCalibration();

        // => Save calibration factors to PREFS
        void saveCalibration();

        // => Load calibration factors from PREFS and apply
        void loadCalibration();

    public:
        // Last rotor values
        unsigned long last_ms = 0.0;
        int last_adc_value = 0;
        float last_adc_volts = 0.0;
        float last_angle = 0.0;
        float last_angle_rad = 0.0;
        
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

        // => Get current raw ADC value
        int getADCValue();

        // => Get current ADC voltage
        float getADCVolts();

        // => Get current rotor's azimuth angle
        float getAngle();

        // => Start rotation in given direction
        void startRotation(const int &dir);

        // => Stop rotor
        void stopRotor();

        // => Set DAC voltage on speed pin
        void setSpeedDAC(const int &speed);

        // => Read ADC and update last rotor position values
        void update();        
    };


    // Rotor Messenger Class
    // *********************
    // Handles outgoing messages with rotor data
    class Messenger {
    private:
        String msg_buffer;

        // => Create rotation JSON message from last rotor values and save in msg_buffer
        void setLastRotationMsg(const bool &with_angle);
    public:
        // Pointer to rotor instance defined in parent class
        RotorController* rotor_ptr;

        // Constructor
        Messenger();

        // => Send last rotation valueset
        void sendLastRotation(const bool &with_angle);

        // => Send new rotation values from ADC, always includes angle
        void sendNewRotation();

        // => Send max speed
        void sendSpeed();

        // => Send current calibration parameters
        void sendCalibration();

        // => Send auto rotation target
        void sendTarget();
    };
    

    // Rotor Controller Class
    // **********************
    // Controls rotor state and rotor-messenger
    class RotorController {
    private:
        // Auto rotation variables
        struct {
            const int max_angle = 449;
            const int min_distance = 2;
            const float tolerance = 0.7f;
            const unsigned long timeout = 4000;
            Timer* timer;
        } auto_rot;

        // Variables from previous rotor update
        struct {
            unsigned long last_ms = 0;
            float last_angle = 0.0f;
            float angular_speed = 0.0f;
            unsigned long interval = 200;
            Timer* timer;
        } previous;

        // Speed ramp variables
        struct {
            const float gradient = 1.0f;
            const float min_distance = 20.0f;
            float ramp_distance = 10.0f;
            float start_angle = 0.0f;
            float speed_distance_factor = 1.0f; 
        } speed_ramp;

        // => Get current speed when ramping up / down speed during auto-rotation
        int getSmoothSpeed();

        // => Set current rotor speed (DAC), doesn't distribute to clients
        void setCurrentSpeed(const int spd);


    public:
        // Rotor state
        bool is_rotating = false;
        bool is_auto_rotating = false;
        float auto_rotation_target;         // Target angle in degrees
        float auto_rotation_target_rad;     // Target angle in radians
        bool smooth_speed_active = false;
        int direction = 0;                  // 0: CCW, 1: CW
        int max_speed = 0;                  // 0% to 100%
        int current_speed = 0;              // 0% to max_speed
        float angular_speed = 0.0f;         // in °/s

        // Settings
        struct {
            bool use_overlap = true;
            bool use_smooth_speed = true;
        } settings;

        // Messenger and rotor instances
        Messenger messenger;
        Rotation rotor;

        RotorController() {};

        // => Initialisation, to be called from setup()
        bool init();

        // => Start rotating in given direction, distribute new state to clients
        void startRotation(const int dir);

        // => Stop rotor, distribute new state to clients
        void stop();

        // => Set max rotor speed, distribute new state to clients.
        // Is applied to DAC only if speed is not ramping up / down.
        void setMaxSpeed(const int spd);

        // => Set calibration parameters, distribute new state to clients
        void setCalibration(const float u1, const float u2,
                            const float a1, const float a2);

        // => Set angle-offset, distribute new state to clients
        void setAngleOffset(const int offset);

        // => Auto-rotate to to given angle.
        // If use_overlap is true -> choose shortest path to angle, include overlap region.
        // If use_smooth_speed is true -> ramp up/down speed when starting/stopping auto-rotating
        void rotateTo(const float angle, const bool use_overlap = true, const bool use_smooth_speed = false);

        // => Stop auto rotation if target has been reached.
        // To be called continously from main loop during auto rotation.
        // If angular-speed is zero 3s into auto-rotation, start 3s timeout.
        void watchAutoRotation();

        // => Update rotor values from ADC and calculate angular speed
        void update(bool with_angular_speed = false);
    };
}

extern Rotor::RotorController rotor_ctrl;

#endif //ROTORCONTROLLER_H
