#ifndef ROTORSERVER_H
#define ROTORSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#include <globals.h>

namespace RotorServer {

    // ===== Config =====
    // ==================

    struct serverConfig {
        Preferences prefs;
        int port = sta_default_port;
        String user = sta_default_user;
        String password = sta_default_pw;
        const char* http_username = sta_default_user.c_str();
        const char* http_password = sta_default_pw.c_str();
    };

    extern struct serverConfig server_config;

    // => Load server config from Preferences
    void loadConfig();
    // => Save server config in Preferences
    bool saveConfig();
    // => Reset server config
    bool resetConfig();
    // => Print current server config to Serial
    void printConfig();


    // ===== Server =====
    // ==================

    // => Check a request for authentication
    void authenticateRequest(AsyncWebServerRequest * request);
    // => Add routes to server
    void addRoutes();
    // => Start server
    void startServer();
}

// Expose global server instance
extern AsyncWebServer *server;

#endif //ROTORSERVER_H
