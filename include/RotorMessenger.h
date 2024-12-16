#ifndef ROTORMESSENGER_H
#define ROTORMESSENGER_H

#include <Arduino.h>


namespace Rotor {

    class RotorController;

    // Rotor Messenger Class
    // *********************
    // Handles outgoing messages with rotor data
    class Messenger {
    private:
        String msg_buffer;

        // => Create rotation JSON message from last rotor values and save in msg_buffer
        void setLastRotationMsg(const bool &with_angle);

    public:
        // Pointer to rotor instance, declared in parent class
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
}

#endif //ROTORMESSENGER_H
