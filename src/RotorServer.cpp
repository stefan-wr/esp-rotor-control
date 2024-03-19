#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include <globals.h>
#include <RotorController.h>  // Exposes Global: rotor_ctrl
#include <WiFiFunctions.h>
#include <RotorServer.h>
#include <RotorSocket.h>      // Exposes Global: websocket
#include <Firmware.h>         // Exposes Global: firmware

#define CONFIG_PREFS_KEY "serverPrefs"

namespace RotorServer {

  // => Check an http request for authentication
  // @param request: Pointer to AsyncWebServerRequest
  // @return False if not authenticated but required, else true
  bool authenticateRequest(AsyncWebServerRequest *request) {
    if (rotor_server.config.authenticate) { 
      if (!request->authenticate(rotor_server.config.http_username,
                                rotor_server.config.http_password)) {
        request->requestAuthentication();
        return false;
      }
    } 
    return true;
  }


  // ==============================
  // Server Config
  // ==============================

  // => Load server config from Preferences
  void RotorServer::loadConfig() {
    if (!config_prefs.begin(CONFIG_PREFS_KEY, true) && verbose) {
      Serial.println("[Server] Could not load server configuration! Use default configuration instead.");
    }
    config.port = config_prefs.getInt("port", sta_default_port);
    config.user = config_prefs.getString("user", sta_default_user);
    config.password = config_prefs.getString("password", sta_default_pw);
    updateHttpCredentials();
    config_prefs.end();
  }

  // => Save server config in Preferences
  bool RotorServer::saveConfig() {
    if (!config_prefs.begin(CONFIG_PREFS_KEY, false)) {
      Serial.println("[Server] Error: Could not save server configuration!");
      return false;
    };
    config_prefs.putInt("port", config.port);
    config_prefs.putString("user", config.user);
    config_prefs.putString("password", config.password);
    config_prefs.end();
    return true;
  }

  // => Reset server config in Preferences
  bool RotorServer::resetConfig() {
    config.port = sta_default_port;
    config.user = sta_default_user;
    config.password = sta_default_pw;
    updateHttpCredentials();
    return saveConfig();
  }

  // => Print current server config to Serial
  void RotorServer::printConfig() {
    Serial.print("[Server] config from PREFS: (User) ");
    Serial.print(config.user);
    Serial.print(" | (PW) ");
    Serial.print(config.password);
    Serial.print(" | (Port) ");
    Serial.println(config.port);
  }

  // => Update http login variables
  void RotorServer::updateHttpCredentials() {
    config.http_username = config.user.c_str();
    config.http_password = config.password.c_str();
    if (config.password == "") {
      config.authenticate = false;
    }
  }


  // ==============================
  // Server
  // ==============================

  // => Init server
  void RotorServer::init() {
    loadConfig();
    server = new AsyncWebServer(config.port);
    addRoutes();
    RotorSocket::initWebsocket();
    server->addHandler(&websocket);
    server->begin();
  }


  // ***********************
  // => Add routes to server
  // ***********************
  void RotorServer::addRoutes() {
    /*
    // Add headers for CORS preflight request when uploading firmware from VUE dev environment
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Token");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
    DefaultHeaders::Instance().addHeader("Access-Control-Max-Age:", "10");
    DefaultHeaders::Instance().addHeader("Access-Control-Expose-Headers", "Token");
    */
    
  // Catch all route
    server->onNotFound([](AsyncWebServerRequest *request) {
      if (request->method() == HTTP_OPTIONS) {
          // CORS preflight
        request->send(200);
      } else {
        // Send root, necessary for page reloads in Vue-App (redirect won't work)
        if (!authenticateRequest(request)) { return; }
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/app/index.html.gzip", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      }
    });


    // Root route, Vue-App index file
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!authenticateRequest(request)) { return; }
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/app/index.html.gzip", "text/html");
      response->addHeader("Content-Encoding", "gzip");
      response->addHeader("cache-control", "private, max-age=86400");
      request->send(response);
    });


    // Fonts & favicons 
    server->serveStatic("/inter-regular.woff2", LittleFS, "/inter-regular.woff2").setCacheControl("public,max-age=31536000");
    server->serveStatic("/inter-700.woff2", LittleFS, "/inter-700.woff2").setCacheControl("public,max-age=31536000");
    server->serveStatic("/site.webmanifest", LittleFS, "/site.webmanifest").setCacheControl("public,max-age=31536000");
    server->serveStatic("/favicon.ico", LittleFS, "/favicon.ico").setCacheControl("public,max-age=31536000");
    server->serveStatic("/favicon-16x16.png", LittleFS, "/favicon-16x16.png").setCacheControl("public, max-age=31536000");
    server->serveStatic("/favicon-32x32.png", LittleFS, "/favicon-32x32.png").setCacheControl("public,max-age=31536000");
    server->serveStatic("/apple-touch-icon.png", LittleFS, "/apple-touch-icon.png").setCacheControl("public,max-age=31536000");
    server->serveStatic("/android-chrome-192x192.png", LittleFS, "/android-chrome-192x192.png").setCacheControl("public,max-age=31536000");
    server->serveStatic("/android-chrome-512x512.png", LittleFS, "/android-chrome-512x512.png").setCacheControl("public,max-age=31536000");


    // Disconnect ESP from network
    server->on("/disconnect", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!authenticateRequest(request)) { return; }
      request->send(200);
      WiFiFunctions::resetCredentials();
      delay(1000);
      ESP.restart();
    });


    // Reboot ESP
    server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!authenticateRequest(request)) { return; }
      request->send(200);
      delay(1000);
      ESP.restart();
    });


    // Authenticate
    server->on("/authenticate", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (rotor_server.config.authenticate) {
        if (!rotor_server.authentications ||
            !request->authenticate(rotor_server.config.http_username,
                                  rotor_server.config.http_password)) {
          rotor_server.authentications++;
          return request->requestAuthentication();
        }
      }
      rotor_server.authentications = 0;
      request->send(200);
    });


    // Request a firmware udate
    server->on("/request-update", HTTP_POST, Firmware::handleUpdateRequest);

    // Update firmware
    server->on("/update", HTTP_POST, Firmware::handleFirmwareResponse, Firmware::handleFirmwareUpload);    
  }
}

RotorServer::RotorServer rotor_server;
