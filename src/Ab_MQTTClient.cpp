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
    mqttClient.connect(id.c_str(), username, password);
    mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length)
                           { this->callback(topic, payload, length); });
}
bool Ab_MQTTClient::isConnected()
{
    return mqttClient.connected();
}
void Ab_MQTTClient::disconnect()
{
    mqttClient.disconnect();
}
void Ab_MQTTClient::loop()
{
    mqttClient.loop();
}
void Ab_MQTTClient::publish(const char *topic, const char *payload)
{
    bool checker = mqttClient.publish(topic, payload);
    // Serial.println(checker);
}
bool Ab_MQTTClient::publishWithRetry(const char *topic, const char *payload, int maxAttempts, int retryDelay)
{
    for (int attempts = 0; attempts < maxAttempts; ++attempts)
    {
        if (mqttClient.publish(topic, payload) && mqttClient.connected())
        {
            Serial.println("Successfully published to topic: " + String(topic));

            return true; // Exit the function if subscribed successfully
        }

        Serial.println("Failed to published to topic. Retrying...");
        delay(retryDelay);
    }
    Serial.println("Failed to published after multiple attempts. Exiting.");
    return false;
}
void Ab_MQTTClient::subscribe(const char *topic)
{
    bool checker = mqttClient.subscribe(topic);
    Serial.println(checker);
}
bool Ab_MQTTClient::subscribeWithRetry(const char *topic, int maxAttempts, int retryDelay)
{
    for (int attempts = 0; attempts < maxAttempts; ++attempts)
    {
        if (mqttClient.subscribe(topic) && mqttClient.connected())
        {
            Serial.println("Successfully subscribed to topic: " + String(topic));
            return true; // Exit the function if subscribed successfully
        }

        Serial.println("Failed to subscribe to topic. Retrying...");
        delay(retryDelay);
    }
    Serial.println("Failed to subscribe after multiple attempts. Exiting.");
    return false;
}

void Ab_MQTTClient::reconnect()
{

    // Loop until reconnected
    do
    {
        Serial.print("Attempting MQTT connection...");
        if (mqttClient.connect(id.c_str(), username, password))
        {
            Serial.println("connected");

            delay(100);
            mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length)
                                   { this->callback(topic, payload, length); });
            attemptCount = 0;
            delay(10);
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
    } while (!mqttClient.connected());
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

    if (jsonDataFromServer["source"] == "Task observer")
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
            flight["id"] = obj["id"];
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
        // LCD.flight_list = "[{\"flight\":\"WE 040\",\"std\":\"06:50\",\"etd\":\"06:50\",\"bay\":\"A1\",\"gate\":\"A1\"},{\"flight\":\"TG 002\",\"std\":\"06:55\",\"etd\":\"--:--\",\"bay\":\"B6\",\"gate\":\"B6\"},{\"flight\":\"WE 020\",\"std\":\"07:00\",\"etd\":\"07:00\",\"bay\":\"A3\",\"gate\":\"A3\"},{\"flight\":\"TG 323\",\"std\":\"07:00\",\"etd\":\"--:--\",\"bay\":\"E6\",\"gate\":\"E6\"},{\"flight\":\"WE 259\",\"std\":\"07:10\",\"etd\":\"10:05\",\"bay\":\"A1\",\"gate\":\"B7\"},{\"flight\":\"NH 806\",\"std\":\"07:10\",\"etd\":\"07:10\",\"bay\":\"E4\",\"gate\":\"E4\"},{\"flight\":\"TG 102\",\"std\":\"07:25\",\"etd\":\"--:--\",\"bay\":\"B3\",\"gate\":\"B3\"},{\"flight\":\"TG 586\",\"std\":\"07:30\",\"etd\":\"--:--\",\"bay\":\"D8\",\"gate\":\"D8\"},{\"flight\":\"WE 241\",\"std\":\"07:35\",\"etd\":\"07:35\",\"bay\":\"A4\",\"gate\":\"A4\"},{\"flight\":\"TG 289\",\"std\":\"07:40\",\"etd\":\"--:--\",\"bay\":\"F1\",\"gate\":\"B2B\"},{\"flight\":\"TG 560\",\"std\":\"07:45\",\"etd\":\"--:--\",\"bay\":\"G1\",\"gate\":\"G1\"},{\"flight\":\"TG 620\",\"std\":\"07:45\",\"etd\":\"--:--\",\"bay\":\"C9\",\"gate\":\"S111A\"},{\"flight\":\"TG 588\",\"std\":\"07:45\",\"etd\":\"--:--\",\"bay\":\"E1\",\"gate\":\"E1\"},{\"flight\":\"TG 550\",\"std\":\"07:45\",\"etd\":\"--:--\",\"bay\":\"E2\",\"gate\":\"E2\"},{\"flight\":\"MU 9622\",\"std\":\"07:50\",\"etd\":\"07:50\",\"bay\":\"509L\",\"gate\":\"E2A\"},{\"flight\":\"MS 511\",\"std\":\"07:55\",\"etd\":\"08:00\",\"bay\":\"516\",\"gate\":\"-\"},{\"flight\":\"TG 201\",\"std\":\"08:00\",\"etd\":\"--:--\",\"bay\":\"B5\",\"gate\":\"B5\"},{\"flight\":\"TG 403\",\"std\":\"08:00\",\"etd\":\"--:--\",\"bay\":\"G5\",\"gate\":\"G5\"},{\"flight\":\"TG 652\",\"std\":\"08:00\",\"etd\":\"--:--\",\"bay\":\"C9\",\"gate\":\"C9\"},{\"flight\":\"TG 676\",\"std\":\"08:00\",\"etd\":\"--:--\",\"bay\":\"D6\",\"gate\":\"D6\"},{\"flight\":\"TG 600\",\"std\":\"08:00\",\"etd\":\"--:--\",\"bay\":\"E3\",\"gate\":\"E3\"},{\"flight\":\"JL 708\",\"std\":\"08:05\",\"etd\":\"08:05\",\"bay\":\"G3\",\"gate\":\"G3\"}]";
        // LCD.flight_list_size = 22;
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
    StaticJsonDocument<256> jsonDataFromServer;
    DeserializationError error = deserializeJson(jsonDataFromServer, payload, length);

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
                box.engineMinCount = jsonDataFromServer["result"];
                box.writeLongIntoEEPROM(box.ENGINE_HOURS_ADDRESS, box.engineMinCount);
                Serial.print("Engine Hours: ");
                Serial.println(box.engineMinCount);
                box.engineMinutes = String(box.engineMinCount);
                String mqttPayload = box.getMqttPayload();
                mqttClient.publish(box.MQTT_SERVER_MAIN_TOPIC, mqttPayload.c_str());
                Serial.println(mqttPayload);
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
                box.distanceCount = jsonDataFromServer["result"];
                box.writeDoubleIntoEEPROM(box.DISTANCE_ADDRESS, box.distanceCount);
                Serial.print("distance Meter: ");
                Serial.println(box.distanceCount);
                box.distance = String(int(box.distanceCount));
                String mqttPayload = box.getMqttPayload();
                mqttClient.publish(box.MQTT_SERVER_MAIN_TOPIC, mqttPayload.c_str());
                Serial.println(mqttPayload);
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

void Ab_MQTTClient::handleAuthentication(char *topic, byte *payload, unsigned int length)
{
    StaticJsonDocument<256> jsonDataFromServer;
    DeserializationError error = deserializeJson(jsonDataFromServer, payload, length);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        LCD.driverLoginFailed = true;
        return;
    }

    String level = jsonDataFromServer["level"];

    if (level != "success")
    {

        Serial.println("Error");
        LCD.driverLoginFailed = true;
        return;
    }

    String firstName = jsonDataFromServer["first_name"];
    String lastName = jsonDataFromServer["last_name"];
    String vehicle = jsonDataFromServer["vehicle"];
    String empID = jsonDataFromServer["employee_id"];
    String driverName = firstName + " " + lastName;
    Serial.println("driverName " + driverName + empID);

    isAuthenSuccess = true;

    LCD.Driver = driverName;
    LCD.employeeId = empID;
    LCD.driverLoginFailed = false;
    LCD.isLogin = true;
}
void Ab_MQTTClient::handleMyassignment(char *topic, byte *payload, unsigned int length)
{
    StaticJsonDocument<200> filter;
    filter["event"] = true;
    filter["id"] = true;
    filter["flight"]["flight_number"] = true;
    filter["last_action"] = true;

    StaticJsonDocument<256> jsonDataFromServer;
    // DeserializationError error = deserializeJson(jsonDataFromServer, payload, length);
    DeserializationError error = deserializeJson(jsonDataFromServer, payload, DeserializationOption::Filter(filter));
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        LCD.driverLoginFailed = true;
        return;
    }
    String event = jsonDataFromServer["event"];
    if (event == "get" || event == "create")
    {
        if (!LCD.isInitalTaskReady)
        {
            LCD.isInitalTaskReady = true;
        }

        String id = jsonDataFromServer["id"];
        String currentFlight = jsonDataFromServer["flight"]["flight_number"];
        LCD.currentFlight = currentFlight;

        if (id != "null")
        {
            LCD.isSelectFlight_Ok = true;
            LCD.taskId = id;
            // String flight = jsonDataFromServer["flight"];
            // LCD.currentFlight = flight;
            // เด่ววววว
            Serial.println(event);
            Serial.println("task ID " + LCD.taskId);
            Serial.println(LCD.currentFlight);
            uint8_t step = jsonDataFromServer["last_action"]["step"];
            Serial.print("last step ");
            Serial.println(step);
            // Set recconect Step and Job Step
            if (step < 7)
            {
                if (step == 0)
                {
                    LCD.job_step = 0;
                }
                else if (step % 2 == 1) // 1,3,5
                {
                    LCD.job_step = 1;
                }
                else if (step == 6) // 6
                {
                    LCD.job_step = 2;
                }
                else // 2,4
                {
                    LCD.job_step = 2;
                    set_text("button", "button_dropoff_pickup", "Next Round"); // re-setup
                }

                if (step >= 0 && step < 3)
                {
                    LCD.currentRound = 1;
                }
                else if (step >= 3 && step < 5)
                {
                    LCD.currentRound = 2;
                }
                else
                {
                    LCD.currentRound = 3;
                }

                LCD.step = step + 1;
            }
            Serial.print("job_step ");
            Serial.println(LCD.job_step);
        }
        else
        {
            Serial.println("No task_assignment_id");
        }
    }
    else if (event == "update")
    {
        uint8_t step = jsonDataFromServer["last_action"]["step"];
        Serial.println(event);
        Serial.print("last step ");
        Serial.println(step);
        LCD.isStepAction_Ok = true;
        String currentFlight = jsonDataFromServer["flight"]["flight_number"];
        LCD.currentFlight = currentFlight;
    }
    else if (event == "cancel")
    {
        Serial.println("cancel");
        LCD.isCancelTask_Ok = true;
        LCD.currentFlight = "";
    }
}

void Ab_MQTTClient::callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    LCD.timeOutInProgress = false;
    if (strncmp(topic, "client/response/flight/short/", strlen("client/response/flight/short/")) == 0)
    {
        handleFlightListTopic(topic, payload, length);
    }
    else if (strncmp(topic, "client/tasklist/", strlen("client/tasklist/")) == 0)
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
    else if (strncmp(topic, "client/authentication/", strlen("client/authentication/")) == 0)
    {
        handleAuthentication(topic, payload, length);
    }
    else if (strncmp(topic, "client/myassignment/", strlen("client/myassignment/")) == 0)
    {
        handleMyassignment(topic, payload, length);
    }
    else if (strcmp(topic, "client/flight/check/") == 0)
    {
        handleFlightCheck(topic, payload, length);
    }
}