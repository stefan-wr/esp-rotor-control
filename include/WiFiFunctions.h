#ifndef WIFIFUNCTIONS_H
#define WIFIFUNCTIONS_H

#include<Arduino.h>
#include<ESPAsyncWebServer.h>
#include<DNSServer.h>


namespace WiFiFunctions {

    // =============
    // Configuration
    // =============

    // WiFi credentials
    struct WiFiConfig {
        String ssid;
        String bssid;
        String password;
        uint8_t bssid_uint8t[6];
    };

    extern struct WiFiConfig wifi_config;

    // => Clear WiFi credentials with empty strings
    bool resetCredentials();

    // =========
    // Utilities
    // =========

    // => Return ip url from current AP IP
    String get_ip_url();

    // =============
    // Networks Scan
    // =============

    // => Start async scan for WiFi-networks
    void startNetworkScan();
    // => Check for completion of async network scan and create <ul> of found networks
    void watchNetworkScan();

    // ======
    // Server
    // ======

    // => Initialise WiFi connection from saved parameters.
    // @return true if success, false if connecting fails
    bool initWiFi();

    // => Start AP mode server and ask for local WiFi credentials.
    // @return true if success, false if server could no be started
    bool startAPServer(AsyncWebServer *server, DNSServer &dns_server);
}

#endif // WIFIFUNCTIONS_H
