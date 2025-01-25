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
    void save() const;
public:
    Favorites();
    void init();
    void set(char* msg);
    void send() const;
};

extern Favorites favorites;

#endif //FAVORITES_H
