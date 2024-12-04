#include <Arduino.h>
#include <Favorites.h>
#include <SimpleFS.h>
#include <RotorSocket.h>

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
    favs_buffer = readFromFS(FAVORITES_PATH);
}

// Save favorites to FS
void Favorites::save() {
    writeToFS(FAVORITES_PATH, favs_buffer);
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
