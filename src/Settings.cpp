#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include <Settings.h>
#include <globals.h>
#include <WiFiFunctions.h>
#include <RotorSocket.h>


namespace Settings {
    String settings_buffer;

    // => Send general settings to clients
    void sendSettings() {
        settings_buffer.reserve(200);
        settings_buffer = "SETTINGS|";

        StaticJsonDocument<192> doc;
        doc["version"] = version;
        doc["espID"] = esp_id;
        doc["ssid"] = wifi_ssid;
        doc["rssi"] = (String)WiFi.RSSI();
        doc["hasScreen"] = has_screen;
        doc["useScreen"] = use_screen;

        serializeJson(doc, settings_buffer);
        websocket.textAll(settings_buffer);
    }

    // => Send screen setting to clients
    void sendScreen() {
        settings_buffer.reserve(30);
        settings_buffer = "SETTINGS|";

        StaticJsonDocument<16> doc;
        doc["useScreen"] = use_screen;

        serializeJson(doc, settings_buffer);
        websocket.textAll(settings_buffer); 
    }
}
