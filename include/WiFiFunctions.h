#ifndef WIFIFUNCTIONS_H
#define WIFIFUNCTIONS_H

#include<Arduino.h>
#include<ESPAsyncWebServer.h>

// STATION mode global WiFi-credentials
// Defined in WiFiFunctions.cpp
extern String wifi_ssid;
extern String wifi_bssid;
extern String wifi_pw;
extern uint8_t wifi_bssid_uint8[6];

// => Load WiFi credentials from SPIFFS
void loadCredentials();

// => Save credentials to SPIFFS
void saveCredentials();

// => Overwrite the saved credentials with empty strings
void resetCredentials();

// => Convert wifi_bssid BSSID string to uint8_t
bool bssidToUint8();

// => Blink WiFi LED n-times, blocking
void blinkWifiLed(const int n);

// => Create HTML <li> item for a scanned network
String networkItemHTML(const String &ssid, const String &bssid, const int &rssi);

// => Scan for WiFi-networks and create <ul> of found networks
void scanNetworks(String &networks_html);

// => Initialise WiFi connection from saved parameters.
// Return false when connecting fails.
bool initWiFi();

// Start AP mode server and ask for local WiFi credentials.
void getCredentials(AsyncWebServer &server);

#endif // WIFIFUNCTIONS_H
