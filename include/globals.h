#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// Serial monitor baud rate
const int serial_speed = 115200;
const bool verbose = true;

// I/O pins
const uint8_t wifi_status_led = 19;
const uint8_t multi_button_pin = 32;
const uint8_t rot_pins[2] = {33, 25};   // 0: ccw/left, 1: cw/right
const uint8_t safety_stop_pin = 13;
const uint8_t speed_pin = 26;

// AP mode server config
const bool use_custom_ip = true;
const int ip[4] = {192, 168, 4, 1};         // AP mode custom IP
const String ip_url = "http://192.168.4.1"; // AP mode custom IP-URL
const String ap_ssid_name = "RotorControl"; // AP mode SSID
const char local_url[] = "rotor";           // mDNS URL

// STATION mode default server config
const int sta_default_port = 80;
const String sta_default_user = "rotor";
const String sta_default_pw = "password";

// External state variables
extern bool multi_btn_pressed;
extern bool scan_now;
extern bool in_station_mode;

// External ESP ID
extern String esp_id;

#endif //GLOBALS_H
