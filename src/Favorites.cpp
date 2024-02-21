#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <Favorites.h>
#include <SimpleFS.h>


extern AsyncWebSocket websocket; // Websocket instance from main.cpp

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
