#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <math.h>

#include <globals.h>
#include <SimpleSPIFFS.h>
#include <WiFiFunctions.h>
#include <ToggleSwitch.h>
//#include <ADS1115.h>
#include <RotorController.h>

// Title on Website
const char* TITLE = "Rotor-Controller";

// AP mode SSID
const char* ap_ssid = "ESP32 Rotor";

// STATION mode HTTP login-credentials
const char* http_username = "otto";
const char* http_password = "dl2rz";

// URLS
const char* DISCONNECT_URL = "/disconnect";
const char* REBOOT_URL = "/reboot";
const char* LOGOUT_URL = "/logout";
const char* SWITCH_URL = "/switch";
const char* RESET_SWITCHES_URL = "/reset-switches";

// Create rotor-controller instance
Rotor::RotorController rotor_ctrl;

// Initialise switches {pin, id, name, dscr, default}
ToggleSwitch switches[n_switches] = {};
/* ToggleSwitch switches[n_switches] = {ToggleSwitch(17, "pin-17", "PIN #17", "Standardzustand: LOW", false),
                                     ToggleSwitch(16, "pin-16", "PIN #16", "Standardzustand: HIGH", true)}; */

// State variables
bool in_station_mode = true;      // ESP is connected with WiFi -> STA mode
bool scan_now = false;            // Scan for networks now
bool reset_now = false;           // Set by button, tested in main loop
bool switches_save_mode = false;  // Set switches to default positions now
int clients_connected = 0;        // Number of connected socket clients
bool safety_stop_now = false;     // Set by button, testesd in main loop

// Stores the HTML-code for list of available networks
String networks_html = "";

// Create AsyncWebServer object
AsyncWebServer server(port);
AsyncWebSocket socket("/ws");

// Setup an interrupt button (WiFi Reset)
// *************************************
unsigned long wifi_btn_last_ms = 0;

// => Interrupt for button press
void IRAM_ATTR wifiButtonAction() {
  unsigned long btn_ms = millis();
  // Debounce 250 ms
  if (btn_ms - wifi_btn_last_ms > 250) {
    wifi_btn_last_ms = btn_ms;
    reset_now = true;
  }
}

// => Initialise interrupt button
void initWiFiButton() {
  pinMode(button_pin, INPUT_PULLUP);
  attachInterrupt(button_pin, wifiButtonAction, FALLING);
}

// Setup an interrupt button (rotor safety stop)
// *********************************************
unsigned long rotor_btn_last_ms = 0;

// => Interrupt for button press
void IRAM_ATTR rotorButtonAction() {
  unsigned long btn_ms = millis();
  // Debounce 250 ms
  if (btn_ms - rotor_btn_last_ms > 250) {
    rotor_btn_last_ms = btn_ms;
    safety_stop_now = true;
  }
}

// => Initialise interrupt button
void initRotorButton() {
  pinMode(safety_stop_pin, INPUT_PULLUP);
  attachInterrupt(safety_stop_pin, rotorButtonAction, FALLING);
}


// => HTML processor, replaces placeholders in HTML files
// ******************************************************
String processor(const String &var) {
  if (var == "TITLE") {
    return TITLE;
  }
  if (var == "NETWORKS") {
    return networks_html;
  }
  if (var == "SSID") {
    return wifi_ssid;
  }
  if (var == "RSSI") {
    return (String)WiFi.RSSI();
  }
  if (var == "DISCONNECT_URL") {
    return DISCONNECT_URL;
  }
  if (var == "REBOOT_URL") {
    return REBOOT_URL;
  }
  if (var == "LOGOUT_URL") {
    return LOGOUT_URL;
  }
  if (var == "SWITCHES") {
    return switchesListHTML();
  }
  return String();
}


// WebSocket stuff
// ***************
void socketRecieve(char* msg, const size_t &len) {
  // Separate message from identifier, separated by '|'
  int sep_idx;  // Index of separator
  for(sep_idx =  0; sep_idx < len; ++sep_idx) {
    if (msg[sep_idx] == '|') { break; }
  }

  // Split message by inserting null-terminator
  if (sep_idx != 0 && sep_idx != len) {
    msg[sep_idx] = '\0';
  } else {
    Serial.println("Can't parse message.");
    return;
  }

  /*
  String msg_str = (String)msg;
  int sep_index = msg_str.indexOf("|");
  String identifier = msg_str.substring(0, sep_index);
  String msg_json = msg_str.substring(sep_index + 1, msg_str.length());
  */

  String identifier(msg);

  // ----- ROTOR -----
  if (identifier == "ROTOR") {
    // Deserialize JSON
    StaticJsonDocument<50> doc;
    DeserializationError err = deserializeJson(doc, msg + sep_idx + 1);

    // Test wether deserialization succeeded
    if (err) {
      Serial.print("JSON parse failed: ");
      Serial.println(err.f_str());
      return;
    }

    // Unpack message
    JsonVariant val;
    val = doc["rotation"];
    if (!val.isNull()) {
      switch (val.as<const int>()) {
        case 0: 
          rotor_ctrl.stop(); break;
        case -1:
          rotor_ctrl.startRotation(0); break;
        case 1:
          rotor_ctrl.startRotation(1); break;
      }
    }

    // Rotor speed
    val = doc["speed"];
    if (!val.isNull()) {
      rotor_ctrl.setSpeed(val.as<const int>());
    }
  }

  // ----- SETTINGS -----
  if (identifier == "SETTINGS") {
    // Deserialize JSON
    StaticJsonDocument<200> doc;
    DeserializationError err = deserializeJson(doc, msg + sep_idx + 1);

    // Test wether deserialization succeeded
    if (err) {
      Serial.print("JSON parse failed: ");
      Serial.println(err.f_str());
      return;
    }

    // \/\/ Unpack message \/\/

    // Calibration
    JsonVariant cal_u1 = doc["cal"]["u1"];
    JsonVariant cal_u2 = doc["cal"]["u2"];
    JsonVariant cal_a1 = doc["cal"]["a1"];
    JsonVariant cal_a2 = doc["cal"]["a2"];
    if (!cal_u1.isNull() && !cal_u2.isNull()
        && !cal_a1.isNull() && !cal_a2.isNull()) {
      rotor_ctrl.setCalibration(cal_u1.as<const float>(), cal_u2.as<const float>(),
                                cal_a1.as<const float>(), cal_a2.as<const float>());
    }

    // Angle offset
    JsonVariant offset = doc["offset"];
    if (!offset.isNull()) {
      rotor_ctrl.setAngleOffset(offset.as<float>());
    }
  }
}


// Initialise new client connection
void socketInitClient(){
  rotor_ctrl.messenger.sendSpeed();
}

// Websocket event handler
void onSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      ++clients_connected;
      socketInitClient();
      Serial.println("[Websocket] Client " + (String)(client->id()) + " connected.");
      if (n_switches != 0) socket.textAll("SWITCHES|" + getSwitchesJSON());
      break;
    case WS_EVT_DISCONNECT:
      --clients_connected;
      Serial.println("[Websocket] Client " + (String)(client->id()) + " disconnected.");
      break;
    case WS_EVT_ERROR:
      Serial.println("[WebSocket] Error received.");
      break;
    case WS_EVT_PONG:
      Serial.println("[WebSocket] Got pinged.");
      break;      
    case WS_EVT_DATA:
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
      if(info->final && info->index == 0 && info->len == len){
        //the whole message is in a single frame and we got all of it's data
        if(info->opcode == WS_TEXT){
          data[len] = 0;

          if (verbose) {
            Serial.print("[Websocket] Data, client "); Serial.print(client->id());
             Serial.print(": "); Serial.println((char*) data);
          }

          socketRecieve((char*) data, len);
        } else {
          Serial.println("[Websocket] Binary data recieved unexpectedly.");
        }
      } else {
        Serial.println("[Websocket] Data recieved in multiple frames.");
      }
      break;
  }
}

// => Add event handler to socket
void initWebSocket() {
  socket.onEvent(onSocketEvent);
  server.addHandler(&socket);
}




// ----------------------------------------------------------------------------------
// SETUP ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
void setup() {
  // Open serial port
  Serial.begin(serial_speed);

  // Initialise SPIFFS
  if (!mountSPIFFS()) {
    Serial.println("Mounting SPIFFS failed.");
    return;
  }

  // Initialise I/O
  pinMode(wifi_status_led, OUTPUT);    // Configure WiFi status LED
  digitalWrite(wifi_status_led, LOW);  // Turn off WiFi status LED
  pinMode(rotor_led, OUTPUT);          // Configure rotor status LED
  digitalWrite(rotor_led, LOW);        // Turn off rotor status LED
  initWiFiButton();                    // Initialise wifi reset button
  initRotorButton();                   // Initialise rotor stop button
  initSwitches();                      // Initialise switches with hard coded default positions
  rotor_ctrl.init();                   // Initialise rotor

  // Load WiFi settings from SPIFFS
  loadCredentials();
  Serial.println("WiFi from SPIFFS: " + wifi_ssid + " | {" + wifi_bssid + "} | PW: " + wifi_pw);

  // Try the loaded settings
  // -----------------------
  if (!initWiFi()) {
    // WiFi connection failed -> launch AccessPoint WiFi mode
    digitalWrite(wifi_status_led, HIGH);
    getCredentials(server);

  } else {
    // WiFi connection established
    digitalWrite(wifi_status_led, LOW);

    // Apply loaded positions to switches
    switchesApply();

    // ----- \/\/ CONFIGURE STA SERVER \/\/ -----
    // ------------------------------------------
    // Root route
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      request->send(SPIFFS, "/sta-index-esp.html", String(), false, processor);
    });

    // Static files (CSS, fonts)
    server.serveStatic("/ui.js", SPIFFS, "/ui.js");
    server.serveStatic("/styles.css", SPIFFS, "/styles.css");
    server.serveStatic("/inter-regular.woff2", SPIFFS, "/inter-regular.woff2");
    server.serveStatic("/inter-700.woff2", SPIFFS, "/inter-700.woff2");
    server.serveStatic("/site.webmanifest", SPIFFS, "/site.webmanifest");
    server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
    server.serveStatic("/favicon-16x16.png", SPIFFS, "/favicon-16x16.png");
    server.serveStatic("/favicon-32x32.png", SPIFFS, "/favicon-32x32.png");
    server.serveStatic("/apple-touch-icon.png", SPIFFS, "/apple-touch-icon.png");
    server.serveStatic("/android-chrome-192x192.png", SPIFFS, "/android-chrome-192x192.png");
    server.serveStatic("/android-chrome-512x512.png", SPIFFS, "/android-chrome-512x512.png");

    // Logout
    server.on(LOGOUT_URL, HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send(401);
    });

    // Reboot ESP
    server.on(REBOOT_URL, HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      request->send(200, "text/plain", "Der ESP32 startet sich neu.");
      delay(2000);
      ESP.restart();
    });

    // Disconnect ESP from network
    server.on(DISCONNECT_URL, HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      request->send(200, "text/plain", "Der ESP32 trennt sich vom Netzwerk und startet sich neu. Der Fernzugriff ist damit inaktiv, bis eine neue Netzwerkverbindung aufgebaut wurde.");
      resetCredentials();
      delay(2000);
      ESP.restart();
    });

    // Toggle a switch -> check and send back switch status to confirm
    server.on(SWITCH_URL, HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!request->authenticate(http_username, http_password))
        return request->requestAuthentication();

      if (request->hasParam("id") && request->hasParam("checked")) {
        // Unpack parameters
        String id = request->getParam("id")->value();
        String checked = request->getParam("checked")->value();
        Serial.println("Request: Toggle (" + id + ") to " + checked + ".");

        // Set switch status and send confirmation
        if (checked == "true") {
          setSwitch(id, true);
          request->send(200);
        } else if (checked == "false") {
          setSwitch(id, false);
          request->send(200);
        } else {
          request->send(200, "text/plain", "Couldn't parse this request: " + checked);
        }
        // Send new switch positions to all clients
        socket.textAll("SWITCHES|" + getSwitchesJSON());
      }
    });

    // Reset Switches to default positions
    server.on(RESET_SWITCHES_URL, HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!request->authenticate(http_username, http_password))
        return request->requestAuthentication();

      switchesDefault(true);
      request->send(SPIFFS, "/sta-index-esp.html", String(), false, processor);
      socket.textAll("SWITCHES|" + getSwitchesJSON());
    });

    // Start server
    initWebSocket();
    server.begin();
  }
}




// ----------------------------------------------------------------------------------
// LOOP -----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
const int n_timers = 5;
unsigned long lasts[n_timers];
// Intervals: { network-scan, WiFi reconnect, remove old websocket, read ADS1115, send COMPASS }
const unsigned long intervals[n_timers] = { 15000, 8000, 1000, 66, 1000 };
unsigned long current_loop_ms;
unsigned long previous_loop_ms = 0;

float adc_volts;                // Current ADC volts
float adc_volts_previous = -1;  // Previous ADC volts

bool is_rotating_prev = false;
int clients_connected_prev;

void loop() {
  // Handle millis() overflow, resets lasts
  current_loop_ms = millis();
  if (current_loop_ms < previous_loop_ms) {
    for (int i = 0; i < n_timers; i++) {
      lasts[i] = 0;
    }
  }
  previous_loop_ms = current_loop_ms;


  // Checking for WiFi button interrupt
  if (reset_now) {
    Serial.print("Button pressed. Waiting 2s to confirm...");
    delay(2000);
    if (!digitalRead(button_pin)) {
      blinkWifiLed(4);
      Serial.println("resetting WiFi credentials and restart.");
      resetCredentials();
      ESP.restart();
    } else {
      reset_now = false;
      Serial.println("failed.");
    }
  }

  // Checking for rotor button interrupt
  if (safety_stop_now) {
    Serial.print("Stop button pressed. Waiting 400ms to confirm...");
    delay(400);
    if (!digitalRead(safety_stop_pin)) {
      Serial.println("stopped rotor.");
      rotor_ctrl.stop();
      safety_stop_now = false;
    } else {
      safety_stop_now = false;
      Serial.println("failed.");
    }
  }


  // Send rotation data
  // Interval: ms
  if (in_station_mode && (clients_connected > 0)) {
    if (millis() - lasts[3] >= intervals[3]) {
     adc_volts = rotor_ctrl.rotor.getADCVolts();

      if ((abs(adc_volts - adc_volts_previous) > 0.003) ||
          (rotor_ctrl.is_rotating != is_rotating_prev) ||
          (millis() - lasts[4] >= intervals[4])) {
        rotor_ctrl.messenger.sendNewRotationMsg(true);
      }

      adc_volts_previous = adc_volts;
      is_rotating_prev = rotor_ctrl.is_rotating;
      lasts[3] = millis();

      if (millis() - lasts[4] >= intervals[4]) {
        lasts[4] = millis();
      }
    }
  }


  // Stop rotor if all clients disconnected
  if (clients_connected == 0 && clients_connected_prev) {
    if (rotor_ctrl.is_rotating) {
      rotor_ctrl.stop();
    }
    Serial.println("[Websocket] ALL clients disconnected.");
    clients_connected_prev = 0;
  } else {
    clients_connected_prev = clients_connected;
  }
  

  // Scanning for networks if not connected to WiFi
  // Interval: 15s
  if (!in_station_mode) {
    if (scan_now || (millis() - lasts[0] >= intervals[0])) {
      if (verbose) Serial.println("Scanning for networks after: " + (String)(millis() - lasts[0]) + " ms");
      scanNetworks(networks_html);
      scan_now = false;
      lasts[0] = millis();
    }
  }


  // Checking for WiFi disconnects -> try reconnect
  // Set switches in save mode (default pos.) while connection lost
  // Interval: 10s
  if (in_station_mode) {
    if (millis() - lasts[1] >= intervals[1]) {
      lasts[1] = millis();
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected. Try reconnecting...");

        // Set switches in save mode
        if (!switches_save_mode) {
          switchesDefault(false);
          switches_save_mode = true;
        }

        // Try to reconnect WiFi
        WiFi.disconnect();
        blinkWifiLed(1);
        WiFi.reconnect();
      } else if (switches_save_mode) {
        switches_save_mode = false;
        switchesReset();
      }
    }
  }


  // Clean old websocket connections
  // Interval: 1s
  if (in_station_mode) {
    if (millis() - lasts[2] >= intervals[2]) {
      socket.cleanupClients();
      lasts[2] = millis();
    }
  }
}
