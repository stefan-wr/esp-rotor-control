#ifndef SETTINGS_H
#define SETTINGS_H

namespace Settings {
    extern String settings_buffer;

    // => Reserve message buffer
    void initBuffer();

    // => Send general settings to clients
    void sendSettings();

    // => Send screen setting to clients
    void sendScreen();

    // => Send on-time to clients
    void sendBootTime();
}

#endif //SETTINGS_H
