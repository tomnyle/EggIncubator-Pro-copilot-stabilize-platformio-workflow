#pragma once
#include "Arduino.h"
struct FakeIPAddress { String toString() const { return String("192.0.2.1"); } };
struct FakeWiFi
{
    FakeIPAddress localIP() const { return FakeIPAddress(); }
    int RSSI() const { return -40; }
};
extern FakeWiFi WiFi;
