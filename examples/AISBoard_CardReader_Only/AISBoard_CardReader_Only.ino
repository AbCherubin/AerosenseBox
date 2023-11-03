#include "configs.h"
#include <Arduino.h>
#include <Ab_GPS.h>
#include <Storage.h>
#include <SHT40.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Ab_AccelerometerLib.h>
#include <Ab_MQTTClient.h>
#include <Ab_BoxParameters.h>
#include <Wiegand.h>
#include <ArduinoJson.h>
#include <Ab_HTTP.h>

#define pincr1 32
#define pincr2 33

WIEGAND wg;
SerialLCD LCD;
AccelerometerLib acc(12345);

#define EEPROM_SIZE 16
BoxParameters box;

// MQTT details
String id = BOX_ID;
const char *mqtt_id = MQTT_CLIENT_ID;
unsigned int MQTT_interval = MQTT_TIMEOUT;
unsigned int port = MQTT_HOST_PORT;
const char *broker = MQTT_GATEWAY;
const char *mqttUsername = MQTT_USER;
const char *mqttPassword = MQTT_PASS;
const char *topicOutput1 = MQTT_TOPIC;

bool isSubscribeTopics = false;
Ab_MQTTClient mqttClient(mqtt_id, broker, port, mqttUsername, mqttPassword);
TaskHandle_t sensors;

Ab_HTTPClient httpClient;

unsigned long lastReconnectAttempt = 0;

long lastMqttCounter = 0;

long lastEngineCounter = 0;

long lastGPSCounter = 0;

int engineSecondCount = 0;

int engine_flag = 0;

void subscribeTopics(String GSEID)
{
  String topic1 = "client/aerosensebox/" + GSEID;
  String topics[] = {topic1};
  for (int i = 0; i < sizeof(topics) / sizeof(topics[0]); i++)
  {
    mqttClient.subscribe(topics[i].c_str());
    Serial.print("Subscribed to topic: ");
    Serial.println(topics[i]);
  }
  isSubscribeTopics = true;
}

void setup()
{
  Serial.begin(115200);
  wg.begin(pincr1, pincr2);
  pinMode(ADC4_pin, INPUT); // Input ADC4 Engine Start-Stop
  Wire.begin();
  SHT40.begin();
  EEPROM.begin(EEPROM_SIZE);
  box.initialize();
  box.id = mqtt_id;
  // box.writeLongIntoEEPROM(box.ENGINE_HOURS_ADDRESS, 12345678);
  acc.setupAccelerometer();
  acc.calibrateSensors(); // Perform calibration once during setup
  box.engineMinCount = box.readLongFromEEPROM(box.ENGINE_HOURS_ADDRESS);
  box.distanceCount = box.readDoubleFromEEPROM(box.DISTANCE_ADDRESS);
  // Initialize the MQTT client
  mqttClient.begin();
  Serial.print("Engine Count (mins) :");
  Serial.println(String(box.engineMinCount));
  Serial.print("box.distanceCount (km) :");
  Serial.println(String(box.distanceCount, 3));
  xTaskCreatePinnedToCore(
      Sensors,   /* Task function. */
      "sensors", /* name of task. */
      15000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &sensors,  /* Task handle to keep track of created task */
      1);        /* pin task to core 0 */
  delay(2000);

  while (!GPS.begin())
  {
    Serial.println("GPS setup fail");
    delay(100);
  }
}

void Sensors(void *pvParameters)
{

  for (;;)
  {
    // Engine Start-Stop
    long now = millis();
    engine_flag = digitalRead(ADC4_pin);
    engine_flag = 1;
    switch (engine_flag)
    {
    case 1:
      box.engineStateADC = "ON";
      break;
    default:
      box.engineStateADC = "FF";
      break;
    }

    if (box.engineStateADC == "ON")
    {

      if (lastEngineCounter == 0)
      {
        lastEngineCounter = now;
      }
      else if (now - lastEngineCounter > 1000)
      {
        lastEngineCounter = now;
        engineSecondCount++;
        if (engineSecondCount >= 60)
        {
          engineSecondCount = 0;
          box.engineMinCount++;
          box.writeLongIntoEEPROM(box.ENGINE_HOURS_ADDRESS, box.engineMinCount);
        }
      }
      if (lastGPSCounter == 0)
      {
        lastGPSCounter = now;
      }
      else if (now - lastGPSCounter > 5000)
      {
        if (GPS.available())
        {
          lastGPSCounter = now;
          GPS.preLocationlat = GPS.curLocationlat;
          GPS.preLocationlng = GPS.curLocationlng;

          GPS.curLocationlat = GPS.latitude();
          GPS.curLocationlng = GPS.longitude();

          if (GPS.preLocationlat != 0.0 && GPS.preLocationlng != 0.0)
          {
            double distance = GPS.haversine(GPS.preLocationlat, GPS.preLocationlng, GPS.curLocationlat, GPS.curLocationlng);
            Serial.print("Distance (km): ");
            Serial.println(distance, 6);
            box.distanceCount = box.distanceCount + distance;
            Serial.print("box.distanceCount (km): ");
            Serial.println(box.distanceCount, 6);
            box.writeDoubleIntoEEPROM(box.DISTANCE_ADDRESS, box.distanceCount);
          }
        }
      }
    }
    else
    {
      lastEngineCounter = 0;
      lastGPSCounter = 0;
    }
    /////////////////////////////////////////////

    if (wg.available())
    {
      box.rfid = String(wg.getCode(), HEX);
      box.rfid.toUpperCase();
      Serial.println(box.rfid);
      if (box.GSEID != "")
      {
        if (httpClient.postDriverAPI(box.rfid, box.GSEID))
        {
          box.driver = httpClient.driverName;
        }
        else
        {
          Serial.println("PostDriverAPI Unsuceessful");
        }
      }
      else
      {
        Serial.println("No GSEID");
      }
    }
    ///////////////////////////////////////
    delay(10);
  }
}

void loop()
{

  if (!mqttClient.isConnected())
  {
    Serial.println("MQTT NOT CONNECTED!");
    mqttClient.reconnect();
    isSubscribeTopics = false;
  }
  else
  {

    if (box.GSEID == "")
    {
      httpClient.getVehicleAPI(mqtt_id);
      if (httpClient.vehicleID)
      {
        box.GSEID = httpClient.vehicleID;
        subscribeTopics(box.GSEID);
      }
    }
    if (!isSubscribeTopics && box.GSEID != "")
    {
      subscribeTopics(box.GSEID);
    }

    long now = millis();
    if (box.engineStateADC == "FF")
    {
      if (MQTT_interval != MQTT_STANDBY_TIME)
      {
        MQTT_interval = MQTT_STANDBY_TIME;
        lastMqttCounter = 0;
        Serial.println("STANDBY MODE<60s>");
      }
    }
    else
    {

      if (MQTT_interval != MQTT_TIMEOUT)
      {
        MQTT_interval = MQTT_TIMEOUT;
        lastMqttCounter = 0;
        Serial.println("ACTIVE MODE<10s>");
      }
    }

    if (now - lastMqttCounter > MQTT_interval || lastMqttCounter == 0)
    {

      lastMqttCounter = now;

      if (GPS.available())
      {
        box.latitude = String(GPS.latitude(), 6);
        box.longitude = String(GPS.longitude(), 6);
        box.speed = String(GPS.speed(), 2);
        box.dateTime = GPS.getDateTime();
      }
      else
      {
        box.dateTime = "";
        box.latitude = "";
        box.longitude = "";
        box.speed = "";
        GPS.wakeup();
      }
      box.signalStrength = Network.getSignalStrength();
      box.engineTemperatureADC = SHT40.readTemperature();
      box.engineMinutes = String(box.engineMinCount);
      Serial.println(box.distanceCount, 3);
      box.distance = String(box.distanceCount, 3);
      if (acc.isAccReady())
      {
        double gySum = acc.getGySum();
        box.acceleration = String(gySum);
      }
      String mqttPayload = box.getMqttPayload();
      mqttClient.publish(box.MQTT_SERVER_MAIN_TOPIC, mqttPayload.c_str());
      Serial.println(mqttPayload);
      // client.publish("updatebox", "Hello world");
      delay(100);
    }
  }
  mqttClient.loop();
}