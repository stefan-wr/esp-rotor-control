#include <WiFiFunctions.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <globals.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <DNSServer.h>

// PREFS instance
Preferences wifi_prefs;
Preferences server_prefs;

// Stores the HTML-code for list of available networks in AP mode
String networks_html = "";

// Stores alert message
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
void saveServerConfig() {
  server_prefs.begin("serverPrefs", false);
  server_prefs.putInt("port", sta_port);
  server_prefs.putString("user", sta_user_str);
  server_prefs.putString("password", sta_pw_str);
  server_prefs.end();
}

// => Load server config from PREFS
void loadServerConfig() {
  server_prefs.begin("serverPrefs", true);
  sta_port = server_prefs.getInt("port", sta_default_port);
  sta_user_str = server_prefs.getString("user", sta_default_user);
  sta_pw_str = server_prefs.getString("password", sta_default_pw);
  server_prefs.end();
}

// Reset saved server config to default
void resetServerConfig() {
  sta_port = sta_default_port;
  sta_user_str = sta_default_user;
  sta_pw_str = sta_default_pw;
  saveServerConfig();
}

// ----------------------------------- WiFi Credentials

// => Save credentials to PREFS
void saveCredentials() {
  wifi_prefs.begin("wifiPrefs", false);
  wifi_prefs.putString("ssid", wifi_ssid);
  wifi_prefs.putString("bssid", wifi_bssid);
  wifi_prefs.putString("pw", wifi_pw);
  wifi_prefs.end();
}

// => Load WiFi credentials from PREFS
void loadCredentials() {
  wifi_prefs.begin("wifiPrefs", true);
  wifi_ssid = wifi_prefs.getString("ssid", "");
  wifi_bssid = wifi_prefs.getString("bssid", "");
  wifi_pw = wifi_prefs.getString("pw", "");
  wifi_prefs.end();
}

// => Overwrite the saved credentials with empty strings
void resetCredentials() {
  wifi_prefs.begin("wifiPrefs", false);
  wifi_prefs.clear();
  wifi_prefs.end();
}

// -----------------------------------

// => React to button interrupt for resetting WiFi credentials
void resetCredentialsInterrupt() {
  Serial.print("Button pressed. Waiting 2s to confirm...");
  delay(2000);
  if (!digitalRead(button_pin)) {
    blinkWifiLed(4);
    Serial.println("Button pressed. Resetting WiFi credentials and restart.");
    resetCredentials();
    ESP.restart();
  } else {
    reset_now = false;
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

// => Blink WiFi LED n-times, blocking
void blinkWifiLed(const int n) {
  for (int i = 0; i < n * 2; i++) {
    digitalWrite(wifi_status_led, !digitalRead(wifi_status_led));
    delay(250);
  }
}

// => Init mDNS
void initMDNS() {
  if(!MDNS.begin(local_url)) {
      Serial.println("Error starting mDNS");
      return;
  }
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
void scanNetworks() {
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();

  // Show found networks on serial monitor
  if (verbose) {
    if (n != 0) {
      Serial.println("-----------------------------");
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
  if (n != 0) {
    String nws = "<ul class=\"blocks\">";
    for (int i = 0; i < n; ++i) {
      nws += networkItemHTML(i, WiFi.SSID(i), WiFi.BSSIDstr(i), WiFi.RSSI(i));
    }
    nws += "</ul>";
    networks_html = nws;
  } else {
    networks_html = "<p>Keine verfügbaren Netzwerke gefunden.<br>Bitte warte etwas und lade dann diese Seite erneut.</p>";
  }
}


// => Initialise WiFi connection from saved parameters. Return false when connecting fails.
// ****************************************************************************************
bool initWiFi() {
  // Load WiFi settings from Prefs
  loadCredentials();
  Serial.print("WiFi from PREFS: ");
  Serial.print(wifi_ssid);
  Serial.print(" {");
  Serial.print(wifi_bssid);
  Serial.print("} | PW: ");
  Serial.println(wifi_pw);

  // Check for SSID
  if (wifi_ssid == "") {
    Serial.println("Can not start in STATION-mode. SSID is missing.");
    return false;
  }

  // Check for MAC address
  if (wifi_bssid == "") {
    Serial.println("Can not start in STATION-mode. BSSID is missing.");
    return false;
  } else if (!bssidToUint8()) {
    Serial.println("Can not start in STATION-mode. BSSID conversion failed.");
    return false;
  }

  // Set WiFi mode
  WiFi.mode(WIFI_STA);

  // Try repeatedly to connect to WiFi for 6 seconds
  // Reset when button pressed for two seconds
  int n_try = 1;
  const long interval = 6000;
  unsigned long start = millis();

  // Start WiFi, first try
  Serial.print("Connecting WiFi to ");
  Serial.print(wifi_ssid);
  Serial.print(" {");
  Serial.print(wifi_bssid);
  Serial.println("}");
  WiFi.begin(wifi_ssid.c_str(), wifi_pw.c_str(), 0, wifi_bssid_uint8);

  // Try continuously
  while (WiFi.status() != WL_CONNECTED) {
    // Check for interrupt
    if (reset_now) {
      resetCredentialsInterrupt();
    }
    // Retry after interval
    if (millis() - start >= interval) {
      Serial.print("[Try ");
      Serial.print(n_try);
      Serial.print("] Failed to connect. WiFi-Status: ");
      Serial.println(WiFi.status());
      WiFi.disconnect();
      blinkWifiLed(1);
      WiFi.begin(wifi_ssid.c_str(), wifi_pw.c_str(), 0, wifi_bssid_uint8);
      start = millis();
      n_try++;
    }
  }

  // Connection established
  Serial.print("[Try ");
  Serial.print(n_try);
  Serial.print("] Connection established. IP: ");
  Serial.println(WiFi.localIP());
  initMDNS();
  return true;
}


// => Load server config
// *********************
void initServerConfig() {
    loadServerConfig();
    Serial.print("Server config from PREFS: User: ");
    Serial.print(sta_user_str);
    Serial.print(" | PW: ");
    Serial.print(sta_pw_str);
    Serial.print(" | Port: ");
    Serial.println(sta_port);
}


// Start AP mode server and ask for local WiFi credentials.
// ********************************************************
void getCredentials(AsyncWebServer *server, DNSServer &dns_server) {
  in_station_mode = false;  // not connected to a WiFi network

  // Define server with standard HTTP port 80
  server = new AsyncWebServer(80);
  initServerConfig();

  // Start WiFi in AP mode
  WiFi.mode(WIFI_AP);

  // Set custom IP address
  if (use_custom_ip) {
    IPAddress local_ip(ip[0], ip[1], ip[2], ip[3]);
    IPAddress gateway(ip[0], ip[1], ip[2], ip[3]);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_ip, gateway, subnet);
  }

  // Enable mDNS
  initMDNS();
  
  // Start WiFI AP
  WiFi.softAP(ap_ssid, NULL);
  Serial.print("Starting AP-mode. AP-IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("AP-URL: ");
  Serial.print(get_ip_url());
  Serial.print(":");
  Serial.println(sta_port);

  // Start DNS Server for captive portal
  // Redirects all URLs to AP server
  dns_server.setTTL(300);
  dns_server.setErrorReplyCode(DNSReplyCode::NoError);
  Serial.print("Starting DNS server...");
  if (dns_server.start(53, "*", WiFi.softAPIP())) {
    Serial.println("success.");
  } else {
    Serial.println("failed.");
  }

  // Root URL
  server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    alert = "";
    request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
  });

  // Set Wifi Credentials
  server->on("/network", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("pw") && request->hasParam("ssid") && request->hasParam("bssid")) {
      // Unpack parameters, WiFi credentials
      wifi_ssid = request->getParam("ssid")->value();
      wifi_bssid = request->getParam("bssid")->value();
      wifi_pw = request->getParam("pw")->value();
      Serial.println("Received WiFI Credentials | " + wifi_ssid + " (" + wifi_bssid + ") PW: " + wifi_pw);

      // Save credentials
      saveCredentials();

      // Send message and reboot ESP
      String msg = "Der Rotor Controller startet sich jetzt neu und versucht sich mit dem Netzwerk ";
      msg += wifi_ssid;
      msg += " {";
      msg += wifi_bssid;
      msg += "} zu verbinden.";
      request->send(200, "text/plain", msg);

      // Restart ESP
      delay(2000);
      ESP.restart();
    }
    alert += "Fehler: Die Netzwerkauswahl konnte nicht übernommen werden!<br>";
    request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
  });

  // Set server config
  server->on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("user") && request->hasParam("pw") && request->hasParam("port")) {
      // Unpack parameters
      sta_user_str = request->getParam("user")->value();
      sta_pw_str = request->getParam("pw")->value();
      sta_port = request->getParam("port")->value().toInt();
      Serial.println("Received Server Config | User: " + sta_user_str + " PW: " + sta_pw_str + " | Port: " + String(sta_port));
      saveServerConfig();
      alert += "Neue Serverkonfiguration wurde übernommen.<br>";
    } else {
      alert += "Fehler: Die Serverkonfiguration konnte nicht übernommen werden!<br>";
    }
    request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
  });

  // Reset server config
  server->on("/reset", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Reset server config
    resetServerConfig();
    Serial.println("Reset Server Config | User: " + sta_user_str + " PW: " + sta_pw_str + " | Port: " + String(sta_port));
    alert += "Die Serverkonfiguration wurde zurückgesetzt.<br>";
    request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
  });

  // Trigger a rescan for networks
  server->on("/rescan", HTTP_ANY, [](AsyncWebServerRequest* request) {
    request->redirect("/");
    scan_now = true;
  });

  // Catch all route
  server->onNotFound([](AsyncWebServerRequest *request){
    request->redirect(ip_url);
  });

  // Static files (CSS, fonts)
  server->serveStatic("/ap-styles.css", SPIFFS, "/ap-styles.css");
  server->serveStatic("/inter-regular.woff2", SPIFFS, "/inter-regular.woff2");
  server->serveStatic("/inter-700.woff2", SPIFFS, "/inter-700.woff2");
  server->on("/favicon.ico",[](AsyncWebServerRequest *request){request->send(404);});

  // Captive Portal responses
  // ------------------------
  // Required
	server->on("/connecttest.txt",[](AsyncWebServerRequest *request){request->redirect("http://logout.net");}); // windows 11 captive portal workaround
	server->on("/wpad.dat",[](AsyncWebServerRequest *request){request->send(404);});                            // Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32
  //-----
	server->on("/generate_204",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});         // android captive portal redirect
	server->on("/redirect",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});             // microsoft redirect
	server->on("/hotspot-detect.html",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});  // apple call home
	server->on("/canonical.html",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});       // firefox captive portal call home
	server->on("/success.txt",[](AsyncWebServerRequest *request){request->send(200);});                       // firefox captive portal call home
	server->on("/ncsi.txt",[](AsyncWebServerRequest *request){request->redirect(get_ip_url());});             // windows call home

  // Start server
  server->begin();

  // Start first network scan
  scan_now = true;
}
