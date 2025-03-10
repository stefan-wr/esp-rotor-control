#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

#define SERIAL_SPEED 115200

const bool verbose = true;

// I/O pins
const uint8_t wifi_led_pin = 19;        // LED
const uint8_t multi_button_pin = 32;    // Push button

// Rotation pins { CCW (left), CW (right) }
const uint8_t rot_pins[2] = {33, 25};   // { Mini-DIN pin 2, Mini-DIN pin 1 }
const uint8_t speed_pin = 26;           // Mini-DIN pin 3

// AP mode server config
const bool use_custom_ip = true;
const int ip[4] = {192, 168, 4, 1};         // AP mode custom IP
const String ip_url = "http://192.168.4.1"; // AP mode custom IP-URL
const char ap_ssid_name[] = "RotorControl"; // AP mode SSID
const char local_url[] = "rotor";           // mDNS URL

// STATION mode default server config
const uint16_t sta_default_port = 80;
const String sta_default_user = "rotor";
const String sta_default_pw = "password";

// *********
// Externals
// *********
extern bool multi_btn_pressed;
extern bool scan_now;
extern bool is_reconnecting;
extern bool in_station_mode;
extern bool has_screen;
extern bool use_screen;
extern String esp_id;
extern const String version;

#endif //GLOBALS_H
