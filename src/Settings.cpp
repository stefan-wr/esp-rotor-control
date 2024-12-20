#include <Arduino.h>
#include <ArduinoJson.h>

#include <Settings.h>
#include <globals.h>
#include <WiFiFunctions.h>
#include <RotorSocket.h>

#define SETTINGS_BUFFER_SIZE 256

namespace Settings {
    String settings_buffer;

    // => Reserve message buffer
    void initBuffer() {
        settings_buffer.reserve(SETTINGS_BUFFER_SIZE);
    }

    // => Send general settings to clients
    void sendSettings() {
        settings_buffer = MSG_ID_SETTINGS;
        settings_buffer += "|"; 

        StaticJsonDocument<192> doc;
        doc["version"] = version;
        doc["espID"] = esp_id;
        doc["ssid"] = WiFiFunctions::wifi_config.ssid;
        doc["rssi"] = (String)WiFi.RSSI();
        doc["hasScreen"] = has_screen;
        doc["useScreen"] = use_screen;
        doc["md5"] = ESP.getSketchMD5();

        serializeJson(doc, settings_buffer);
        websocket.textAll(settings_buffer);
    }

    // => Send screen setting to clients
    void sendScreen() {
        settings_buffer = MSG_ID_SETTINGS;
        settings_buffer += "|"; 

        StaticJsonDocument<16> doc;
        doc["useScreen"] = use_screen;

        serializeJson(doc, settings_buffer);
        websocket.textAll(settings_buffer); 
    }
}
