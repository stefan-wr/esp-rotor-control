#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include <globals.h>
#include <RotorController.h>    // Exposes Global: rotor_ctrl
#include <Settings.h>
#include <Favorites.h>
#include <Screen.h>             // Exposes Global: screen

#define SOCKET_URL "/ws"


extern Favorites favorites;
extern String lock_msg;

AsyncWebSocket websocket(SOCKET_URL);

namespace RotorSocket {
  // Number of connected socket clients
  int clients_connected;

  // Forward-declare functions
  void socketReceive(char* msg, const size_t &len);
  void onSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

  // => Add event handler to socket
  void initWebsocket() {
    websocket.onEvent(onSocketEvent);
  }

  // ********************
  // Socket event handler
  // ********************

  void onSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        ++clients_connected;

        // -----

        websocket.textAll(lock_msg);
        Settings::sendSettings();
        rotor_ctrl.messenger.sendSpeed();
        rotor_ctrl.messenger.sendCalibration();
        rotor_ctrl.messenger.sendLastRotation(false);
        favorites.send();

        // -----

        Serial.print("[Websocket] Client ");
        Serial.print(client->id());
        Serial.print(" connected with IP: ");
        Serial.println(client->remoteIP());
        break;

    case WS_EVT_DISCONNECT:
        --clients_connected;
        Serial.print("[Websocket] Client ");
        Serial.print(client->id());
        Serial.println(" disconnected.");
        break;

      case WS_EVT_ERROR:
        Serial.println("[WebSocket] Error received.");
        break;

      case WS_EVT_PONG:
        Serial.println("[WebSocket] Got pinged.");
        break;     

      case WS_EVT_DATA:
        AwsFrameInfo * info = (AwsFrameInfo*) arg;
        if(info->final && info->index == 0 && info->len == len){
          // The whole message is a single frame and was fully received
          if(info->opcode == WS_TEXT) {
            data[len] = 0;

            // Print message
            if (verbose) {
              Serial.print("[Websocket] Data, client ");
              Serial.print(client->id());
              Serial.print(": ");
              Serial.println((char*) data);
            }

            // Receive
            socketReceive((char*) data, len);

          } else {
            Serial.println("[Websocket] Binary data received unexpectedly.");
          }
        } else {
          Serial.println("[Websocket] Data recieved in multiple frames.");
        }
        break;
    }
  }    


  // ***********************
  // Socket receive function
  // ***********************

  void socketReceive(char* msg, const size_t &len) {
    // Separate message from identifier, separated by '|'
    int sep_idx;  // Index of separator
    for(sep_idx = 0; sep_idx < len; ++sep_idx) {
      if (msg[sep_idx] == '|') { break; }
    }

    // Split message by inserting null-terminator
    if (sep_idx != 0 && sep_idx != len) {
      msg[sep_idx] = '\0';
    } else {
      Serial.println("[Websocket] Error: Could not parse message.");
      return;
    }

    String identifier(msg);

    // ----- ROTOR -----
    // -----------------
    if (identifier == "ROTOR") {
      // Deserialize JSON
      StaticJsonDocument<50> doc;
      DeserializationError err = deserializeJson(doc, msg + sep_idx + 1);

      // Test wether deserialization succeeded
      if (err) {
        Serial.print("[Websocket] Error: JSON parse failed: ");
        Serial.println(err.f_str());
        return;
      }

      // \/\/ Unpack message \/\/
      // Rotation
      JsonVariant val;
      val = doc["rotation"];
      if (!val.isNull()) {
        switch (val.as<const int>()) {
          case 0: 
            rotor_ctrl.stop(); break;
          case -1:
            rotor_ctrl.startRotation(0); break;
          case 1:
            rotor_ctrl.startRotation(1); break;
        }
      }

      // Speed
      val = doc["speed"];
      if (!val.isNull()) {
        if (val < 0) {
          rotor_ctrl.setMaxSpeed(0);
        } else if (val > 100) {
          rotor_ctrl.setMaxSpeed(100);
        } else {
          rotor_ctrl.setMaxSpeed(val.as<const int>());
        }
      }

      // Auto-rotation request
      val = doc["target"];
      if (!val.isNull()) {
        JsonVariant use_overlap = doc["useOverlap"];
        JsonVariant use_smooth_speed = doc["useSmoothSpeed"];

        if (!use_overlap.isNull() && !use_smooth_speed.isNull()) {
          rotor_ctrl.rotateTo(val.as<const float>(),
                              use_overlap.as<const bool>(),
                              use_smooth_speed.as<const bool>());
        }
      }
    }

    // ----- CALIBRATION -----
    // -----------------------
    if (identifier == "CALIBRATION") {
      // Deserialize JSON
      StaticJsonDocument<100> doc;
      DeserializationError err = deserializeJson(doc, msg + sep_idx + 1);

      // Test wether deserialization succeeded
      if (err) {
        Serial.print("[Websocket] Error: JSON parse failed: ");
        Serial.println(err.f_str());
        return;
      }

      // \/\/ Unpack message \/\/
      // Calibration
      JsonVariant cal_u1 = doc["u1"];
      JsonVariant cal_u2 = doc["u2"];
      JsonVariant cal_a1 = doc["a1"];
      JsonVariant cal_a2 = doc["a2"];
      if (!cal_u1.isNull() && !cal_u2.isNull()
          && !cal_a1.isNull() && !cal_a2.isNull()) {
        rotor_ctrl.setCalibration(cal_u1.as<const float>(), cal_u2.as<const float>(),
                                  cal_a1.as<const float>(), cal_a2.as<const float>());
      }

      // Angle offset
      JsonVariant offset = doc["offset"];
      if (!offset.isNull()) {
        rotor_ctrl.setAngleOffset(offset.as<float>());
      }
    }

    // ----- SETTINGS -----
    // --------------------
    if (identifier == "SETTINGS") {
      // Deserialize JSON
      StaticJsonDocument<48> doc;
      DeserializationError err = deserializeJson(doc, msg + sep_idx + 1);

      // Test wether deserialization succeeded
      if (err) {
        Serial.print("[Websocket] Error: JSON parse failed: ");
        Serial.println(err.f_str());
        return;
      }

      // \/\/ Unpack message \/\/
      // Screen
      JsonVariant val;
      val = doc["useScreen"];
      if (!val.isNull()) {
        if (!val.as<const bool>()) {
          use_screen = false;
          screen.disable();
        } else {
          use_screen = true;
          screen.enable();
        }
        // Distribute to clients
        Settings::sendScreen();
      }
    }

    // ----- FAVORITES -----
    // ---------------------
    if(identifier == "FAVORITES") {
      // For favorites, just save the JSON message with
      // the identifier rejoined by inserting back '|' into msg
      msg[sep_idx] = '|';
      favorites.set(msg);
    }

    // ----- LOCK -----
    // ----------------
    if (identifier == "LOCK") {
      // For lock, just distribute message to all clients
      msg[sep_idx] = '|';
      lock_msg = (String) msg;
      websocket.textAll(msg);
    }
  }
}
