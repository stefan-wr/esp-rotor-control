#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <math.h>

#include <globals.h>
#include <SimpleSPIFFS.h>
#include <WiFiFunctions.h>
#include <RotorController.h>
#include <Settings.h>

// Create rotor-controller instance
Rotor::RotorController rotor_ctrl;

// Create favorites instance
Settings::Favorites favorites;

// State variables
bool in_station_mode = true;      // ESP is connected with WiFi -> STA mode
bool scan_now = false;            // Scan for networks now
bool reset_now = false;           // Set by button, tested in main loop
int clients_connected = 0;        // Number of connected socket clients
bool safety_stop_now = false;     // Set by button, tested in main loop
bool authenticate = true;         // Authenticate HTTP connections
String lock_msg = "LOCK|{}";      // Buffer storing lock message

// Create AsyncWebServer object
AsyncWebServer *server;
AsyncWebSocket socket("/ws");

// STATION mode default HTTP login-credentials
const char* http_username = sta_default_user.c_str();
const char* http_password = sta_default_pw.c_str();

// Create DNS Server
DNSServer dns_server;

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

// Setup an interrupt button (Rotor Safety-Stop)
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


// **********************************************************************************
// WebSocket Stuff  *****************************************************************
// **********************************************************************************

// Socket receive function
// ***********************
void socketReceive(char* msg, const size_t &len) {
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

  String identifier(msg);

  // ----- ROTOR -----
  // -----------------
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

    // \/\/ Unpack message \/\/
    // Rotation
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

    // Speed
    val = doc["speed"];
    if (!val.isNull()) {
      rotor_ctrl.setSpeed(val.as<const int>());
    }

    // Auto-rotation request
    val = doc["target"];
    if (!val.isNull()) {
      JsonVariant val2 = doc["useOverlap"];
      if (!val2.isNull()) {
        rotor_ctrl.rotateTo(val.as<const float>(), val2.as<const bool>());
      } else {
        rotor_ctrl.rotateTo(val.as<const float>(), true);
      }
    }
  }

  // ----- CALIBRATION -----
  // -----------------------
  if (identifier == "CALIBRATION") {
    // Deserialize JSON
    StaticJsonDocument<100> doc;
    DeserializationError err = deserializeJson(doc, msg + sep_idx + 1);

    // Test wether deserialization succeeded
    if (err) {
      Serial.print("JSON parse failed: ");
      Serial.println(err.f_str());
      return;
    }

    // \/\/ Unpack message \/\/
    // Calibration
    JsonVariant cal_u1 = doc["u1"];
    JsonVariant cal_u2 = doc["u2"];
    JsonVariant cal_a1 = doc["a1"];
    JsonVariant cal_a2 = doc["a2"];
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

  // ----- FAVORITES -----
  // ---------------------
  if(identifier == "FAVORITES") {
    // For favorites we just save the JSON message with
    // the identifier rejoined by inserting back '|' into msg
    msg[sep_idx] = '|';
    favorites.set(msg);
  }

  // ----- LOCK -----
  // ----------------
  if (identifier == "LOCK") {
    msg[sep_idx] = '|';
    lock_msg = (String) msg;
    socket.textAll(msg);
  }

  // ----- SETTINGS -----
  // --------------------
  if (identifier == "SETTINGS") {
    Serial.print("Received SETTINGS message: ");
    Serial.println(msg);
  }
}

// Initialise new client connection
// ********************************
void socketInitClient(){
  rotor_ctrl.messenger.sendSpeed();
  rotor_ctrl.messenger.sendCalibration();
  Settings::sendSettings();
  favorites.send();
  socket.textAll(lock_msg);
}

// Websocket event handler
// ***********************
void onSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      ++clients_connected;
      socketInitClient();
      Serial.println("[Websocket] Client " + (String)(client->id()) + " connected.");
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

          // Print message
          if (verbose) {
            Serial.print("[Websocket] Data, client "); Serial.print(client->id());
             Serial.print(": "); Serial.println((char*) data);
          }

          // Receive
          socketReceive((char*) data, len);
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
  server->addHandler(&socket);
}



// ----------------------------------------------------------------------------------
// SETUP ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
void setup() {
  // Open serial port
  Serial.begin(serial_speed);

  // Show CPU Frequency
  if (verbose) {
    Serial.print("CPU Frequency: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println(" MHz");
  }

  // Initialise SPIFFS
  if (!mountSPIFFS()) {
    Serial.println("Mounting SPIFFS failed.");
    delay(5000);
    return;
  }

  // Initialise I/O
  pinMode(wifi_status_led, OUTPUT);    // Configure WiFi status LED
  digitalWrite(wifi_status_led, LOW);  // Turn off WiFi status LED
  initWiFiButton();                    // Initialise WiFi reset button
  initRotorButton();                   // Initialise rotor stop button
  rotor_ctrl.init();                   // Initialise rotor
  favorites.init();                    // Initialise favorites

  // Try the loaded settings
  // -----------------------
  if (!initWiFi()) {
    // WiFi connection failed -> launch AccessPoint WiFi mode
    digitalWrite(wifi_status_led, HIGH);
    getCredentials(server, dns_server);

  } else {
    // WiFi connection established
    digitalWrite(wifi_status_led, LOW);

    // ----- \/\/ CONFIGURE STA SERVER \/\/ -----
    // ------------------------------------------

    // Init server config
    initServerConfig();

    http_username = sta_user_str.c_str();
    http_password = sta_pw_str.c_str();
    
    // Disable authentication if password is empty string
    if (sta_pw_str == "") {
      authenticate = false;
    }

    // Define server to set port
    server = new AsyncWebServer(sta_port);

    // Root route, Vue-App index files
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (authenticate && !request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      request->send(SPIFFS, "/index.html");
    });

    server->on("/index.js", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (authenticate && !request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      request->send(SPIFFS, "/index.js");
    });

    // Catch all route, necessary for page reloads in Vue-App
    server->onNotFound([](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html");
    });

    // CSS
    server->serveStatic("/index.css", SPIFFS, "/index.css");

    // Favicons
    server->serveStatic("/inter-regular.woff2", SPIFFS, "/inter-regular.woff2").setCacheControl("public,max-age=31536000");
    server->serveStatic("/inter-700.woff2", SPIFFS, "/inter-700.woff2").setCacheControl("public,max-age=31536000");
    server->serveStatic("/site.webmanifest", SPIFFS, "/site.webmanifest").setCacheControl("public,max-age=31536000");
    server->serveStatic("/favicon.ico", SPIFFS, "/favicon.ico").setCacheControl("public,max-age=31536000");
    server->serveStatic("/favicon-16x16.png", SPIFFS, "/favicon-16x16.png").setCacheControl("public, max-age=31536000");
    server->serveStatic("/favicon-32x32.png", SPIFFS, "/favicon-32x32.png").setCacheControl("public,max-age=31536000");
    server->serveStatic("/apple-touch-icon.png", SPIFFS, "/apple-touch-icon.png").setCacheControl("public,max-age=31536000");
    server->serveStatic("/android-chrome-192x192.png", SPIFFS, "/android-chrome-192x192.png").setCacheControl("public,max-age=31536000");
    server->serveStatic("/android-chrome-512x512.png", SPIFFS, "/android-chrome-512x512.png").setCacheControl("public,max-age=31536000");

    // Disconnect ESP from network
    server->on("/disconnect", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (authenticate && !request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      request->send(200);
      resetCredentials();
      delay(1000);
      ESP.restart();
    });

    // Reboot ESP
    server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (authenticate && !request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      request->send(200);
      delay(1000);
      ESP.restart();
    });

    // Logout
    server->on("/logout", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send(401);
    });

    // Start server
    initWebSocket();
    server->begin();

    Serial.print("Started Server on http://");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.println(sta_port);
  }
}




// ----------------------------------------------------------------------------------
// LOOP -----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

const int n_timers = 5;
unsigned long lasts[n_timers];
// Intervals: { network-scan, WiFi reconnect, remove old websocket, read ADS1115, send COMPASS }
const unsigned long intervals[n_timers] = { 20000, 8000, 1000, 66, 1000 };
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
      rotor_ctrl.stop();
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
      rotor_ctrl.stop();
      blinkWifiLed(1);
      Serial.println("stopped rotor.");
      safety_stop_now = false;
    } else {
      safety_stop_now = false;
      Serial.println("failed.");
    }
  }

  // Watch active auto rotation
  if (rotor_ctrl.is_auto_rotating) {
    rotor_ctrl.watchAutoRotation();
  }

  // Send rotation data
  if (in_station_mode && (clients_connected > 0)) {
    if (millis() - lasts[3] >= intervals[3]) {
     adc_volts = rotor_ctrl.rotor.getADCVolts();

      /* Send rotation message if either:
          1. voltage changed significantly compared to last message
          2. rotor started or stopped rotation
          3. last message was sent more than 1 second ago
      */
      if ((abs(adc_volts - adc_volts_previous) > 0.003) ||
          (rotor_ctrl.is_rotating != is_rotating_prev) ||
          (millis() - lasts[4] >= intervals[4])) {
        rotor_ctrl.messenger.sendNewRotation(true);
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


  // Checking for WiFi disconnects -> try reconnect, stop rotor
  if (in_station_mode) {
    if (millis() - lasts[1] >= intervals[1]) {
      lasts[1] = millis();
      if (WiFi.status() != WL_CONNECTED) {
        rotor_ctrl.stop();
        Serial.println("WiFi disconnected. Try reconnecting...");
        // Try to reconnect WiFi
        WiFi.disconnect();
        blinkWifiLed(1);
        WiFi.reconnect();
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


  // ********** AP MODE  **********
  // ******************************

  // Scanning for networks if not connected to WiFi
  if (!in_station_mode) {
    if (scan_now || (millis() - lasts[0] >= intervals[0])) {
      if (verbose) Serial.println("Scanning for networks after: " + (String)(millis() - lasts[0]) + " ms");
      scanNetworks();
      scan_now = false;
      lasts[0] = millis();
    }
  }

  // DNS Server
  if (!in_station_mode) {
    dns_server.processNextRequest();
  }
}

