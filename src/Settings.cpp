#include <Settings.h>
#include <SimpleSPIFFS.h>
#include <ArduinoJson.h>

extern String wifi_ssid;
extern String esp_id;

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

    // Set and save favorites from message
    void Favorites::set(char* msg) {
        favs_buffer = (String) msg;
        save();
        send();
    }

    // Send favorites to clients
    void Favorites::send() {
        socket.textAll(favs_buffer);
    }


    // **************
    // Other Settings
    // **************
    String settings_buffer;

    // Send settings
    void sendSettings() {
        settings_buffer.reserve(90);
        settings_buffer = "SETTINGS|";

        StaticJsonDocument<128> doc;
        doc["esp_id"] = esp_id; 
        doc["ssid"] = wifi_ssid;
        doc["rssi"] = (String)WiFi.RSSI();

        serializeJson(doc, settings_buffer);
        socket.textAll(settings_buffer);
    }
}