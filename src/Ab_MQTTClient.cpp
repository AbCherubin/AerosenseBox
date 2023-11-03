#include "Ab_MQTTClient.h"

Ab_MQTTClient::Ab_MQTTClient(const char *id, const char *broker, int port, const char *username, const char *password)
    : mqttClient(gsmClient), id(id), broker(broker), port(port), username(username), password(password) {}

void Ab_MQTTClient::begin()
{
    while (!GSM.begin())
    {
        Serial.println("GSM setup failed");
        delay(100);
    }

    mqttClient.setServer(broker, port);
    mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length)
                           { this->callback(topic, payload, length); });
    mqttClient.connect(id.c_str(), username, password);
}
bool Ab_MQTTClient::isConnected()
{
    return mqttClient.connected();
}
void Ab_MQTTClient::loop()
{
    mqttClient.loop();
}
void Ab_MQTTClient::publish(const char *topic, const char *payload)
{
    mqttClient.publish(topic, payload);
}

void Ab_MQTTClient::subscribe(const char *topic)
{
    mqttClient.subscribe(topic);
}

void Ab_MQTTClient::reconnect()
{
    // Loop until reconnected
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (mqttClient.connect(id.c_str(), username, password))
        {
            Serial.println("connected");

            attemptCount = 0;
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 2 seconds");
            attemptCount++;
            delay(2000);
        }

        if (attemptCount > 90)
        {
            Serial.println(attemptCount);
            Serial.println("restarting ESP32...");
            ESP.restart();
        }
    }
}

void Ab_MQTTClient::handleFlightListTopic(char *topic, byte *payload, unsigned int length)
{
    DynamicJsonDocument jsonDataFromServer(ESP.getMaxAllocHeap() - 1024);
    DeserializationError error = deserializeJson(jsonDataFromServer, payload, length);
    jsonDataFromServer.shrinkToFit();
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    DynamicJsonDocument flight_list_doc(6000);
    JsonArray flight_list = flight_list_doc.to<JsonArray>();
    LCD.flight_list = "";

    if (jsonDataFromServer["type"] == "Flight")
    {
        JsonArray results = jsonDataFromServer["results"];

        // Create a vector of JsonObjects for sorting
        std::vector<JsonObject> sortedFlights;

        for (JsonObject obj : results)
        {
            sortedFlights.push_back(obj);
        }

        // Create a custom comparison function to compare schedule_flight_time
        auto compareScheduleTime = [](const JsonObject &a, const JsonObject &b) -> bool
        {
            const char *timeA = a["schedule_flight_time"];
            const char *timeB = b["schedule_flight_time"];
            return strcmp(timeA, timeB) < 0;
        };

        // Sort the flight list based on schedule_flight_time
        std::sort(sortedFlights.begin(), sortedFlights.end(), compareScheduleTime);

        for (JsonObject obj : sortedFlights)
        {
            JsonObject flight = flight_list.createNestedObject();
            const char *scheduleFlightTime = obj["schedule_flight_time"];
            String std = String(scheduleFlightTime).substring(16, 11);
            const char *estimateFlightTime = obj["estimate_flight_time"];
            String etd = String(estimateFlightTime).substring(16, 11);
            flight["flight"] = obj["flight_number"];
            flight["std"] = std;
            if (!etd.isEmpty())
            {
                flight["etd"] = etd;
            }
            else
            {
                flight["etd"] = "--:--";
            }
            flight["bay"] = obj["bay"];
            if (obj["gate"] == "")
            {
                flight["gate"] = "-";
            }
            else
            {
                flight["gate"] = obj["gate"];
            }
        }

        LCD.flight_list_size = flight_list.size();
        serializeJson(flight_list, LCD.flight_list);
        Serial.println(LCD.flight_list);
        jsonDataFromServer.clear();
        flight_list_doc.clear();
        LCD.refreshData();
    }
}

void Ab_MQTTClient::handleVehicleStatus(char *topic, byte *payload, unsigned int length)
{
    DynamicJsonDocument jsonDataFromServer(ESP.getMaxAllocHeap() - 1024);
    DeserializationError error = deserializeJson(jsonDataFromServer, payload, length);
    jsonDataFromServer.shrinkToFit();
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    // Extract "finished" value
    bool isFinished = jsonDataFromServer["finished"];

    JsonObject taskObject = jsonDataFromServer["task"];

    String taskStatus = taskObject["status"];

    JsonArray actionsArray = jsonDataFromServer["actions"];

    int currentStep = taskObject["current_step"];
    String currentStepName = taskObject["current_step_name"];

    Serial.print("Finished: ");
    Serial.println(isFinished);
    Serial.print("Task Status: ");
    Serial.println(taskStatus);
    Serial.print("Current Step: ");
    Serial.println(currentStep);
    Serial.print("Current Step Name: ");
    Serial.println(currentStepName);

    String flight = taskObject["flight"];
    String bay = taskObject["bay"];
    String STD = taskObject["STD"];
    String ETD = taskObject["ETD"];

    // You can now work with these extracted fields
    Serial.print("Flight: ");
    Serial.println(flight);
    Serial.print("Bay: ");
    Serial.println(bay);
    Serial.print("STD: ");
    Serial.println(STD);
    Serial.print("ETD: ");
    Serial.println(ETD);
}

void Ab_MQTTClient::handleAerosensebox(char *topic, byte *payload, unsigned int length)
{
    DynamicJsonDocument jsonDataFromServer(ESP.getMaxAllocHeap() - 1024);
    DeserializationError error = deserializeJson(jsonDataFromServer, payload, length);
    jsonDataFromServer.shrinkToFit();

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    if (jsonDataFromServer.containsKey("type"))
    {
        String messageType = jsonDataFromServer["type"];

        if (messageType == "enginehours")
        {
            if (jsonDataFromServer.containsKey("result"))
            {
                long engineHours = jsonDataFromServer["result"];
                box.writeLongIntoEEPROM(box.ENGINE_HOURS_ADDRESS, engineHours);
                Serial.print("Engine Hours: ");
                Serial.println(engineHours);
            }
            else
            {
                Serial.println("Missing 'result' field for 'enginehours' type.");
            }
        }
        else if (messageType == "distance")
        {
            if (jsonDataFromServer.containsKey("result"))
            {
                long distance = jsonDataFromServer["result"];
                box.writeLongIntoEEPROM(box.DISTANCE_ADDRESS, distance);
                Serial.print("distance Meter: ");
                Serial.println(distance);
            }
            else
            {
                Serial.println("Missing 'result' field for 'tripmeter' type.");
            }
        }
        else if (messageType == "restartbox")
        {
            Serial.println("Received 'restartbox' type. Restarting...");
            ESP.restart();
        }
        else
        {
            Serial.println("Unknown message type: " + messageType);
        }
    }
    else
    {
        Serial.println("Missing 'type' field in the JSON message.");
    }
}

void Ab_MQTTClient::handleFlightCheck(char *topic, byte *payload, unsigned int length)
{
    LCD.recheck_flight_list = true;
}

void Ab_MQTTClient::callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    if (strncmp(topic, "client/response/flight/short/", strlen("client/response/flight/short/")) == 0)
    {
        handleFlightListTopic(topic, payload, length);
    }
    else if (strncmp(topic, "client/vehiclestatus/", strlen("client/vehiclestatus/")) == 0)
    {
        handleVehicleStatus(topic, payload, length);
    }
    else if (strncmp(topic, "client/aerosensebox/", strlen("client/aerosensebox/")) == 0)
    {
        handleAerosensebox(topic, payload, length);
    }
    else if (strcmp(topic, "client/flight/check/") == 0)
    {
        handleFlightCheck(topic, payload, length);
    }
}