#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include <globals.h>
#include <RotorController.h>  // Exposes Global: rotor_ctrl
#include <WiFiFunctions.h>
#include <RotorServer.h>
#include <RotorSocket.h>      // Exposes Global: websocket
#include <Firmware.h>         // Exposes Global: firmware

#include <AppIndex.h>
#include <AppAssets.h>

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
    config.port = config_prefs.getUShort("port", sta_default_port);
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
    config_prefs.putUShort("port", config.port);
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
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gzip, index_html_gzip_len);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      }
    });


    // Root route, Vue-App index file
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!authenticateRequest(request)) { return; }
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gzip, index_html_gzip_len);
      response->addHeader("Content-Encoding", "gzip");
      response->addHeader("cache-control", "private, max-age=86400");
      request->send(response);
    });


    // Fonts
    // -----
    server->on("/inter-regular.woff2", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "font/woff2", inter_regular_woff2, inter_regular_woff2_len);
      response->addHeader("cache-control", ASSET_CACHE_CONTROL);
      request->send(response);
    });

    server->on("/inter-700.woff2", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "font/woff2", inter_700_woff2, inter_700_woff2_len);
      response->addHeader("cache-control", ASSET_CACHE_CONTROL);
      request->send(response);
    });

    // Favicons
    // --------
    server->on("/favicon-16x16.png", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "image/png", favicon_16x16_png, favicon_16x16_png_len);
      response->addHeader("cache-control", ASSET_CACHE_CONTROL);
      request->send(response);
    });

    server->on("/favicon-32x32.png", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "image/png", favicon_32x32_png, favicon_32x32_png_len);
      response->addHeader("cache-control", ASSET_CACHE_CONTROL);
      request->send(response);
    });

    server->on("/apple-touch-icon.png", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "image/png", apple_touch_icon_png, apple_touch_icon_png_len);
      response->addHeader("cache-control", ASSET_CACHE_CONTROL);
      request->send(response);
    });

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
