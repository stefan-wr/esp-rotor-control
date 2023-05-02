#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebSocket socket;

namespace Settings {

    // Favorites Class
    // ***************
    class Favorites {
    private:
        String favs_buffer;
        void load();
        void save();
    public:
        Favorites();
        void init();
        void set(char* msg);
        void send();
    };
}

#endif //SETTINGS_H
