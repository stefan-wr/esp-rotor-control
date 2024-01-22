#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include <Settings.h>
#include <SimpleSPIFFS.h>


extern AsyncWebSocket websocket;
extern String wifi_ssid;
extern String esp_id;
extern String version;

namespace Settings {
    // Path to save favorites at in SPIFFS
    const char* favs_path = "/favorites.json";


    // ******************************
    // Define Favorites class members
    // ******************************
    Favorites::Favorites() {
        favs_buffer.reserve(600);
    }

    // Init buffer
    void Favorites::init() {
        load();
    }

    // Load favorites from SPIFFFs
    void Favorites::load() {
        favs_buffer = readFromSPIFFS(favs_path);
    }

    // Save favorites to SPIFFs
    void Favorites::save() {
        writeToSPIFFS(favs_path, favs_buffer);
    }

    // Set, save and send favorites from message
    void Favorites::set(char* msg) {
        favs_buffer = (String) msg;
        save();
        send();
    }

    // Send favorites to clients
    void Favorites::send() {
        websocket.textAll(favs_buffer);
    }


    // **************
    // Other Settings
    // **************
    String settings_buffer;

    // Send settings
    void sendSettings() {
        settings_buffer.reserve(110);
        settings_buffer = "SETTINGS|";

        StaticJsonDocument<200> doc;
        doc["version"] = version;
        doc["esp_id"] = esp_id;
        doc["ssid"] = wifi_ssid;
        doc["rssi"] = (String)WiFi.RSSI();

        serializeJson(doc, settings_buffer);
        websocket.textAll(settings_buffer);
    }
}