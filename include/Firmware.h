#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>

#include <Timer.h>

namespace Firmware {

    class Firmware {
    public:
        size_t size;
        String md5;
        bool is_updating;
        uint8_t upload_progress;
        Timer timeout;
        String token;
        Timer token_timeout;

        // Constrcutor
        Firmware() {};

        // => Init firmware
        void init();

        // => Generate a random token string, default length = 32
        void generateToken(const int &len = 32);
    };

    // => Serial print update error
    void printUpdateError();

    // => Handler for update request
    void handleUpdateRequest(AsyncWebServerRequest *request);

    // => Upload handler for firmware upload via HTTP Post multipart/formdata
    void handleFirmwareUpload(AsyncWebServerRequest *request, String filename,
                            size_t index, uint8_t *data, size_t len, bool final);

    // => Handler for when upload finished
    void handleFirmwareResponse(AsyncWebServerRequest *request);
}

extern Firmware::Firmware firmware;

#endif //FIRMWARE_H
