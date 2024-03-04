#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <math.h>

#include <globals.h>
#include <SimpleFS.h>
#include <WiFiFunctions.h>
#include <RotorController.h>  // Exposes Global: rotor_ctrl
#include <Settings.h>
#include <Favorites.h>
#include <Timer.h>
#include <Screen.h>           // Exposes Global: screen
#include <Firmware.h>         // Exposes Global: firmware
#include <BlinkingLED.h>
#include <Stats.h>

#define HAS_SCREEN true
#define COUNT_LOOP_CYCLE_TIME false

// Software version
const String version = "0.9.2";

// ESP ID
String esp_id = "";

// Create instances
Favorites favorites;
BlinkingLED wifi_led(wifi_led_pin, LOW, 250);

// State variables
bool in_station_mode = true;        // ESP is connected with WiFi -> STA mode
bool scan_now = false;              // Scan for networks now
bool multi_btn_pressed = false;     // Set true by pressing interrupt button
bool multi_btn_hold = false;        // True if button is being held
int clients_connected = 0;          // Number of connected socket clients
bool authenticate = true;           // Authenticate HTTP connections
bool has_screen = HAS_SCREEN;       // Device has an SSD1306 screen
bool use_screen = HAS_SCREEN;       // Use the SSD1306 screen
uint8_t authentications = 0;        // Number of authentications, used in /authenticate URL

// Buffer storing lock message. This default message resets lock on ESP restart
String lock_msg = "LOCK|{\"isLocked\":false,\"by\":\"\"}";

// Create AsyncWebServer object
AsyncWebServer *server;
AsyncWebSocket websocket("/ws");

// STATION mode default HTTP login-credentials
const char* http_username = sta_default_user.c_str();
const char* http_password = sta_default_pw.c_str();

// Create DNS Server
DNSServer dns_server;


// Setup an interrupt button
// *************************
Timer multi_btn_press_debounce_timer(100);

// => Interrupt for button press
void IRAM_ATTR multiButtonPressAction() {
  // Debounce 250 ms
  if (multi_btn_press_debounce_timer.passed() && !multi_btn_hold) {
    multi_btn_pressed = true;
  }
}

// => Initialise interrupt button
void initMultiButton() {
  pinMode(multi_button_pin, INPUT_PULLUP);
  attachInterrupt(multi_button_pin, multiButtonPressAction, FALLING);
}

// => Show a fatal error message and restart ESP
// *********************************************
void fatalError(String err, bool restart = true) {
  Serial.print("[FATAL ERROR] ");
  Serial.println(err);
  if (has_screen) {
    screen.setAlertImmediatly("ERROR:\n" + err);
  }
  if (restart) {
    delay(2000);
    Serial.println("[ESP] Restart in 10 seconds.");
    delay(10000);
    ESP.restart();
  }
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
    Serial.println("[Websocket] Error: Could not parse message.");
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
      Serial.print("[Websocket] Error: JSON parse failed: ");
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
        rotor_ctrl.setMaxSpeed(0);
      } else if (val > 100) {
        rotor_ctrl.setMaxSpeed(100);
      } else {
        rotor_ctrl.setMaxSpeed(val.as<const int>());
      }
    }

    // Auto-rotation request
    val = doc["target"];
    if (!val.isNull()) {
      JsonVariant use_overlap = doc["useOverlap"];
      JsonVariant use_smooth_speed = doc["useSmoothSpeed"];

      if (!use_overlap.isNull() && !use_smooth_speed.isNull()) {
        rotor_ctrl.rotateTo(val.as<const float>(),
                            use_overlap.as<const bool>(),
                            use_smooth_speed.as<const bool>());
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
      Serial.print("[Websocket] Error: JSON parse failed: ");
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

  // ----- SETTINGS -----
  // --------------------
  if (identifier == "SETTINGS") {
    // Deserialize JSON
    StaticJsonDocument<48> doc;
    DeserializationError err = deserializeJson(doc, msg + sep_idx + 1);

    // Test wether deserialization succeeded
    if (err) {
      Serial.print("[Websocket] Error: JSON parse failed: ");
      Serial.println(err.f_str());
      return;
    }

    // \/\/ Unpack message \/\/
    // Screen
    JsonVariant val;
    val = doc["useScreen"];
    if (!val.isNull()) {
      if (!val.as<const bool>()) {
        use_screen = false;
        screen.disable();
      } else {
        use_screen = true;
        screen.enable();
      }
      // Distribute to clients
      Settings::sendScreen();
    }
  }

  // ----- FAVORITES -----
  // ---------------------
  if(identifier == "FAVORITES") {
    // For favorites, just save the JSON message with
    // the identifier rejoined by inserting back '|' into msg
    msg[sep_idx] = '|';
    favorites.set(msg);
  }

  // ----- LOCK -----
  // ----------------
  if (identifier == "LOCK") {
    // For lock, just distribute message to all clients
    msg[sep_idx] = '|';
    lock_msg = (String) msg;
    websocket.textAll(msg);
  }
}

// Initialise new client connection
// ********************************
void socketInitClient(){
  websocket.textAll(lock_msg);
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
        // The whole message is a single frame and was fully received
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
          Serial.println("[Websocket] Binary data received unexpectedly.");
        }
      } else {
        Serial.println("[Websocket] Data recieved in multiple frames.");
      }
      break;
  }
}

// => Add event handler to socket
void initWebSocket() {
  websocket.onEvent(onSocketEvent);
  server->addHandler(&websocket);
}





// ----------------------------------------------------------------------------------
// SETUP ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
void setup() {
  // Open serial port
  Serial.begin(serial_speed);

  // Software version
  Serial.print("\n\r##########  RotorControl v");
  Serial.print(version);
  Serial.println("  ##########");

  // Boot cycle counter
  Stats::Counter boot_counter = Stats::Counter("boot");
  boot_counter.add(1);
  Serial.print("[Stats] Boot #: ");
  boot_counter.printlnToSerial();

  // ESP ID derived from MAC address
  esp_id = WiFi.macAddress();
  esp_id.replace(":", "");
  Serial.print("[ESP] Hardware-ID: ");
  Serial.println(esp_id);

  // Show CPU Frequency
  if (verbose) {
    Serial.print("[ESP] CPU Frequency: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println(" MHz");
  }

  // Initialisations
  // ---------------

  // Initialise screen
  if (has_screen) {
    use_screen = screen.init();
    if (!use_screen) {
      has_screen = false;
      Serial.println("[Screen] Failed to initialise the screen.");
    }
  }

  // Initialise multi purpose button
  initMultiButton();          

  // Initialise rotor controller instance
  if(!rotor_ctrl.init()) {    
    fatalError("Failed to properly initialise rotor controller instance.", false);
  }

  // Initialise filesystem
  if (!mountFS()) {
    fatalError("Failed to mount filesystem! ESP may require reflashing.");
    return;
  }

  // Initialise favorites
  favorites.init();                 

  // Start WiFi connection
  // ---------------------
  if (!initWiFi()) {
    // WiFi connection failed -> launch ESP in AccessPoint WiFi mode
    wifi_led.invert();
    if (!startAPServer(server, dns_server)) {
      fatalError("Failed to start WiFi in AP mode! Try resetting WiFi with button, or reflash firmware.");
    }
  } else {
    // WiFi connection established
    wifi_led.off();

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

    // Redeclare server to set corrct port
    server = new AsyncWebServer(sta_port);

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
      // CORS preflight
      if (request->method() == HTTP_OPTIONS) {
        request->send(200);
      // Redirect to index.html, necessary for page reloads in Vue-App
      } else {
        if (authenticate && !request->authenticate(http_username, http_password))
          return request->requestAuthentication();
        request->send(LittleFS, "/index.html");
        }
    });

    // Root route, Vue-App index file
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (authenticate && !request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html");
      response->addHeader("cache-control", "private, max-age=86400");
      request->send(response);
    });

    /*
    server->on("/index.js", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (authenticate && !request->authenticate(http_username, http_password))
        return request->requestAuthentication();
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.js");
      response->addHeader("cache-control", "private, max-age=86400");
      request->send(response);
    });
    */

    // CSS
    //server->serveStatic("/index.css", LittleFS, "/index.css").setCacheControl("public, max-age=86400");

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

    // Authenticate
    server->on("/authenticate", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (authenticate) {
        Serial.println(authentications);
        if (!authentications || !request->authenticate(http_username, http_password)) {
          authentications++;
          return request->requestAuthentication();
        }
      }
      authentications = 0;
      request->send(200);
    });

    // Request a firmware udate
    server->on("/request-update", HTTP_POST, Firmware::handleUpdateRequest);

    // Update firmware
    server->on("/update", HTTP_POST, Firmware::handleFirmwareResponse, Firmware::handleFirmwareUpload);

    // Init firmware instace
    firmware.initFirmware();

    // Start server
    initWebSocket();
    server->begin();

    Serial.print("[Server] started on http://");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.println(sta_port);
    Serial.println();
  }
}




// ----------------------------------------------------------------------------------
// LOOP -----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

// Timers, that control how often each task in the mainloop is executed
struct {                                      // Intervals
  Timer *networkScan = new Timer(20000);      // 20 s
  Timer *checkWiFi = new Timer(8000);         // 8 s
  Timer *reconnectTimeout = new Timer(90000); // 90 s
  Timer *reboot = new Timer(86400000 * 3);    // 3 days
  Timer *multiBtnHold = new Timer(500);       // 500 ms, 2Hz
  Timer *cleanSockets = new Timer(1000);      // 1 s
  Timer *rotorUpdate = new Timer(40);         // 40 ms, 25 Hz
  Timer *rotorMessage = new Timer(1000);      // 1 s
  Timer *speedRamp = new Timer(40);           // 40 ms, 25 Hz
  Timer *updateScreen = new Timer(40);        // 40 ms, 25 Hz
  Timer *loopTimer = new Timer(1000);         // 1 s
  Timer *fwUpdateChecker = new Timer(50);     // 50 ms, 20 Hz
} timers;

bool is_reconnecting = false;   // Is WiFi trying to reconnect
float adc_volts_prev = -1;      // Previous loop ADC-Volts
bool is_rotating_prev = false;  // Was rotor rotating in previous loop cycle
int clients_connected_prev;     // N of clients connected in previous loop cycle
bool is_updating_prev = false;  // Was firmware updating in previous loop cycle

unsigned long loopCounter = 0;
unsigned long loop_mus = micros();



void loop() {
  if (COUNT_LOOP_CYCLE_TIME) loopCounter++;

  // *********** Button ***********
  // ******************************

  // Check for multi button being pressed down
  if (multi_btn_pressed && !multi_btn_hold && !firmware.is_updating) {
    Serial.println("[BTN] pressed.");

    // Toggle screen
    if (has_screen && use_screen) {
        screen.toggleScreens();
    }

    // Stop rotor
    if (rotor_ctrl.is_rotating) {
      rotor_ctrl.stop();
      wifi_led.blinkBlocking(1, 250ul);
    }

    // Enable further checks for wether multi button is being held down
    multi_btn_pressed = false;
    multi_btn_hold = true;
    timers.multiBtnHold->reset();
    timers.multiBtnHold->start();
  }

  // Check wether button is being held down
  if (multi_btn_hold && !firmware.is_updating) {
    // Button was released
    if (digitalRead(multi_button_pin)) {
      multi_btn_hold = false;

    // Button is still being held down
    } else if (timers.multiBtnHold->passed()) {
      // Reset WiFi after timer 2 s
      if (timers.multiBtnHold->n_passed == 4) {
        rotor_ctrl.stop();
        wifi_led.blinkBlocking(4, 250ul);
        Serial.println("[BTN] held for 2s. Resetting WiFi credentials and restart.");
        resetCredentials();
        ESP.restart();
      }
    }
  }



  // *********** Rotor ***********
  // *****************************

  // Update rotor values and send rotation message to clients every second update.
  // Update angular speed every 8th update.
  if (in_station_mode && timers.rotorUpdate->passed() && !firmware.is_updating) {
    rotor_ctrl.update(!(timers.rotorUpdate->n_passed % 8));

    // Send rotation message every second update and only if clients are connected
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
  if (in_station_mode && rotor_ctrl.is_auto_rotating) {
    rotor_ctrl.watchAutoRotation();
  }

  // Watch active speed ramp
  if (in_station_mode && rotor_ctrl.smooth_speed_active && timers.speedRamp->passed()) {
    rotor_ctrl.watchSmoothSpeedRamp();
  }

  // Stop rotor if all clients disconnected
  if (in_station_mode && !clients_connected && clients_connected_prev) {
    rotor_ctrl.stop();
    Serial.println("[Websocket] ALL clients disconnected.");
    if (has_screen) {
      screen.setAlert("All clients disc.");
    }
    clients_connected_prev = 0;
  } else {
    clients_connected_prev = clients_connected;
  }



  // ************ WiFi ************
  // ******************************

  // Check for loss of WiFi connection -> stop rotor, try to reconnect
  if (in_station_mode && timers.checkWiFi->passed()) {
    if (!WiFi.isConnected()) {
      // Stop rotor
      if (rotor_ctrl.is_rotating) {
        rotor_ctrl.stop();
      }

      // Start reconnect timeout
      if (!is_reconnecting) {
        timers.reconnectTimeout->start();
        is_reconnecting = true;
      }

      // Try to reconnect
      Serial.println("[WiFi] disconnected! Try reconnecting...");
      WiFi.disconnect();
      wifi_led.blinkBlocking(1, 250ul);
      WiFi.reconnect();
    }
  }

  // Stop timeout if connection was reestablished
  if (in_station_mode && is_reconnecting && WiFi.isConnected()) {
    is_reconnecting = false;
    Serial.println("[WiFi] reconnected.");
  }

  // Reboot ESP after reconnect timeout
  if (in_station_mode && is_reconnecting && timers.reconnectTimeout->passed()) {
    Serial.println("[WiFi] Reconnecting failed! Restarting ESP.");
    delay(1000);
    ESP.restart();
  }



  // ******** Housekeeping ********
  // ******************************

  // Update the screen
  if (has_screen && timers.updateScreen->passed()) {
    screen.update();
  }

  // Blinking-LED tick
  wifi_led.tick();


  // Clean old websocket connections
  if (in_station_mode && timers.cleanSockets->passed()) {
    websocket.cleanupClients();
  }


  // Reboot ESP after a few days
  if (in_station_mode && timers.reboot->passed() && !firmware.is_updating) {
    ESP.restart();
  }



  // ***** Updating  Firmware *****
  // ******************************

  if (in_station_mode && timers.fwUpdateChecker->passed()) {
 
    // When update starts 
    if (!is_updating_prev && firmware.is_updating) {
      websocket.enable(false);
      wifi_led.startBlinking(500);
      firmware.timeout->start();
      is_updating_prev = true;
    }

    // During update
    if (firmware.is_updating) {
      if (firmware.timeout->passed()) {
        Serial.println("[ESP] Update error: Connection lost during upload.");
        delay(1000);
        ESP.restart();
      }
    }

    // When update ends
    if (is_updating_prev && !firmware.is_updating) {
      websocket.enable(true);
      wifi_led.stopBlinking();
      is_updating_prev = false;
    }
  }



  // ********** AP MODE  **********
  // ******************************

  // Scanning for networks
  if (!in_station_mode) {
    if (scan_now || timers.networkScan->passed()) {
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

  if (timers.loopTimer->passed() && COUNT_LOOP_CYCLE_TIME) {
    double loop_cycle = (float) ((micros() - loop_mus) / loopCounter) / 1000.0;
    Serial.print("[ESP] Loop cycle time: ");
    Serial.printf("%3.2f", loop_cycle);
    Serial.println(" ms");
    loopCounter = 0;
    loop_mus = micros();
  }
}
