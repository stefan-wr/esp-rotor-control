#include <SimpleSPIFFS.h>
#include <WiFiFunctions.h>
#include <WiFi.h>
#include <globals.h>

// STATION mode WiFi-credentials
String wifi_ssid;
String wifi_bssid;
String wifi_pw;
uint8_t wifi_bssid_uint8[6];

// Paths to save credentials to
const char *ssid_path = "/ssid.txt";
const char *bssid_path = "/bssid.txt";
const char *pw_path = "/pw.txt";

// => Load WiFi credentials from SPIFFS
void loadCredentials() {
    Serial.println((uint32_t)&n_switches, HEX);
    wifi_ssid = readFromSPIFFS(ssid_path);
    wifi_bssid = readFromSPIFFS(bssid_path);
    wifi_pw = readFromSPIFFS(pw_path);
}

// => Save credentials to SPIFFS
void saveCredentials() {
    writeToSPIFFS(ssid_path, wifi_ssid);
    writeToSPIFFS(bssid_path, wifi_bssid);
    writeToSPIFFS(pw_path, wifi_pw);
}

// => Overwrite the saved credentials with empty strings
void resetCredentials() {
    removeFromSPIFFS(ssid_path);
    removeFromSPIFFS(bssid_path);
    removeFromSPIFFS(pw_path);
}

// => Convert wifi_bssid BSSID-String to uint8_t
bool bssidToUint8() {
  if (6 == sscanf(wifi_bssid.c_str(), "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", &wifi_bssid_uint8[0], &wifi_bssid_uint8[1], &wifi_bssid_uint8[2], &wifi_bssid_uint8[3], &wifi_bssid_uint8[4], &wifi_bssid_uint8[5])) {
    return true;
  } else {
    return false;
  }
}

// => Blink WiFi LED n-times, blocking
void blinkWifiLed(const int n) {
  for (int i = 0; i < n * 2; i++) {
    digitalWrite(wifi_status_led, !digitalRead(wifi_status_led));
    delay(250);
  }
}

// => Create HTML <li> item for a scanned network
// **********************************************
String networkItemHTML(const String &ssid, const String &bssid, const int &rssi) {
    String s = "";
    s += "<li><form class=\"network-form\">";
    s += "<span class=\"l-align wrd-break\">" + ssid + "</span>";
    s += "<span class=\"r-align small\">RSSI: " + String(rssi) + "</span>";
    s += "<input type=\"password\" placeholder=\"Passwort eingeben\" name=\"pw\" required>";
    s += "<button type=\"submit\" class=\"small\">Verbinden</button>";
    s += "<input type=\"hidden\" name=\"ssid\" value=\"" + ssid + "\">";
    s += "<input type=\"hidden\" name=\"bssid\" value=\"" + bssid + "\">";
    s += "</form></li>";
    return s;
}


// => Scan for WiFi-networks and create <ul> of found networks
// ***********************************************************
void scanNetworks(String &networks_html) {
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();

  // Show on serial monitor
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
    String nws = "<p>Netzwerk waehlen:</p>";
    nws += "<ul class=\"blocks\">";
    for (int i = 0; i < n; ++i) {
      nws += networkItemHTML(WiFi.SSID(i), WiFi.BSSIDstr(i), WiFi.RSSI(i));
    }
    nws += "</ul>";
    networks_html = nws;
  } else {
    networks_html = "<p>Keine Netzwerke gefunden.<br>Warte 15 Sekunden und aktualisiere die Seite.</p>";
  }
}


// => Initialise WiFi connection from saved parameters. Return false when connecting fails.
// ****************************************************************************************
bool initWiFi() {
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

  // Try repeatedly to connect to WiFi for 10 seconds
  // Reset when button pressed for two seconds
  int n_try = 1;
  const long interval = 8000;
  unsigned long start = millis();

  // Start WiFi, first try
  Serial.println("Connecting WiFi to " + wifi_ssid + " {" + wifi_bssid + "}");
  WiFi.begin(wifi_ssid.c_str(), wifi_pw.c_str(), 0, wifi_bssid_uint8);

  // Try continuously
  while (WiFi.status() != WL_CONNECTED) {
    // Check for interrupt
    if (reset_now) {
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

    // Retry after interval
    if (millis() - start >= interval) {
      Serial.println("[Try " + (String)n_try + "] Failed to connect. WiFi-Status: " + (String)WiFi.status());
      WiFi.disconnect();
      blinkWifiLed(1);
      WiFi.begin(wifi_ssid.c_str(), wifi_pw.c_str(), 0, wifi_bssid_uint8);
      start = millis();
      n_try++;
    }
  }

  // Connection established
  Serial.print("[Try " + (String)n_try + "] Connection established. IP: ");
  Serial.println(WiFi.localIP());
  return true;
}


// Start AP mode server and ask for local WiFi credentials.
// ********************************************************
extern String processor(const String &var);

void getCredentials(AsyncWebServer &server) {
  in_station_mode = false;  // not connected to a WiFi network

  // Start WiFi in AP mode
  WiFi.mode(WIFI_AP);

  // Set custom IP address
  if (use_custom_ip) {
    IPAddress local_ip(ip[0], ip[1], ip[2], ip[3]);
    IPAddress gateway(ip[0], ip[1], ip[2], ip[3]);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_ip, gateway, subnet);
  }
  
  WiFi.softAP(ap_ssid, NULL);
  Serial.print("Starting AP-mode. AP-IP: ");
  Serial.println(WiFi.softAPIP());

  // Configure AP-mode server root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("pw") && request->hasParam("ssid") && request->hasParam("bssid")) {
      // Unpack parameters, WiFi credentials
      wifi_ssid = request->getParam("ssid")->value();
      wifi_bssid = request->getParam("bssid")->value();
      wifi_pw = request->getParam("pw")->value();
      Serial.println(wifi_ssid + " (" + wifi_bssid + ") PW: " + wifi_pw);

      // Save submitted credentials
      saveCredentials();

      // Send message and reboot ESP
      String msg = "Der ESP32 starte sich jetzt neu und versucht sich mit dem Netzwerk ";
      msg += wifi_ssid + " {" + wifi_bssid + "} zu verbinden.";
      request->send(200, "text/plain", msg);
      delay(2000);
      ESP.restart();
    } else {
      request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
    }
  });

  // Trigger a rescan for networks
  server.on("/rescan.html", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/ap-index-esp.html", String(), false, processor);
    scan_now = true;
  });

  // Static files (CSS, fonts)
  server.serveStatic("/styles.css", SPIFFS, "/styles.css");
  server.serveStatic("/inter-regular.woff2", SPIFFS, "/inter-regular.woff2");
  server.serveStatic("/inter-700.woff2", SPIFFS, "/inter-700.woff2");

  // Start server
  server.begin();
}
