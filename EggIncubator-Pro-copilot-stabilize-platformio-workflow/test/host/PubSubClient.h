#pragma once
#include <map>
#include <set>
#include <string>
#include "Arduino.h"
#include "WiFiClient.h"

class PubSubClient;
extern PubSubClient *activeClient;

class PubSubClient
{
public:
    using Callback = void (*)(char *, byte *, unsigned int);
    explicit PubSubClient(WiFiClient &) { activeClient = this; }
    void setServer(const char *, uint16_t) {}
    void setKeepAlive(int) {}
    void setSocketTimeout(int) {}
    bool setBufferSize(int) { return true; }
    void setCallback(Callback value) { callback = value; }
    bool connected() const { return online; }
    bool connect(const char *, const char *, const char *, const char *, int, bool, const char *)
    {
        online = true;
        return true;
    }
    void disconnect() { online = false; }
    int state() const { return 0; }
    bool subscribe(const char *topic) { subscriptions.insert(topic); return true; }
    bool publish(const char *topic, const char *message, bool) { publications[topic] = message; return true; }
    void loop() {}
    void inject(std::string topic, std::string message)
    {
        callback(&topic[0], reinterpret_cast<byte *>(&message[0]), message.size());
    }
    std::map<std::string, std::string> publications;
    std::set<std::string> subscriptions;
private:
    bool online = false;
    Callback callback = nullptr;
};
