#include <Arduino.h>
#include <ArduinoJson.h>

#include <globals.h>
#include <RotorController.h>
#include <RotorMessenger.h>
#include <RotorSocket.h>        // Exposes Global: websocket

#define ROTORSOCKET_BUFFER_SIZE 100

namespace Rotor {

    // ************************
    // Define Messenger members
    // ************************

    Messenger::Messenger() {
        // Set message buffer size on the heap
        msg_buffer.reserve(ROTORSOCKET_BUFFER_SIZE);
    }

    // => Create JSON message from last rotor values and save in msg_buffer
    void Messenger::setLastRotationMsg(const bool with_angle) {
        if (rotor_ptr != NULL) {
            // Buffers
            msg_buffer = MSG_ID_ROTOR;
            msg_buffer += "|";
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
    void Messenger::sendLastRotation(const bool with_angle) {
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
        msg_buffer = MSG_ID_ROTOR;
        msg_buffer += "|";
        StaticJsonDocument<16> doc;
        doc["speed"] = rotor_ptr->max_speed;
        serializeJson(doc, msg_buffer);
        websocket.textAll(msg_buffer);
    }

    // => Send current calibration parameters
    void Messenger::sendCalibration() {
        msg_buffer = MSG_ID_CALIBRATION;
        msg_buffer += "|";
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
        msg_buffer = MSG_ID_ROTOR;
        msg_buffer += "|";
        StaticJsonDocument<32> doc;
        doc["target"] = round(rotor_ptr->auto_rotation_target * 100.0) / 100.0;
        serializeJson(doc, msg_buffer);
        websocket.textAll(msg_buffer);
    }
}
