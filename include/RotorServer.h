#ifndef ROTORSERVER_H
#define ROTORSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#include <globals.h>


namespace RotorServer {

    // => Check an http request for authentication
    // @param request: Pointer to AsyncWebServerRequest
    // @return False if not authenticated but required, else true
    bool authenticateRequest(AsyncWebServerRequest *request);

    // Rotor Server Class
    // ------------------
    // - wraps AsyncWebServer
    class RotorServer {
    private:
        Preferences config_prefs;

        // => Add routes to server
        void addRoutes();

        // => Update http login variables
        void updateHttpCredentials();

    public:
        // Server
        AsyncWebServer *server;

        // Configuration
        struct {
            int port = sta_default_port;
            String user = sta_default_user;
            String password = sta_default_pw;
            const char* http_username = sta_default_user.c_str();
            const char* http_password = sta_default_pw.c_str();
            bool authenticate = true;
        } config;

        // Number of authentications, used in "/authenticate" route
        uint8_t authentications = 0;

        // => Load server config from Preferences
        void loadConfig();
        // => Save server config in Preferences
        bool saveConfig();
        // => Reset server config
        bool resetConfig();
        // => Print current server config to Serial
        void printConfig();

        // Constructor
        RotorServer() {};
        // => Init server
        void init();
    };
}

// Expose global server instance
extern RotorServer::RotorServer rotor_server;

#endif //ROTORSERVER_H
