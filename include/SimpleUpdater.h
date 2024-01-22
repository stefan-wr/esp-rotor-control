#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Timer.h>

extern bool updating_firmware;
extern unsigned int update_progress;
extern Timer updateTimeout;

// => Upload handler for firmware upload via HTTP Post multipart/formdata
void handleUpdateUpload(AsyncWebServerRequest *request, String filename,
                        size_t index, uint8_t *data, size_t len, bool final);
