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
#include <Favorites.h>        // Exposes Global: favorites
#include <Timer.h>
#include <Screen.h>           // Exposes Global: screen
#include <Firmware.h>         // Exposes Global: firmware
#include <BlinkingLED.h>
#include <Stats.h>
#include <RotorSocket.h>      // Exposes Global: websocket 
#include <RotorServer.h>      // Exposes Global: rotor_server 

#define HAS_SCREEN true
//#define COUNT_LOOP_CYCLE_TIME

// Software version
const String version = "0.9.8";

// ESP ID
String esp_id = "";

// Create instances
BlinkingLED wifi_led(wifi_led_pin, LOW, 250);

// Create DNS Server
DNSServer dns_server;

// Buffer storing lock message. This default message resets lock on ESP restart
String lock_msg = MSG_ID_LOCK;

// State variables
bool in_station_mode = true;        // ESP is connected with WiFi -> STA mode
bool scan_now = false;              // Scan for networks now
bool multi_btn_pressed = false;     // Set true by pressing interrupt button
bool multi_btn_hold = false;        // True if button is being held
bool has_screen = HAS_SCREEN;       // Device has an SSD1306 screen
bool use_screen = HAS_SCREEN;       // Use the SSD1306 screen


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


// ----------------------------------------------------------------------------------
// SETUP ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
void setup() {
  // Open serial port
  Serial.begin(SERIAL_SPEED);

  // Software version
  Serial.print("\n\r##########  RotorControl v");
  Serial.print(version);
  Serial.println("  ##########");

  // Boot cycle counter
  Stats::Counter boot_counter = Stats::Counter("boot");
  boot_counter.add(1);
  Serial.print("[Stats] Boot #: ");
  boot_counter.printlnToSerial();

  // Firmware MD5 and size
  Serial.print("[ESP] Firmware MD5: ");
  Serial.println(ESP.getSketchMD5());
  Serial.print("[ESP] Firmware size: ");
  Serial.println(ESP.getSketchSize());

  // ESP ID derived from MAC address
  esp_id = WiFi.macAddress();
  esp_id.replace(":", "");
  Serial.print("[ESP] Hardware-ID: ");
  Serial.println(esp_id);

  // Show CPU Model & Frequency
  if (verbose) {
    Serial.print("[ESP] Chip model: ");
    Serial.println(ESP.getChipModel());
    Serial.print("[ESP] CPU frequency: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println(" MHz");
    Serial.print("[Websocket] queue size: ");
    Serial.println(WS_MAX_QUEUED_MESSAGES);
  }

  // Initialisations
  // ---------------

  // Init Settings message buffer
  Settings::initBuffer();

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

  // Favorites & firmware
  favorites.init();
  firmware.init();    

  // Default lock message, resets lock on ESP boot
  lock_msg += "|{\"isLocked\":false,\"by\":\"\"}";         

  // Start WiFi connection
  // ---------------------
  if (!WiFiFunctions::initWiFi()) {
    // WiFi connection failed -> launch ESP in AccessPoint WiFi mode
    wifi_led.invert();
    if (!WiFiFunctions::startAPServer(rotor_server.server, dns_server)) {
      fatalError("Failed to start WiFi in AP mode! Try resetting WiFi with button, or reflash firmware.");
    }
  } else {
    // WiFi connection established
    wifi_led.off();

    // Setup & start server
    // --------------------
    rotor_server.init();
    rotor_server.printConfig();

    Serial.print("[Server] started on http://");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.println(rotor_server.config.port);
    Serial.println();
  }
}




// ----------------------------------------------------------------------------------
// LOOP -----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

// Timers, that control how often each task in the mainloop is executed
struct {                          // Intervals
  Timer networkScan{20000};       // 20 s
  Timer checkWiFi{8000};          // 8 s
  Timer reconnectTimeout{90000};  // 90 s
  Timer reboot{86400000 * 3};     // 3 days
  Timer multiBtnHold{500};        // 500 ms, 2Hz
  Timer cleanSockets{1000};       // 1 s
  Timer rotorUpdate{40};          // 40 ms, 25 Hz
  Timer rotorMessage{1000};       // 1 s
  Timer speedRamp{40};            // 40 ms, 25 Hz
  Timer updateScreen{40};         // 40 ms, 25 Hz
  Timer loopTimer{1000};          // 1 s
  Timer fwUpdateChecker{50};      // 50 ms, 20 Hz
  Timer justBootedTimeout{8000};
} timers;

bool is_reconnecting = false;   // Is WiFi trying to reconnect
float adc_volts_prev = -1;      // Previous loop ADC-Volts
bool is_rotating_prev = false;  // Was rotor rotating in previous loop cycle
uint8_t clients_connected_prev; // N of clients connected in previous loop cycle
bool is_updating_prev = false;  // Was firmware updating in previous loop cycle
bool just_booted = true;

#ifdef COUNT_LOOP_CYCLE_TIME
unsigned long loopCounter = 0;
unsigned long loop_mus = micros();
#endif



void loop() {
  #ifdef COUNT_LOOP_CYCLE_TIME
  loopCounter++;
  #endif

  if (timers.justBootedTimeout.n_passed < 2 && timers.justBootedTimeout.passed()) {
    just_booted = false;
  }

  // *********** Button ***********
  // ******************************

  // Check for multi button being pressed down
  if (multi_btn_pressed && !multi_btn_hold && !firmware.is_updating) {
    Serial.println("[BTN] pressed.");

    // Toggle screen if rotor is not rotating
    if (has_screen && use_screen && !rotor_ctrl.is_rotating) {
      screen.toggleScreens();
    }

    // Stop rotor
    if (rotor_ctrl.is_rotating) {
      rotor_ctrl.stop();
      wifi_led.blink(1, 250ul);
    }

    // Enable further checks for wether multi button is being held down
    multi_btn_pressed = false;
    multi_btn_hold = true;
    timers.multiBtnHold.reset();
    timers.multiBtnHold.start();
  }

  // Check wether button is being held down
  if (multi_btn_hold && !firmware.is_updating) {
    // Button was released
    if (digitalRead(multi_button_pin)) {
      multi_btn_hold = false;

    // Button is still being held down
    } else if (timers.multiBtnHold.passed()) {
      // Reset WiFi after timer 2 s
      if (timers.multiBtnHold.n_passed == 4) {
        rotor_ctrl.stop();
        wifi_led.blinkBlocking(4, 250ul);
        Serial.println("[BTN] held for 2s. Resetting WiFi credentials and restart.");
        WiFiFunctions::resetCredentials();
        ESP.restart();
      }
    }
  }



  // *********** Rotor ***********
  // *****************************

  // Update rotor values and send rotation message to clients every second update.
  // Update angular speed every 8th update.
  if (in_station_mode && timers.rotorUpdate.passed() && !firmware.is_updating) {
    rotor_ctrl.update(!(timers.rotorUpdate.n_passed % 8));

    // Send rotation message every second update and only if clients are connected
    if (RotorSocket::clients_connected && timers.rotorUpdate.n_passed % 2 == 0) {
      /* Send rotation message if either:
          1. voltage changed significantly compared to last message
          2. rotor started or stopped rotation
          3. last message was sent more than 1 second ago
      */
      if ((abs(rotor_ctrl.rotor.last_adc_volts - adc_volts_prev) > 0.003) ||
          (rotor_ctrl.is_rotating != is_rotating_prev) ||
          (timers.rotorMessage.passed())) {
        rotor_ctrl.messenger.sendLastRotation(true);
        adc_volts_prev = rotor_ctrl.rotor.last_adc_volts;
        is_rotating_prev = rotor_ctrl.is_rotating;
        timers.rotorMessage.start();
      }      
    }
  }

  // Watch active auto rotation
  if (in_station_mode && rotor_ctrl.is_auto_rotating) {
    rotor_ctrl.watchAutoRotation();
  }

  // Watch active speed ramp
  if (in_station_mode && rotor_ctrl.smooth_speed_active && timers.speedRamp.passed()) {
    rotor_ctrl.watchSmoothSpeedRamp();
  }

  // Stop rotor if all clients disconnected
  if (in_station_mode && !RotorSocket::clients_connected && clients_connected_prev) {
    rotor_ctrl.stop();
    Serial.println("[Websocket] ALL clients disconnected.");
    if (has_screen) {
      screen.setAlert("All clients disc.");
    }
    clients_connected_prev = 0;
  } else {
    clients_connected_prev = RotorSocket::clients_connected;
  }



  // ************ WiFi ************
  // ******************************

  // Check for loss of WiFi connection -> stop rotor, try to reconnect
  if (in_station_mode && timers.checkWiFi.passed()) {
    if (!WiFi.isConnected()) {
      // Stop rotor
      if (rotor_ctrl.is_rotating) {
        rotor_ctrl.stop();
      }

      // Start reconnect timeout
      if (!is_reconnecting) {
        timers.reconnectTimeout.start();
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
  if (in_station_mode && is_reconnecting && timers.reconnectTimeout.passed()) {
    Serial.println("[WiFi] Reconnecting failed! Restarting ESP.");
    delay(1000);
    ESP.restart();
  }



  // ******** Housekeeping ********
  // ******************************

  // Update the screen
  if (has_screen && timers.updateScreen.passed()) {
    screen.update();
  }

  // Blinking-LED tick
  wifi_led.tick();


  // Clean old websocket connections
  if (in_station_mode && timers.cleanSockets.passed()) {
    websocket.cleanupClients();
  }


  // Reboot ESP after a few days
  if (in_station_mode && timers.reboot.passed() && !firmware.is_updating) {
    ESP.restart();
  }



  // ***** Updating  Firmware *****
  // ******************************

  if (in_station_mode && timers.fwUpdateChecker.passed()) {
 
    // When update starts 
    if (!is_updating_prev && firmware.is_updating) {
      websocket.enable(false);
      wifi_led.startBlinking(500);
      firmware.timeout.start();
      is_updating_prev = true;
    }

    // During update
    if (firmware.is_updating) {
      if (firmware.timeout.passed()) {
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
    if (scan_now || timers.networkScan.passed()) {
      WiFiFunctions::startNetworkScan();
      scan_now = false;
    }
    WiFiFunctions::watchNetworkScan();
  }

  // DNS Server
  if (!in_station_mode) {
    dns_server.processNextRequest();
  }



  // ****** Loop Cycle Time  ******
  // ******************************

  #ifdef COUNT_LOOP_CYCLE_TIME
  if (timers.loopTimer.passed()) {
    double loop_cycle = (float) ((micros() - loop_mus) / loopCounter) / 1000.0;
    Serial.print("[ESP] Loop cycle time: ");
    Serial.printf("%3.2f", loop_cycle);
    Serial.println(" ms");
    loopCounter = 0;
    loop_mus = micros();
  }
  #endif
}
