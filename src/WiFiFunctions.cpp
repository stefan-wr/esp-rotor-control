#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <Preferences.h>
#include <LittleFS.h>

#include <globals.h>
#include <WiFiFunctions.h>
#include <BlinkingLED.h>
#include <Timer.h>
#include <RotorServer.h>

#include <AppAssets.h>

extern BlinkingLED wifi_led;

namespace WiFiFunctions {

  // =============
  // Configuration
  // =============

  // PREFS instances
  Preferences wifi_prefs;

  // WiFi credentials
  struct WiFiConfig wifi_config;

    // => Convert wifi_bssid BSSID-String to uint8_t
  bool bssidToUint8() {
    int n_fields = sscanf(wifi_config.bssid.c_str(), "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",
                          &wifi_config.bssid_uint8t[0], &wifi_config.bssid_uint8t[1],
                          &wifi_config.bssid_uint8t[2], &wifi_config.bssid_uint8t[3],
                          &wifi_config.bssid_uint8t[4], &wifi_config.bssid_uint8t[5]);
    if (n_fields == 6 ) {
      return true;
    } else {
      return false;
    }
  }

  // => Save credentials to PREFS
  bool saveCredentials() {
    if (!wifi_prefs.begin("wifiPrefs", false)) {
      Serial.println("[WiFi] Error: Could not save WiFi credentials!");
      return false;
    }
    wifi_prefs.putString("ssid", wifi_config.ssid);
    wifi_prefs.putString("bssid", wifi_config.bssid);
    wifi_prefs.putString("pw", wifi_config.password);
    wifi_prefs.end();
    return true;
  }

  // => Load WiFi credentials from PREFS
  void loadCredentials() {
    if (!wifi_prefs.begin("wifiPrefs", true) && verbose) {
      Serial.println("[WiFi] Could not load WiFi credentials!");
      return;
    }
    wifi_config.ssid = wifi_prefs.getString("ssid", "");
    wifi_config.bssid = wifi_prefs.getString("bssid", "");
    wifi_config.password = wifi_prefs.getString("pw", "");
    wifi_prefs.end();
  }

  // => Clear WiFi credentials with empty strings
  bool resetCredentials() {
    if (!wifi_prefs.begin("wifiPrefs", false)) {
      Serial.println("[WiFi] Error: Could not reset WiFi credentials.");
      return false;
    }
    wifi_prefs.clear();
    wifi_prefs.end();
    return true;
  }


  // =========
  // Utilities
  // =========

  // => React to button interrupt for resetting WiFi credentials
  void resetCredentialsInterrupt() {
    Serial.print("[BTN] pressed. Waiting 2s to confirm...");
    delay(2000);
    if (!digitalRead(multi_button_pin)) {
      wifi_led.blinkBlocking(4, 250);
      Serial.println("[BTN] pressed. Resetting WiFi credentials and restart.");
      resetCredentials();
      ESP.restart();
    } else {
      multi_btn_pressed = false;
      Serial.println("failed.");
    }
  }

  // => Return ip url from current AP IP
  String get_ip_url() {
    return "http://" + WiFi.softAPIP().toString();
  }

  // => Init mDNS
  bool initMDNS() {
    if(!MDNS.begin(local_url)) {
        Serial.println("[WiFi] Error: Could not start mDNS");
        return false;
    }
    Serial.print("[Server] Local URL: http://");
    Serial.print(local_url);
    Serial.println(".local");
    return true;
  }


  // =============
  // Networks Scan
  // =============

  // AP mode buffers for alert messages and networks list
  String networks_html = "";
  String alert = "";


  // => Create HTML <li> item for a scanned network
  String networkItemHTML(int id, const String &ssid, const String &bssid, const int &rssi) {
    int buffer_size = sizeof(networkItem) - 12 + ssid.length() * 2 + bssid.length() + 10 + 48;
    char buffer[buffer_size];
    int written = snprintf(buffer, buffer_size, networkItem, id, ssid.c_str(), rssi, id, ssid.c_str(), bssid.c_str());

    if (written < buffer_size) {
      return buffer;
    } else {
      return "<li>Unidentifiziertes Netzwerk</li>";
    }
  }


  // => Start async scan for WiFi-networks
  void startNetworkScan() {
    Serial.println("[WiFi] Start async WiFi networks scan.");
    WiFi.scanNetworks(true);
  }

  // => Check for completion of async network scan and create <ul> of found networks
  // To be called from main loop
  void watchNetworkScan() {
    // Get scan state/result
    int16_t n = WiFi.scanComplete();

    if (n < 0) {
      // Scan in progress or not triggered
    } else if  (n == 0 ) {
      // No networks found
      networks_html = "<p>Keine verfügbaren Netzwerke gefunden.<br>Bitte warte etwas und lade dann diese Seite erneut.</p>";
    } else if (n > 0) {
      // At least one network found
      // Show found networks on serial monitor
      if (verbose) {
        if (n != 0) {
          Serial.println("--------------------------------------");
          for (int i = 0; i < n; ++i) {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" | ");
            Serial.print(WiFi.BSSIDstr(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.println(")");
          }
          Serial.println("");
        }
      }

      // Create network HTML-list
      networks_html = "<ul class=\"blocks\">";
      for (int i = 0; i < n; ++i) {
        networks_html += networkItemHTML(i, WiFi.SSID(i), WiFi.BSSIDstr(i), WiFi.RSSI(i));
      }
      networks_html += "</ul>";
      WiFi.scanDelete();
    }
  }

  // =================================================
  // Initialise WiFi connection from saved parameters.
  // =================================================

  // => Initialise WiFi connection from saved parameters.
  // @return true if success, false if connecting fails
  bool initWiFi() {
    // Load WiFi credentials from Prefs
    loadCredentials();
    Serial.print("[WiFi] from PREFS: (SSID) ");
    Serial.print(wifi_config.ssid);
    Serial.print(" {");
    Serial.print(wifi_config.bssid);
    Serial.print("} | (PW) ");
    Serial.println(wifi_config.password);

    // Check for SSID
    if (wifi_config.ssid == "") {
      Serial.println("[WiFi] Can not connect WiFi. SSID is missing.");
      return false;
    }

    // Check for MAC address
    if (wifi_config.bssid == "") {
      Serial.println("[WiFi] Can not connect WiFi. BSSID is missing.");
      return false;
    } else if (!bssidToUint8()) {
      Serial.println("[WiFi] Can not connect WiFi. BSSID conversion failed.");
      return false;
    }

    // Set WiFi mode
    if (!WiFi.mode(WIFI_STA)) {
      Serial.println("[WiFi] Error: Could not set WiFi to STATION mode.");
      return false;
    }

    // Set hostname
    WiFi.hostname("RotorControl-" + version);

    // Try repeatedly to connect to WiFi for 6 seconds
    // Reset when button pressed for two seconds
    uint8_t n_try = 1;
    Timer connect_timer(6000);  // 6 s

    // Connect WiFi, first try
    Serial.print("[WiFi] Connecting to ");
    Serial.print(wifi_config.ssid);
    Serial.print(" {");
    Serial.print(wifi_config.bssid);
    Serial.println("}");
    WiFi.begin(wifi_config.ssid.c_str(), wifi_config.password.c_str(), 0, wifi_config.bssid_uint8t);

    // Try continuously to connect WiFi
    while (!WiFi.isConnected()) {
      // Check for interrupt
      if (multi_btn_pressed) {
        resetCredentialsInterrupt();
      }

      // Retry after interval
      if (connect_timer.passed()) {
        Serial.print("[Try ");
        Serial.printf("%2d", n_try);
        Serial.print("] Failed to connect. WiFi-Status: ");
        Serial.println(WiFi.status());
        WiFi.disconnect();
        wifi_led.blinkBlocking(1, 250);
        WiFi.begin(wifi_config.ssid.c_str(), wifi_config.password.c_str(), 0, wifi_config.bssid_uint8t);
        n_try++;
      }

      // Reboot after 99 tries
      if (n_try >= 99) {
        Serial.println("[WiFi] Failed to connect 99 times. Rebooting device.");
        delay(1000);
        ESP.restart();
      }
    }

    // Enable IPv6
    /*
    if(WiFi.enableIpV6()) {
      Serial.print("[WiFi] Enabled IPv6.");
    }
    */


    // Connection established
    Serial.print("[Try ");
    Serial.printf("%2d", n_try);
    Serial.print("] Connection established. IP: ");
    Serial.println(WiFi.localIP());

    initMDNS();
    return true;
  }


  // ===============
  // Start AP server
  // ===============

  
  // => HTML template processor
  String processor(const String &var) {
    if (var == "ALERT") {
      return alert;
    }
    if (var == "USER") {
      return rotor_server.config.user;
    }
    if (var == "PASSWORD") {
      return rotor_server.config.password;
    }
    if (var == "PORT") {
      return String(rotor_server.config.port);
    }
    if (var == "NETWORKS") {
      return networks_html;
    }
    return String();
  }

  // => Start AP mode server and ask for local WiFi credentials.
  // @return true if success, false if server could no be started
  bool startAPServer(AsyncWebServer *server, DNSServer &dns_server) {
    in_station_mode = false;  // not connected to a WiFi network

    // Define server with standard HTTP port 80
    server = new AsyncWebServer(80);

    // Start WiFi in AP mode
    if (!WiFi.mode(WIFI_AP)) {
      Serial.println("[WiFi] Error: Could not set WiFi to AP mode!");
      return false;
    }

    // Set custom IP address
    if (use_custom_ip) {
      IPAddress local_ip(ip[0], ip[1], ip[2], ip[3]);
      IPAddress gateway(ip[0], ip[1], ip[2], ip[3]);
      IPAddress subnet(255, 255, 255, 0);
      if (!WiFi.softAPConfig(local_ip, gateway, subnet)){
        Serial.println("[WiFi] Error: Could not set AP configuration!");
        return false;
      }
    }

    // Enable mDNS
    initMDNS();
    
    // Start WiFI AP
    char ap_ssid[63];
    snprintf(ap_ssid, sizeof(ap_ssid), "%s-%s", ap_ssid_name, esp_id.c_str());
    if (!WiFi.softAP(ap_ssid, NULL)) {
      Serial.println("[WiFi] Error: Could not start AP!");
      return false;
    }
    Serial.print("[WiFi] Started access point at: (IP) ");
    Serial.print(WiFi.softAPIP());
    Serial.print(" | (URL) ");
    Serial.print(get_ip_url());
    Serial.print(":");
    Serial.println(80);

    // Start DNS Server for captive portal
    // Redirects all URLs to AP server
    dns_server.setTTL(300);
    dns_server.setErrorReplyCode(DNSReplyCode::NoError);
    if (dns_server.start(53, "*", WiFi.softAPIP())) {
      Serial.println("[DNS] Started DNS server for captive portal.");
    } else {
      Serial.println("[DNS] Error: Could not start DNS server for captive portal.");
    }

    // Load rotor server config
    rotor_server.loadConfig();
    rotor_server.printConfig();

    // --------------------------
    // Configure AP server routes
    // --------------------------

    // Root URL
    // --------
    
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      alert = "";
      request->send(LittleFS, "/ap-index.html", String(), false, processor);
    });

    // Set WiFi credentials
    // --------------------
    server->on("/network", HTTP_GET, [](AsyncWebServerRequest* request) {
      bool success = false;
      if (request->hasParam("pw") && request->hasParam("ssid") && request->hasParam("bssid")) {
        // Unpack parameters, WiFi credentials
        wifi_config.ssid = request->getParam("ssid")->value();
        wifi_config.bssid = request->getParam("bssid")->value();
        wifi_config.password = request->getParam("pw")->value();

        Serial.print("Received WiFI credentials: (SSID) ");
        Serial.print(wifi_config.ssid);
        Serial.print(" (");
        Serial.print(wifi_config.bssid);
        Serial.print(") | (PW) ");
        Serial.println(wifi_config.password);

        success = saveCredentials();
      }

      if (success) {
        // Send message and reboot ESP
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Der Rotor Controller startet sich jetzt neu und versucht sich mit dem Netzwerk %s {%s} zu verbinden.",
                 wifi_config.ssid.c_str(), wifi_config.bssid.c_str());
        request->send(200, "text/plain", msg);
        delay(2000);
        ESP.restart();
      } else {
        // Could not receive/save credentials
        alert += "Fehler: Die Netzwerkauswahl konnte nicht übernommen werden!<br>";
        request->send(LittleFS, "/ap-index.html", String(), false, processor);
      }
    });


    // Set server config
    // -----------------
    server->on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
      bool success = false;

      if (request->hasParam("user") && request->hasParam("pw") && request->hasParam("port")) {
        // Unpack parameters, server config
        rotor_server.config.user = request->getParam("user")->value();
        rotor_server.config.password = request->getParam("pw")->value();
        rotor_server.config.port = (uint16_t) request->getParam("port")->value().toInt();

        Serial.print("Received server config: (User) ");
        Serial.print(rotor_server.config.user);
        Serial.print(" | (PW) ");
        Serial.print(rotor_server.config.password);
        Serial.print(" | (Port) ");
        Serial.println(rotor_server.config.port);

        success = rotor_server.saveConfig();
      }

      // Could not receive/save server config
      if (success) {
        alert += "Neue Serverkonfiguration wurde übernommen.<br>";
      } else {
        alert += "Fehler: Die Serverkonfiguration konnte nicht übernommen werden!<br>";
      }
      request->send(LittleFS, "/ap-index.html", String(), false, processor);
    });


    // Reset server config
    // --------------------
    server->on("/reset", HTTP_GET, [](AsyncWebServerRequest* request) {
      // Reset server config
      bool success = rotor_server.resetConfig();

      if (success) {
        Serial.print("Reset server config to: (User) ");
        Serial.print(rotor_server.config.user);
        Serial.print(" | (PW) ");
        Serial.print(rotor_server.config.password);
        Serial.print(" | (Port) ");
        Serial.println(rotor_server.config.port);
        alert += "Die Serverkonfiguration wurde zurückgesetzt.<br>";
      } else {
        Serial.println("Received request to reset server config | Error: Could not reset server configuration.");
        alert += "Die Serverkonfiguration konnte nicht zurückgesetzt.<br>";
      }
      request->send(LittleFS, "/ap-index.html", String(), false, processor);
    });


    // Trigger a rescan for networks
    // -----------------------------
    server->on("/rescan", HTTP_ANY, [](AsyncWebServerRequest* request) {
      request->redirect("/");
      scan_now = true;
    });


    // Catch all route
    // ---------------
    server->onNotFound([](AsyncWebServerRequest *request){
      request->redirect(get_ip_url());
    });


    // Static files (CSS, fonts)
    // -------------------------
    /*server->serveStatic("/ap-styles.css", LittleFS, "/ap-styles.css");
    server->serveStatic("/inter-regular.woff2", LittleFS, "/inter-regular.woff2");
    server->serveStatic("/inter-700.woff2", LittleFS, "/inter-700.woff2");
    server->on("/favicon.ico",[](AsyncWebServerRequest *request){request->send(404);});
    */
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


    // Captive Portal responses
    // ------------------------
    // Required
    server->on("/connecttest.txt",[](AsyncWebServerRequest *request){request->redirect("http://logout.net");}); // windows 11 captive portal workaround
    server->on("/wpad.dat",[](AsyncWebServerRequest *request){request->send(404);});                            // honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32
    //-----
    server->on("/generate_204",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});         // android captive portal redirect
    server->on("/redirect",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});             // microsoft redirect
    server->on("/hotspot-detect.html",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});  // apple call home
    server->on("/canonical.html",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});       // firefox captive portal call home
    server->on("/success.txt",[](AsyncWebServerRequest *request){request->send(200);});                       // firefox captive portal call home
    server->on("/ncsi.txt",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});             // windows call home

    // Start server
    server->begin();
    Serial.println("[Server] started in AP mode.\n\r");

    // Start first network scan
    scan_now = true;

    return true;
  }
}
