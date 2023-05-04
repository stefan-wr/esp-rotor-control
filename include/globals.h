#ifndef GLOBALS_H
#define GLOBALS_H

// Serial monitor baud rate
const int serial_speed = 115200;

// More verbose output on serial monitor
const bool verbose = true;

// Switches
const int n_switches = 0;

// I/O pins
const uint8_t wifi_status_led = 19;
const uint8_t button_pin = 14;

// 0: ccw/left, 1: cw/right
const uint8_t rot_pins[2] = {32, 33};
const uint8_t safety_stop_pin = 13;
const uint8_t speed_pin = 25;
const String directions[2] = {"CCW", "CW"};

// STATION mode server port
const int sta_port = 1339;

// AP mode server config
const bool use_custom_ip = true;
const int ip[4] = {192, 168, 4, 1};
const String ip_url = "http://192.168.4.1";

// External state variables
extern bool reset_now;
extern bool scan_now;
extern bool in_station_mode;

// Access-Point SSID
extern const char* ap_ssid;

// Local URL for mDNS
extern const char* local_url;

#endif //GLOBALS_H
