#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <Preferences.h>
#include <SPIFFS.h>

#include <globals.h>
#include <WiFiFunctions.h>
#include <Timer.h>
#include <BlinkingLED.h>


// PREFS instances
Preferences wifi_prefs;
Preferences server_prefs;

// AP mode buffers for alert messages and list of networks
String networks_html = "";
String alert = "";

// STATION mode WiFi-credentials
String wifi_ssid;
String wifi_bssid;
String wifi_pw;
uint8_t wifi_bssid_uint8[6];

// STATION server config
int sta_port;
String sta_user_str;
String sta_pw_str;


// ----------------------------------- Server Config

// => Save server config in PREFS
bool saveServerConfig() {
  if (!server_prefs.begin("serverPrefs", false)) {
    Serial.println("[Server] Error: Could not save server configuration!");
    return false;
  };
  server_prefs.putInt("port", sta_port);
  server_prefs.putString("user", sta_user_str);
  server_prefs.putString("password", sta_pw_str);
  server_prefs.end();
  return true;
}

// => Load server config from PREFS
void loadServerConfig() {
  if (!server_prefs.begin("serverPrefs", true) && verbose) {
    Serial.println("[Server] Could not load server configuration! Use default configuration instead.");
  }
  sta_port = server_prefs.getInt("port", sta_default_port);
  sta_user_str = server_prefs.getString("user", sta_default_user);
  sta_pw_str = server_prefs.getString("password", sta_default_pw);
  server_prefs.end();
}

// => Reset saved server config to default
bool resetServerConfig() {
  sta_port = sta_default_port;
  sta_user_str = sta_default_user;
  sta_pw_str = sta_default_pw;
  return saveServerConfig();
}

// ----------------------------------- WiFi Credentials

// => Save credentials to PREFS
bool saveCredentials() {
  if (!wifi_prefs.begin("wifiPrefs", false)) {
    Serial.println("[WiFi] Error: Could not save WiFi credentials!");
    return false;
  }
  wifi_prefs.putString("ssid", wifi_ssid);
  wifi_prefs.putString("bssid", wifi_bssid);
  wifi_prefs.putString("pw", wifi_pw);
  wifi_prefs.end();
  return true;
}

// => Load WiFi credentials from PREFS
void loadCredentials() {
  if (!wifi_prefs.begin("wifiPrefs", true && verbose)) {
    Serial.println("[WiFi] Could not load WiFi credentials!");
    return;
  }
  wifi_ssid = wifi_prefs.getString("ssid", "");
  wifi_bssid = wifi_prefs.getString("bssid", "");
  wifi_pw = wifi_prefs.getString("pw", "");
  wifi_prefs.end();
}

// => Overwrite the saved credentials with empty strings
bool resetCredentials() {
  if (!wifi_prefs.begin("wifiPrefs", false)) {
    Serial.println("[WiFi] Error: Could not reset WiFi credentials.");
    return false;
  }
  wifi_prefs.clear();
  wifi_prefs.end();
  return true;
}

// -----------------------------------

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

// => Convert wifi_bssid BSSID-String to uint8_t
bool bssidToUint8() {
  if (6 == sscanf(wifi_bssid.c_str(), "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", &wifi_bssid_uint8[0], &wifi_bssid_uint8[1], &wifi_bssid_uint8[2], &wifi_bssid_uint8[3], &wifi_bssid_uint8[4], &wifi_bssid_uint8[5])) {
    return true;
  } else {
    return false;
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
  return true;
}

// => HTML template processor
// **************************
String processor(const String &var) {
  if (var == "ALERT") {
    return alert;
  }
  if (var == "USER") {
    return sta_user_str;
  }
  if (var == "PASSWORD") {
    return sta_pw_str;
  }
  if (var == "PORT") {
    return String(sta_port);
  }
  if (var == "NETWORKS") {
    return networks_html;
  }
  return String();
}


// => Create HTML <li> item for a scanned network
// **********************************************
String networkItemHTML(int id, const String &ssid, const String &bssid, const int &rssi) {
  String s = "";
  s += "<li><form class=\"network-form\" action=\"/network\" onsubmit=\"setDotsById('nw-btn-" + String(id) + "')\">";
  s += "<span class=\"l-align wrd-break\">" + ssid + "</span>";
  s += "<span class=\"r-align small\">RSSI: " + String(rssi) + "</span>";
  s += "<input type=\"password\" placeholder=\"Passwort eingeben\" name=\"pw\" required>";
  s += "<button id=\"nw-btn-" + String(id) + "\" type=\"submit\" class=\"small\">Verbinden</button>";
  s += "<input type=\"hidden\" name=\"ssid\" value=\"" + ssid + "\">";
  s += "<input type=\"hidden\" name=\"bssid\" value=\"" + bssid + "\">";
  s += "</form></li>";
  return s;
}


// => Scan for WiFi-networks and create <ul> of found networks
// ***********************************************************

// => Start async scan for WiFi-networks
void startNetworkScan() {
  Serial.println("[WiFi] Start async WiFi networks scan.");
  WiFi.scanNetworks(true);
}

// => Check for completion of async network scan and create <ul> of found networks
void watchNetworkScan() {
  // Get scan state/result
  int n = WiFi.scanComplete();

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
    String nws = "<ul class=\"blocks\">";
    for (int i = 0; i < n; ++i) {
      nws += networkItemHTML(i, WiFi.SSID(i), WiFi.BSSIDstr(i), WiFi.RSSI(i));
    }
    nws += "</ul>";
    networks_html = nws;
    WiFi.scanDelete();
  }
}


// => Initialise WiFi connection from saved parameters. Return false when connecting fails.
// ****************************************************************************************
bool initWiFi() {
  // Load WiFi credentials from Prefs
  loadCredentials();
  Serial.print("[WiFi] from PREFS: (SSID) ");
  Serial.print(wifi_ssid);
  Serial.print(" {");
  Serial.print(wifi_bssid);
  Serial.print("} | (PW) ");
  Serial.println(wifi_pw);

  // Check for SSID
  if (wifi_ssid == "") {
    Serial.println("[WiFi] Can not connect WiFi. SSID is missing.");
    return false;
  }

  // Check for MAC address
  if (wifi_bssid == "") {
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

  // Try repeatedly to connect to WiFi for 6 seconds
  // Reset when button pressed for two seconds
  int n_try = 1;
  Timer connect_timer(6000);  // 6 s

  // Connect WiFi, first try
  Serial.print("[WiFi] Connecting to ");
  Serial.print(wifi_ssid);
  Serial.print(" {");
  Serial.print(wifi_bssid);
  Serial.println("}");
  WiFi.begin(wifi_ssid.c_str(), wifi_pw.c_str(), 0, wifi_bssid_uint8);

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
      WiFi.begin(wifi_ssid.c_str(), wifi_pw.c_str(), 0, wifi_bssid_uint8);
      n_try++;
    }
    // Reboot 99 tries
    if (n_try >= 99) {
      Serial.println("[WiFi] Failed to connect 99 times. Rebooting device.");
      delay(1000);
      ESP.restart();
    }
  }

  // Connection established
  Serial.print("[Try ");
  Serial.printf("%2d", n_try);
  Serial.print("] Connection established. IP: ");
  Serial.println(WiFi.localIP());
  initMDNS();
  return true;
}


// => Load server config
// *********************
void initServerConfig() {
    loadServerConfig();
    Serial.print("[Server] config from PREFS: (User) ");
    Serial.print(sta_user_str);
    Serial.print(" | (PW) ");
    Serial.print(sta_pw_str);
    Serial.print(" | (Port) ");
    Serial.println(sta_port);
}


// ****************************************************
// Start AP  server and ask for local WiFi credentials.
// ****************************************************
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
  const char* ap_ssid = (ap_ssid_name + "-" + esp_id).c_str();
  if (!WiFi.softAP(ap_ssid, NULL)) {
    Serial.println("[WiFi] Error: Could not start AP!");
    return false;
  }
  Serial.print("[WiFi] Started access point at: (IP) ");
  Serial.print(WiFi.softAPIP());
  Serial.print(" | (URL) ");
  Serial.print(get_ip_url());
  Serial.print(":");
  Serial.println(sta_port);

  // Start DNS Server for captive portal
  // Redirects all URLs to AP server
  dns_server.setTTL(300);
  dns_server.setErrorReplyCode(DNSReplyCode::NoError);
  Serial.print("[DNS] Starting DNS server for captive portal...");
  if (dns_server.start(53, "*", WiFi.softAPIP())) {
    Serial.println("success.");
  } else {
    Serial.println("failed.");
  }

  // Load station server settings
  initServerConfig();

  // -------------------
  // Configure AP server
  // -------------------

  // Root URL
  // --------
  server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    alert = "";
    request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
  });

  // Set WiFi credentials
  // --------------------
  server->on("/network", HTTP_GET, [](AsyncWebServerRequest* request) {
    bool success = false;
    if (request->hasParam("pw") && request->hasParam("ssid") && request->hasParam("bssid")) {
      // Unpack parameters, WiFi credentials
      wifi_ssid = request->getParam("ssid")->value();
      wifi_bssid = request->getParam("bssid")->value();
      wifi_pw = request->getParam("pw")->value();
      Serial.println("Received WiFI credentials | " + wifi_ssid + " (" + wifi_bssid + ") PW: " + wifi_pw);
      bool success = saveCredentials();

      // Send message and reboot ESP
      String msg = "Der Rotor Controller startet sich jetzt neu und versucht sich mit dem Netzwerk ";
      msg += wifi_ssid;
      msg += " {";
      msg += wifi_bssid;
      msg += "} zu verbinden.";
      request->send(200, "text/plain", msg);
      delay(2000);
      ESP.restart();
    }
    // Could not receive/save credentials
    if (!success) {
      alert += "Fehler: Die Netzwerkauswahl konnte nicht übernommen werden!<br>";
      request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
    }
  });

  // Set server config
  // -----------------
  server->on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
    bool success = false;
    if (request->hasParam("user") && request->hasParam("pw") && request->hasParam("port")) {
      // Unpack parameters, server config
      sta_user_str = request->getParam("user")->value();
      sta_pw_str = request->getParam("pw")->value();
      sta_port = request->getParam("port")->value().toInt();
      Serial.println("Received server config | User: " + sta_user_str + " PW: " + sta_pw_str + " | Port: " + String(sta_port));
      success = saveServerConfig();
    } 
    // Could not receive/save server config
    if (success) {
      alert += "Neue Serverkonfiguration wurde übernommen.<br>";
    } else {
      alert += "Fehler: Die Serverkonfiguration konnte nicht übernommen werden!<br>";
    }
    request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
  });

  // Reset server config
  // --------------------
  server->on("/reset", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Reset server config
    bool success = resetServerConfig();
    if (success) {
      Serial.println("Received request to reset server config | User: " + sta_user_str + " PW: " + sta_pw_str + " | Port: " + String(sta_port));
      alert += "Die Serverkonfiguration wurde zurückgesetzt.<br>";
    } else {
      Serial.println("Received request to reset server config | Error: Could not reset server configuration.");
      alert += "Die Serverkonfiguration konnte nicht zurückgesetzt.<br>";
    }
    request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
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
  server->serveStatic("/ap-styles.css", SPIFFS, "/ap-styles.css");
  server->serveStatic("/inter-regular.woff2", SPIFFS, "/inter-regular.woff2");
  server->serveStatic("/inter-700.woff2", SPIFFS, "/inter-700.woff2");
  server->on("/favicon.ico",[](AsyncWebServerRequest *request){request->send(404);});

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
  Serial.println("[Server] started in AP mode.\n");

  // Start first network scan
  scan_now = true;

  return true;
}
