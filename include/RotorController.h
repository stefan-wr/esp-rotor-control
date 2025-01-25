#ifndef ROTORCONTROLLER_H
#define ROTORCONTROLLER_H

#include <Arduino.h>
#include <Timer.h>
#include <Rotation.h>
#include <RotorMessenger.h>


namespace Rotor {

    // Rotor Controller Class
    // **********************
    // Controls rotor state and rotor-messenger
    class RotorController {
    private:
        // Auto rotation variables
        struct {
            const int max_angle = 449;
            const uint8_t min_distance = 2;
            const float tolerance = 0.7f;
            uint8_t timeout_counter = 0;
            const unsigned long timeout = 4000;
            const unsigned long counter_interval = 500;
            Timer counterTimer;
            Timer timer;
        } auto_rot;

        // Rotor angle from previous angular speed calculation
        struct {
            unsigned long last_ms = 0;
            float last_angle = 0.0f;
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
        int getSmoothSpeed() const;

        // => Get smooth speed scaling factor, using tanh
        float getSpeedRampFactor(const float x, const float gradient) const;

        // => Set current rotor speed (DAC), doesn't distribute to clients
        void setCurrentSpeed(const uint8_t spd);


    public:
        // Rotor state
        bool is_rotating = false;
        bool is_auto_rotating = false;
        float auto_rotation_target;         // Target angle in degrees
        bool smooth_speed_active = false;
        uint8_t direction = 0;              // 0: CCW, 1: CW
        uint8_t max_speed = 0;              // 0% to 100%
        uint8_t current_speed = 0;          // 0% to max_speed
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
        void startRotation(const uint8_t dir);

        // => Stop rotor, distribute new state to clients
        void stop(const bool &distribute = true);

        // => Set max rotor speed, distribute new state to clients.
        // Is applied to DAC only if speed is not ramping up / down.
        void setMaxSpeed(const uint8_t spd);

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

        // => Set smooth speed to DAC.
        // To be called continously from main loop if speed ramp is active
        void watchSmoothSpeedRamp();

        // => Update rotor values from ADC and calculate angular speed
        void update(bool with_angular_speed = false);
    };
}

extern Rotor::RotorController rotor_ctrl;

#endif //ROTORCONTROLLER_H
