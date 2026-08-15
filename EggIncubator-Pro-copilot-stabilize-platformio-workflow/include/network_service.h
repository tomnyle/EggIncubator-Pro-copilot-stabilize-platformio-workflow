#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>

class NetworkService
{
public:

    /******************************************************
     * Constructor
     ******************************************************/
    NetworkService();

    /******************************************************
     * Service
     ******************************************************/
    void begin();

    void loop();

    /******************************************************
     * Status
     ******************************************************/
    bool connected() const;

    /******************************************************
     * Information
     ******************************************************/
    String ip() const;

    String mac() const;

    String hostname() const;

    int32_t rssi() const;

private:

    /******************************************************
     * Disable Copy
     ******************************************************/
    NetworkService(const NetworkService&) = delete;
    NetworkService& operator=(const NetworkService&) = delete;

    /******************************************************
     * Internal
     ******************************************************/
    void connectWiFi();

    void updateStatus();

    /******************************************************
     * Variables
     ******************************************************/
    bool wifiConnected = false;

    unsigned long lastReconnect = 0;

    static constexpr unsigned long RECONNECT_INTERVAL = 5000;
};

#endif