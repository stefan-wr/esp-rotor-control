#ifndef ROTORCONTROLLER_H
#define ROTORCONTROLLER_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <ESPAsyncWebServer.h>
#include <Timer.h>


// Get websocket object from main.cpp
extern AsyncWebSocket websocket;

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

        // => Calculate gradient and offset from calibration factors
        void applyCalibration();

        // => Save calibration factors to PREFS
        void saveCalibration();

        // =>Load calibration factors from PREFS and apply.
        // Create storage if it doesnt already exist.
        void loadCalibration();

    public:
        // Last rotor values
        unsigned long last_ms = 0.0;
        int last_adc_value = 0;
        float last_adc_volts = 0.0;
        float last_angle = 0.0;
        float last_angle_rad = 0.0;
        
        // Calibration factors
        struct {
            float u1, u2, a1, a2;
            float d_grad, u_0;
            const float volt_div_factor = 1.5;
            float offset;
        } calibration;

        // Construcutor
        Rotation() {};

        // => Initialistion, call from setup()
        void init();

        // => Set calibration factors and apply and save
        void calibrate(const float &u1, const float &u2,
                       const float &a1, const float &a2);

        // => Set constant angle-offset and save calibration
        void setAngleOffset(const float &offset);

        // => Read ADC and update rotor position values (ADC value -> ADC volts -> angle)
        void update();

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

        // => Send last rotation values over web socket
        void sendLastRotation(const bool &with_angle);

        // => Send new rotation values over web socket, always with angle
        void sendNewRotation();

        // => Send speed of rotation over web socket
        void sendSpeed();

        // => Send current calibration parameters over web socket
        void sendCalibration();

        // => Send auto rotation target over web socket
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
            const float tolerance = 0.7;
            const unsigned long timeout = 4000;
            Timer* timer;
        } auto_rot;

        // Variables from previous rotor update
        struct {
            unsigned long last_ms = 0;
            float last_angle = 0;
            float angular_speed = 0;
            unsigned long interval = 200;
            Timer* timer;
        } previous;

    public:
        // Rotor state
        bool is_rotating = false;
        bool is_auto_rotating = false;
        float auto_rotation_target;
        float auto_rotation_target_rad;
        int direction = 0;          // 0: CCW, 1: CW
        int speed = 0;              // 0% to 100%
        float angular_speed = 0;    // in °/s

        // Messenger and rotor instances
        Messenger messenger;
        Rotation rotor;

        RotorController() {};

        // => Initialisation, called from setup()
        void init();

        // => Start rotating in given direction, distribute new state to clients
        void startRotation(const int dir);

        // => Stop rotor, distribute new state to clients
        void stop();

        // => Set rotor speed, distribute new state to clients
        void setSpeed(const int spd);

        // => Set calibration, distribute new state to clients
        void setCalibration(const float u1, const float u2,
                            const float a1, const float a2);

        // => Set angle-offset, distribute new state to clients
        void setAngleOffset(const int offset);

        // => Auto-rotate to to given angle, if use-overlap is true choose shortest path to angle
        void rotateTo(const float angle, const bool use_overlap);

        // => Stop auto rotation if target has been reached.
        // To be called continously from main loop during auto rotation.
        // If angular-speed is zero 3s into auto-rotation, start 3s timeout.
        void watchAutoRotation();

        // => Update rotor values from ADC and calculate angular speed
        void update(bool with_angular_speed = false);
    };
}

#endif //ROTORCONTROLLER_H
