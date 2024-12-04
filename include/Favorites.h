#ifndef FAVORITES_H
#define FAVORITES_H

#include <Arduino.h>

#define FAVORITES_PATH "/favorites.json"

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

#endif //FAVORITES_H
