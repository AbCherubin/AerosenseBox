#ifndef Ab_MQTTClient_h
#define Ab_MQTTClient_h
#include "Ab_LCD.h"
#include <GSMClient.h>
#include <PubSubClient.h>
#include <SIM76xx.h>
#include <ArduinoJson.h>

extern SerialLCD LCD;

class Ab_MQTTClient
{
public:
    Ab_MQTTClient(const char *id, const char *broker, int port, const char *username, const char *password);
    void begin();
    void loop();
    void publish(const char *topic, const char *payload);
    bool isConnected();
    void reconnect();
    void subscribe(const char *topic);
    GSMClient gsmClient;

private:
    PubSubClient mqttClient;
    const String id;
    const char *broker;
    int port;
    const char *username;
    const char *password;
    unsigned int attemptCount = 0;
    bool connected;
    void callback(char *topic, byte *payload, unsigned int length);
    void handleFlightListTopic(char *topic, byte *payload, unsigned int length);
    void handleFlightCheck(char *topic, byte *payload, unsigned int length);
    void handleVehicleStatus(char *topic, byte *payload, unsigned int length);
};

#endif
