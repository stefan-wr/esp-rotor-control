#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>

#include <SimpleUpdater.h>
#include <globals.h>
#include <Timer.h>

extern const char* http_username;
extern const char* http_password;
extern bool authenticate;

bool updating_firmware = false;
unsigned int update_progress = 0;
Timer updateTimeout(1000 * 60 * 2);

size_t fw_size = UPDATE_SIZE_UNKNOWN;
const char *fw_md5;
bool request_has_md5 = false;
bool requestHasSize = false;

// => Upload handler for firmware upload via HTTP Post multipart/formdata
void handleUpdateUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (authenticate && !request->authenticate(http_username, http_password))
        return request->requestAuthentication();

    // First data packet
    // -----------------
    if (!index) {
        fw_size = UPDATE_SIZE_UNKNOWN;
        fw_md5 = "";
        request_has_md5 = false;
        requestHasSize = false;
        update_progress = 0;
        updating_firmware = true;
        wifi_led.startBlinking(500);
        updateTimeout.start();

        // Get MD5 from header
        if (request->hasHeader("Firmware-MD5")) {
            AsyncWebHeader *header = request->getHeader("Firmware-MD5");
            fw_md5 = header->value().c_str();
            request_has_md5 = true;
        }

        // Get size from header
        if (request->hasHeader("Firmware-Size")) {
            AsyncWebHeader *header = request->getHeader("Firmware-Size");
            fw_size = header->value().toInt();
            requestHasSize = true;
        }

        // Init update
        if (!Update.begin(fw_size)) {
            Update.printError(Serial);
        }

        // Set target MD5
        if (request_has_md5) {
            if (!Update.setMD5(fw_md5)) {
                Serial.println("[ESP] Update error: Could not set MD5 hash.");
                Update.abort();
            }
        }

        // Log to serial
        if (!Update.hasError()) {
            Serial.printf("[ESP] Start firmware update from file: '%s'\n", filename.c_str());
            if (request_has_md5) {
                Serial.print("[ESP] Expected MD5: ");
                Serial.println(fw_md5);
            }
            if (requestHasSize) {
                Serial.print("[ESP] Expected size (Bytes): ");
                Serial.println(fw_size);
            } 
        }       
    }

    // Every packet, write to partition
    // --------------------------------
    if (!Update.hasError()) {        
        // Write data to partition
        if (Update.write(data, len) != len) {
            Serial.print("[ESP] Update error: ");
            Serial.println(Update.errorString());
        }

        // Reset timeout
        updateTimeout.start();

        // Update progress
        if (requestHasSize) {
            update_progress = floor((float) index / (float) fw_size * 100.0f);
        }
    } else {
        // Stop update if error occured
        //Update.abort();
    }


    // Last data packet received
    // -------------------------
    if (final) {
        update_progress = 100;
        
        // Conclude update
        if (Update.end(true)) {
            Serial.print("[ESP] Update succesful | MD5: ");
            Serial.print(Update.md5String());
            Serial.print(" | Bytes written: ");
            Serial.println(fw_size);
        } else {
            Serial.print("[ESP] Update error: ");
            Serial.println(Update.errorString());
        }
    }
}
