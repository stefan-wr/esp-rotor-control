#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include <Settings.h>
#include <SimpleFS.h>


extern AsyncWebSocket websocket;
extern String wifi_ssid;
extern String esp_id;
extern String version;
extern bool has_screen;
extern bool use_screen;

namespace Settings {
    // Path to save favorites at in FS
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

    // Load favorites from FS
    void Favorites::load() {
        favs_buffer = readFromFS(favs_path);
    }

    // Save favorites to FS
    void Favorites::save() {
        writeToFS(favs_path, favs_buffer);
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

    // Send screen
    void sendScreen() {
        settings_buffer.reserve(30);
        settings_buffer = "SETTINGS|";

        StaticJsonDocument<16> doc;
        doc["useScreen"] = use_screen;

        serializeJson(doc, settings_buffer);
        websocket.textAll(settings_buffer); 
    }
}