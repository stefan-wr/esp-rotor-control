#ifndef WIFIFUNCTIONS_H
#define WIFIFUNCTIONS_H

#include<Arduino.h>
#include<ESPAsyncWebServer.h>
#include<DNSServer.h>

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

// => Save server config from PREFS
bool saveServerConfig();

// => Load server config in PREFS
void loadServerConfig();

// => Reset saved server config to default
bool resetServerConfig();

// --------------------------------

// => Save credentials to PREFS
bool saveCredentials();

// => Load WiFi credentials from PREFS
void loadCredentials();

// => Overwrite the saved credentials with empty strings
bool resetCredentials();

// --------------------------------

// => React to button interrupt for resetting WiFi credentials
void resetCredentialsInterrupt();

// => Convert wifi_bssid BSSID string to uint8_t
bool bssidToUint8();

// => Return ip url from current AP IP
String get_ip_url();

// => Create HTML <li> item for a scanned network
String networkItemHTML(const String &ssid, const String &bssid, const int &rssi);

// => Scan for WiFi-networks and create <ul> of found networks
void scanNetworks();

// => Start async scan for WiFi-networks
void startNetworkScan();

// => Check for completion of async network scan and create <ul> of found networks
void watchNetworkScan();

// => Initialise WiFi connection from saved parameters.
// Return false when connecting fails.
bool initWiFi();

// => Load server config
void initServerConfig();

// Start AP mode server and ask for local WiFi credentials.
bool startAPServer(AsyncWebServer *server, DNSServer &dns_server);

#endif // WIFIFUNCTIONS_H
