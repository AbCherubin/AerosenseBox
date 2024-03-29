#ifndef Ab_MQTTClient_h
#define Ab_MQTTClient_h
#include "Ab_LCD.h"
#include "Ab_BoxParameters.h"
#include <GSMClient.h>
#include <PubSubClient.h>
#include <SIM76xx.h>
#include <ArduinoJson.h>

extern SerialLCD LCD;
extern BoxParameters box;
class Ab_MQTTClient
{
public:
    Ab_MQTTClient(const char *id, const char *broker, int port, const char *username, const char *password);
    void begin();
    void loop();
    void publish(const char *topic, const char *payload);
    void subscribe(const char *topic);
    bool isConnected();
    void reconnect();
    bool publishWithRetry(const char *topic, const char *payload, int maxAttempts, int retryDelay);
    bool subscribeWithRetry(const char *topic, int maxAttempts, int retryDelay);
    void disconnect();
    GSMClient gsmClient;
    bool isAuthenSuccess = false;

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
    void handleAerosensebox(char *topic, byte *payload, unsigned int length);
    void handleAuthentication(char *topic, byte *payload, unsigned int length);
    void handleMyassignment(char *topic, byte *payload, unsigned int length);
};

#endif
