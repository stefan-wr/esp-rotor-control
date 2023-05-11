#ifndef ROTORCONTROLLER_H
#define ROTORCONTROLLER_H

#include <Arduino.h>
#include <globals.h>
#include <Adafruit_ADS1X15.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

extern AsyncWebSocket socket;

namespace Rotor {

    class Rotation;
    class Messenger;
    class RotorController;

    // Rotor Rotation Class
    // ********************
    class Rotation {
    private:
        Adafruit_ADS1115 adc;
        const int adc_channel = 0;
        bool ads_failed = false;

    public:
        int last_adc_value = 0;
        float last_adc_volts = 0.0;
        float last_angle = 0.0;
        int last_speed = 30;

        struct {
            float u1, u2, a1, a2;
            float d_grad, u_0;
            const float volt_div_factor = 1.5;
            float offset;
        } calibration;

        Rotation();
        void init();
        void calibrate(const float &u1, const float &u2,
                       const float &a1, const float &a2);
        void applyCalibration();
        void saveCalibration();
        void loadCalibration();
        
        void setAngleOffset(const float &offset);
        int getADCValue();
        float getADCVolts();
        float getAngle();
        void update();
        void startRotation(const int &dir);
        void stopRotor();
        void setSpeedDAC(const int &speed);
    };


    // Rotor Messenger Class
    // *********************
    class Messenger {
    private:
        String msg_buffer;
        void setLastRotationMsg(const bool &with_angle);
    public:
        RotorController* rotor_ptr;     // Pointer to parent class instace
        Messenger();
        void sendLastRotation(const bool &with_angle);
        void sendNewRotation(const bool &with_angle);
        void sendSpeed();
        void sendCalibration();
        void sendTarget();
    };
    

    // Rotor Controller Class
    // **********************
    class RotorController {
    private:
        void startAutoRotation(const int dir);
    public:
        bool is_rotating = false;
        int direction = 0;  // 0: CCW, 1: CW
        int speed = 0;
        int max_angle = 449;
        int min_rotation_angle = 2;
        bool is_auto_rotating = false;
        float auto_rotation_target;
        float auto_rotation_buffer = 0.7;

        Messenger messenger;
        Rotation rotor;

        RotorController() {};
        void init();
        void startRotation(const int dir);
        void stop();
        void setSpeed(const int spd);
        void setCalibration(const float u1, const float u2,
                            const float a1, const float a2);
        void setAngleOffset(const int offset);
        void rotateTo(const float angle, const bool use_overlap);
        void watchAutoRotation();
    };
}

#endif //ROTORCONTROLLER_H
