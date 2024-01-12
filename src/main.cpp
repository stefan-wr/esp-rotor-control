#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <math.h>

#include <globals.h>
#include <SimpleSPIFFS.h>
#include <WiFiFunctions.h>
#include <RotorController.h>
#include <Settings.h>
#include <Timer.h>
#include <Screen.h>


// ESP ID
String esp_id = "";

// Create instances
Rotor::RotorController rotor_ctrl;
Screen::Screen screen;
Settings::Favorites favorites;

// State variables
bool in_station_mode = true;      // ESP is connected with WiFi -> STA mode
bool scan_now = false;            // Scan for networks now
bool multi_btn_pressed = false;   // Set true by interrupt button
bool multi_btn_hold = false;      // True if button is being held
int clients_connected = 0;        // Number of connected socket clients
bool authenticate = true;         // Authenticate HTTP connections
bool use_screen = true;

// Buffer storing lock message, default message resets lock on ESP restart
String lock_msg = "LOCK|{\"isLocked\":false,\"by\":\"\"}";

// Create AsyncWebServer object
AsyncWebServer *server;
AsyncWebSocket socket("/ws");

// STATION mode default HTTP login-credentials
const char* http_username = sta_default_user.c_str();
const char* http_password = sta_default_pw.c_str();

// Create DNS Server
DNSServer dns_server;

// Setup an interrupt button
// *************************
Timer multi_btn_debounce_timer(250);

// => Interrupt for button press
void IRAM_ATTR multiButtonAction() {
  // Debounce 250 ms
  if (multi_btn_debounce_timer.passed()) {
    multi_btn_pressed = true;
  }
}

// => Initialise interrupt button
void initMultiButton() {
  pinMode(multi_button_pin, INPUT_PULLUP);
  attachInterrupt(multi_button_pin, multiButtonAction, FALLING);
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
      if (val < 0) {
        rotor_ctrl.setSpeed(0);
      } else if (val > 100) {
        rotor_ctrl.setSpeed(100);
      } else {
        rotor_ctrl.setSpeed(val.as<const int>());
      }
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
  socket.textAll(lock_msg);
  Settings::sendSettings();
  rotor_ctrl.messenger.sendSpeed();
  rotor_ctrl.messenger.sendCalibration();
  rotor_ctrl.messenger.sendLastRotation(false);
  favorites.send();
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

  // Initialise screen
  if (use_screen) {
    use_screen = screen.init();
  }

  // ESP ID derived from MAC address
  esp_id = WiFi.macAddress();
  esp_id.replace(":", "");
  Serial.print("\nHardware-ID: ");
  Serial.println(esp_id);

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

  // Initialise I/O and classes
  pinMode(wifi_status_led, OUTPUT);    // Configure WiFi status LED
  digitalWrite(wifi_status_led, LOW);  // Turn off WiFi status LED
  initMultiButton();                   // Initialise multi purpose button
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
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html");
      response->addHeader("cache-control", "private, max-age=86400");
      request->send(response);
    });

    server->on("/index.js", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (authenticate && !request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.js");
      response->addHeader("cache-control", "private, max-age=86400");
      request->send(response);
    });

    // Catch all route, necessary for page reloads in Vue-App
    server->onNotFound([](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html");
    });

    // CSS
    server->serveStatic("/index.css", SPIFFS, "/index.css").setCacheControl("public, max-age=86400");

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

// Timers, that control how often each task in the mainloop is executed
struct {
  Timer *networkScan = new Timer(20000);
  Timer *checkWiFi = new Timer(8000);
  Timer *reconnectTimeout = new Timer(90000); //600000
  Timer *reboot = new Timer(86400000 * 3);
  Timer *multiBtnHold = new Timer(2000);
  Timer *cleanSockets = new Timer(1000);
  Timer *rotorUpdate = new Timer(40);
  Timer *rotorMessage = new Timer(1000);
  Timer *updateScreen = new Timer(40);
  Timer *loopTimer = new Timer(1000);
} timers;

float adc_volts_prev = -1;  // Previous ADC volts
bool is_rotating_prev = false;
int clients_connected_prev;
bool reconnecting = false;

unsigned long loopCounter = 0;
unsigned long loop_mus = micros();


void loop() {
  loopCounter += 1;

  // Check for multi button being pressed down
  if (multi_btn_pressed && !multi_btn_hold) {
    // Stop rotor
    Serial.println("[BTN] pressed. Stopping rotor.");
    if (rotor_ctrl.is_rotating) {
      rotor_ctrl.stop();
      blinkWifiLed(1);
    }
    // Enable further checks for wether multi button is being held down
    multi_btn_pressed = false;
    multi_btn_hold = true;
    timers.multiBtnHold->start();
  }

  // Check wether button is being held down
  if (multi_btn_hold) {
    if (digitalRead(multi_button_pin)) {
      // Button was released
      multi_btn_hold = false;
    } else if (timers.multiBtnHold->passed()) {
      // Button held for 2s -> reset WiFi 
      rotor_ctrl.stop();
      blinkWifiLed(4);
      Serial.println("[BTN] held for 2s. Resetting WiFi credentials and restart.");
      resetCredentials();
      ESP.restart();
    }
  }



  // Update rotor values and send rotation message to clients.
  // Update angular speed every 10th update.
  if (in_station_mode && timers.rotorUpdate->passed()) {
    rotor_ctrl.update(!(timers.rotorUpdate->n_passed % 5));

    // Send rotation message every second update only if clients are connected
    if (clients_connected > 0 && timers.rotorUpdate->n_passed % 2 == 0) {
      /* Send rotation message if either:
          1. voltage changed significantly compared to last message
          2. rotor started or stopped rotation
          3. last message was sent more than 1 second ago
      */
      if ((abs(rotor_ctrl.rotor.last_adc_volts - adc_volts_prev) > 0.003) ||
          (rotor_ctrl.is_rotating != is_rotating_prev) ||
          (timers.rotorMessage->passed())) {
        rotor_ctrl.messenger.sendLastRotation(true);
        adc_volts_prev = rotor_ctrl.rotor.last_adc_volts;
        is_rotating_prev = rotor_ctrl.is_rotating;
      }      
    }
  }



  // Watch active auto rotation
  if (rotor_ctrl.is_auto_rotating) {
    rotor_ctrl.watchAutoRotation();
  }



  // Stop rotor if all clients disconnected
  if (clients_connected == 0 && clients_connected_prev) {
    screen.setAlert("Alle Verbindungen wurden getrennt.");
    rotor_ctrl.stop();
    Serial.println("[Websocket] ALL clients disconnected.");
    clients_connected_prev = 0;
  } else {
    clients_connected_prev = clients_connected;
  }



  // Check for lost of WiFi connection -> stop rotor, try to reconnect
  if (in_station_mode && timers.checkWiFi->passed()) {
    if (WiFi.status() != WL_CONNECTED) {
      // Stop rotor
      if (rotor_ctrl.is_rotating) {
        rotor_ctrl.stop();
      }

      // Start reboot timer
      if (!reconnecting) {
        timers.reconnectTimeout->start();
        reconnecting = true;
      }

      // Try to reconnect
      Serial.println("[WiFi] WiFi disconnected. Try reconnecting...");
      WiFi.disconnect();
      blinkWifiLed(1);
      WiFi.reconnect();
    }
  }

  // Stop timeout if connection was reestablished
  if (in_station_mode && reconnecting && WiFi.status() == WL_CONNECTED) {
    reconnecting = false;
    Serial.println("[WiFi] Reconnected.");
  }

  // Reboot ESP after reconnect timeout
  if (in_station_mode && reconnecting && timers.reconnectTimeout->passed()) {
    Serial.println("[WiFi] Reconnecting failed. Restarting device.");
    delay(1000);
    ESP.restart();
  }



  // Clean old websocket connections
  if (in_station_mode && timers.cleanSockets->passed()) {
    socket.cleanupClients();
  }



  // Reboot ESP after a few days
  if (in_station_mode && timers.reboot->passed()) {
    ESP.restart();
  }



  // Update screen
  if (use_screen && timers.updateScreen->passed()) {
    screen.update();
  }



  // ********** AP MODE  **********
  // ******************************

  // Scanning for networks
  if (!in_station_mode) {
    if (scan_now || timers.networkScan->passed()) {
      if (verbose) Serial.println("Scanning for networks after.");
      startNetworkScan();
      scan_now = false;
    }
    watchNetworkScan();
  }

  // DNS Server
  if (!in_station_mode) {
    dns_server.processNextRequest();
  }



  // ****** Loop Cycle Time  ******
  // ******************************

  if (timers.loopTimer->passed() && false) {
    double loop_cycle = (float) ((micros() - loop_mus) / loopCounter) / 1000.0;
    Serial.print("Loop cycle: ");
    Serial.printf("%3.2f", loop_cycle);
    Serial.println(" ms");
    loopCounter = 0;
    loop_mus = micros();
  }
}
