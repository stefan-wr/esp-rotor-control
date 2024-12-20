#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>

#include <Firmware.h>       
#include <globals.h>
#include <BlinkingLED.h>
#include <Timer.h>
#include <RotorServer.h>

#define MAX_FIRMWARE_SIZE 1638400

extern BlinkingLED wifi_led;

namespace Firmware {

    // => Init firmware
    void Firmware::init() {
        size = 0;
        is_updating = false;
        upload_progress = 0;
        timeout.changeInterval(1000 * 60 * 2);         // 2 min
        token_timeout.changeInterval(1000 * 60 * 2);   // 2 min
        md5.reserve(32);
        token.reserve(32);
        md5 = "";
        token = "";
    }

    // => Generate a random token string, default length = 32
    void Firmware::generateToken(const int &len) {
        const char* characters = "abcdefghijklm0123456789nopqrstuvwxyz";
        char buffer[len + 1];
        buffer[len] = '\0';
        for(int i = 0; i < len; i++){
            const int index =  (int) round((double) esp_random() / UINT32_MAX * 35);
            buffer[i] = characters[index];
        }
        token = buffer;
        token_timeout.start();
    }

    // => Serial print update error
    void printUpdateError() {
        Serial.print("[ESP] Update error: ");
        Serial.println(Update.errorString());
    }


    // => Handler for update request
    // -----------------------------
    void handleUpdateRequest(AsyncWebServerRequest *request) {
        if (!RotorServer::authenticateRequest(request)) { return; }

        firmware.md5 = "";
        firmware.size = 0;
        firmware.token = "";

        // Get MD5 and size
        if (request->hasParam("md5", true) && request->hasParam("size", true)) {
            firmware.md5 = request->getParam("md5", true)->value();
            firmware.size = request->getParam("size", true)->value().toInt();

            // MD5 has incorrect length
            if (firmware.md5.length() != 32) {
                Serial.println("[ESP] Firmware update request denied, wrong MD5 length.");
                return request->send(200, "text/plain", "denied");
            }

            // File size not suitable
            if (0 >= firmware.size > MAX_FIRMWARE_SIZE) {
                Serial.println("[ESP] Firmware update request denied, file size unsuitable.");
                return request->send(200, "text/plain", "denied");
            }

            // Request approved
            firmware.generateToken();
            Serial.println("[ESP] Firmware update request approved, token generated.");
            return request->send(200, "text/plain", firmware.token);
        } else {
            Serial.println("[ESP] Firmware update request denied, missing MD5 or size.");
            return request->send(200, "text/plain", "denied");
        }
    }


    // => Upload handler for firmware upload via HTTP Post multipart/formdata
    // ----------------------------------------------------------------------
    void handleFirmwareUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        
        // First data packet
        // *****************    
        if (!index) {
            // Init update
            firmware.upload_progress = 1;
            firmware.is_updating = true;

            // Initialise update
            if (!Update.begin(firmware.size)) {
                printUpdateError();
            }

            // Abort update if token is missing or expired
            if (!Update.hasError() && request->hasHeader("Token")) {
                if (request->getHeader("Token")->value() != firmware.token || firmware.token_timeout.passed(false)) {
                    Update.abort();
                    Serial.println("[ESP] Update token does not match.");
                }
            } else {
                Update.abort(); // creates error -> Update.hasError() => true
                Serial.println("[ESP] Update token missing.");            
            }

            // Set target MD5
            if (!Update.hasError()) {
                if (!Update.setMD5(firmware.md5.c_str())) {
                    printUpdateError();
                }
            }

            // Log to serial
            if (!Update.hasError()) {
                Serial.print("[ESP] Start firmware update from file: ");
                Serial.println(filename.c_str());
                Serial.print("[ESP] Expected MD5: ");
                Serial.println(firmware.md5);
                Serial.print("[ESP] Expected size (Bytes): ");
                Serial.println(firmware.size);
            }       
        }

        // Every packet, write to partition
        // ********************************
        if (!Update.hasError()) {        
            // Write data to partition
            if (Update.write(data, len) != len) {
                Serial.println("[ESP] Error while writing firmware.");
            }

            // Reset timeout
            firmware.timeout.start();

            // Update progress
            firmware.upload_progress = floor((float) index / (float) firmware.size * 100.0f);
        }

        // Last data packet received
        // *************************
        if (final) {
            firmware.upload_progress = 100;
            
            // Conclude update
            size_t bytesWritten = Update.progress();
            if (Update.end(true)) {
                Serial.print("[ESP] Update succesful | MD5: ");
                Serial.print(Update.md5String());
                Serial.print(" | Bytes written: ");
                Serial.println(bytesWritten);
            } else {
                printUpdateError();
            }
        }
    }


    // => Handler for when upload finished
    // -----------------------------------
    void handleFirmwareResponse(AsyncWebServerRequest *request) {
        if (!Update.hasError()) {
            request->send(200, "text/plain", "success");
            Serial.println("[ESP] Restart in 1 second.");
            delay(1000);
            ESP.restart();
        } else {
            request->send(200, "text/plain", Update.errorString());
            wifi_led.blinkBlocking(10, 100ul);
        }
            
        firmware.is_updating = false;
    }
}

Firmware::Firmware firmware;
