#include "network_service.h"

#include "app_config.h"
#include "logger.h"

/******************************************************
 * Constructor
 ******************************************************/

NetworkService::NetworkService()
{
}

/******************************************************
 * Begin
 ******************************************************/

void NetworkService::begin()
{
    WiFi.mode(WIFI_STA);

    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);

    WiFi.setHostname(DEVICE_ID);

    connectWiFi();
}

/******************************************************
 * Loop
 ******************************************************/

void NetworkService::loop()
{
    updateStatus();

    if (wifiConnected)
    {
        return;
    }

    if (millis() - lastReconnect < RECONNECT_INTERVAL)
    {
        return;
    }

    lastReconnect = millis();

    connectWiFi();
}

/******************************************************
 * Connect WiFi
 ******************************************************/

void NetworkService::connectWiFi()
{
    Logger::info("--------------------------------");
    Logger::info("Connecting WiFi...");
    Logger::info("--------------------------------");

    WiFi.disconnect(true, true);

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD);
}

/******************************************************
 * Update Status
 ******************************************************/

void NetworkService::updateStatus()
{
    bool current = (WiFi.status() == WL_CONNECTED);

    if (current == wifiConnected)
    {
        return;
    }

    wifiConnected = current;

    if (wifiConnected)
    {
        Logger::info("--------------------------------");
        Logger::info("WiFi Connected");
        Logger::info("--------------------------------");

        Logger::info(("IP  : " + WiFi.localIP().toString()).c_str());
        Logger::info(("RSSI: " + String(WiFi.RSSI())).c_str());
        Logger::info(("MAC : " + WiFi.macAddress()).c_str());
    }
    else
    {
        Logger::warning("--------------------------------");
        Logger::warning("WiFi Disconnected");
        Logger::warning("--------------------------------");
    }
}

/******************************************************
 * Connected
 ******************************************************/

bool NetworkService::connected() const
{
    return wifiConnected;
}

/******************************************************
 * IP
 ******************************************************/

String NetworkService::ip() const
{
    if (!wifiConnected)
    {
        return "";
    }

    return WiFi.localIP().toString();
}

/******************************************************
 * MAC
 ******************************************************/

String NetworkService::mac() const
{
    return WiFi.macAddress();
}

/******************************************************
 * Hostname
 ******************************************************/

String NetworkService::hostname() const
{
    return WiFi.getHostname();
}

/******************************************************
 * RSSI
 ******************************************************/

int32_t NetworkService::rssi() const
{
    if (!wifiConnected)
    {
        return 0;
    }

    return WiFi.RSSI();
}