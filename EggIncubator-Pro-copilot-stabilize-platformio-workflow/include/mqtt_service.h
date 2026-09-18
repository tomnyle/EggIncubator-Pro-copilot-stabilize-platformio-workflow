#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

class MQTTService
{
public:
    MQTTService();

    void begin();
    void loop();
    bool connected();

    bool publish(const char *topic, const char *payload, bool retained = false);
    bool publish(const String &topic, const String &payload, bool retained = false);

    void publishAvailability();
    void publishRelayStates();
    void publishDiagnostics();
    void publishSensorStates();
    void publishFaults();
    void publishAll();

private:
    static void callback(char *topic, byte *payload, unsigned int length);

    void reconnect();
    void subscribeTopics();
    void publishDiscovery();
    void publishBirthMessage();

    WiFiClient wifiClient;
    PubSubClient mqttClient;

    bool mqttConnected;
    bool discoveryPublished;
    uint32_t lastReconnect;
    uint32_t lastPublish;
};

#endif
