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

AsyncWebServer *server;

namespace RotorServer {
  
  // ==============================
  // Server Config
  // ==============================

  struct serverConfig server_config;

  // => Update login credentials variables
  void updateHttpCredentials() {
    server_config.http_username = server_config.user.c_str();
    server_config.http_password = server_config.password.c_str();
  }

  // => Load server config from Preferences
  void loadConfig() {
    if (!server_config.prefs.begin(CONFIG_PREFS_KEY, true) && verbose) {
      Serial.println("[Server] Could not load server configuration! Use default configuration instead.");
    }
    server_config.port = server_config.prefs.getInt("port", sta_default_port);
    server_config.user = server_config.prefs.getString("user", sta_default_user);
    server_config.password = server_config.prefs.getString("password", sta_default_pw);
    updateHttpCredentials();
    server_config.prefs.end();
  }

  // => Save server config in Preferences
  bool saveConfig() {
    if (!server_config.prefs.begin(CONFIG_PREFS_KEY, false)) {
      Serial.println("[Server] Error: Could not save server configuration!");
      return false;
    };
    server_config.prefs.putInt("port", server_config.port);
    server_config.prefs.putString("user", server_config.user);
    server_config.prefs.putString("password", server_config.password);
    server_config.prefs.end();
    return true;
  }

  // => Reset server config in Preferences
  bool resetConfig() {
    server_config.port = sta_default_port;
    server_config.user = sta_default_user;
    server_config.password = sta_default_pw;
    updateHttpCredentials();
    return saveConfig();
  }

  // => Print current server config to Serial
  void printConfig() {
    Serial.print("[Server] config from PREFS: (User) ");
    Serial.print(server_config.user);
    Serial.print(" | (PW) ");
    Serial.print(server_config.password);
    Serial.print(" | (Port) ");
    Serial.println(server_config.port);
  }

  // ==============================
  // Server
  // ==============================

  // Number of authentications, used in "/authenticate" URL
  uint8_t authentications = 0;

  // => Check a request for authentication
  void authenticateRequest(AsyncWebServerRequest * request) {
    if (server_config.password != "" && !request->authenticate(server_config.http_username, server_config.http_password))
      return request->requestAuthentication();
  }

  // => Start server
  void startServer() {
    server = new AsyncWebServer(RotorServer::server_config.port);
    addRoutes();
    RotorSocket::initWebsocket();
    server->addHandler(&websocket);
    server->begin();
  }

  // ***********************
  // => Add routes to server
  // ***********************
  void addRoutes() {
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
        RotorServer::authenticateRequest(request);
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/app/index.html.gzip", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      }
    });


    // Root route, Vue-App index file
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      RotorServer::authenticateRequest(request);
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
      RotorServer::authenticateRequest(request);
      request->send(200);
      resetCredentials();
      delay(1000);
      ESP.restart();
    });


    // Reboot ESP
    server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest* request) {
      RotorServer::authenticateRequest(request);
      request->send(200);
      delay(1000);
      ESP.restart();
    });


    // Authenticate
    server->on("/authenticate", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (RotorServer::server_config.password != "") {
        if (!RotorServer::authentications || !request->authenticate(RotorServer::server_config.http_username, RotorServer::server_config.http_password)) {
          RotorServer::authentications++;
          return request->requestAuthentication();
        }
      }
      RotorServer::authentications = 0;
      request->send(200);
    });


    // Request a firmware udate
    server->on("/request-update", HTTP_POST, Firmware::handleUpdateRequest);

    // Update firmware
    server->on("/update", HTTP_POST, Firmware::handleFirmwareResponse, Firmware::handleFirmwareUpload);    
  }
}
