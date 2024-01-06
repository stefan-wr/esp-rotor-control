#ifndef WIFIFUNCTIONS_H
#define WIFIFUNCTIONS_H

#include<Arduino.h>
#include<ESPAsyncWebServer.h>
#include<DNSServer.h>

// ESP unique ID derived from MAC address
// Defined in WiFiFunctions.cpp
//extern String esp_id;

// STATION mode global WiFi-credentials
// Defined in WiFiFunctions.cpp
extern String wifi_ssid;
extern String wifi_bssid;
extern String wifi_pw;
extern uint8_t wifi_bssid_uint8[6];

// STATION server config
// Defined in WiFiFunctions.cpp
extern int sta_port;
extern String sta_user_str;
extern String sta_pw_str;

// --------------------------------

// => Load server config in PREFS
void loadServerConfig();

// => Save server config from PREFS
void saveServerConfig();

// => Reset saved server config to default
void resetServerConfig();

// --------------------------------

// => Load WiFi credentials from SPIFFS
void loadCredentials();

// => Save credentials to SPIFFS
void saveCredentials();

// => Overwrite the saved credentials with empty strings
void resetCredentials();

// --------------------------------

// => React to button interrupt for resetting WiFi credentials
void resetCredentialsInterrupt();

// => Convert wifi_bssid BSSID string to uint8_t
bool bssidToUint8();

// => Blink WiFi LED n-times, blocking
void blinkWifiLed(const int n);

// => Create HTML <li> item for a scanned network
String networkItemHTML(const String &ssid, const String &bssid, const int &rssi);

// => Scan for WiFi-networks and create <ul> of found networks
void scanNetworks();

// => Start asnc scan for WiFi-networks
void startNetworkScan();

// => Check for completion of async network scan and create <ul> of found networks
void watchNetworkScan();

// => Initialise WiFi connection from saved parameters.
// Return false when connecting fails.
bool initWiFi();

// => Load server config
void initServerConfig();

// Start AP mode server and ask for local WiFi credentials.
void getCredentials(AsyncWebServer *server, DNSServer &dns_server);

#endif // WIFIFUNCTIONS_H
