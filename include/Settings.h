#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

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

    // Other Settings
    // **************
    void sendSettings();
    void sendScreen();
}

#endif //SETTINGS_H
