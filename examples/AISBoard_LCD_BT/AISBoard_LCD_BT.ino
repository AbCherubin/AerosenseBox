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
#include <Ab_LCD.h>
#include <Wiegand.h>
#include <ArduinoJson.h>
#include <Ab_HTTP.h>
#include <ESP32Time.h>
#include <Ab_Bluetooth.h>
#include <BluetoothSerial.h>

#define LCD_RX 16
#define LCD_TX 17

#define pincr1 32
#define pincr2 33

ESP32Time rtc(7 * 3600);

WIEGAND wg;
SerialLCD LCD;
Ab_Bluetooth bt;
BluetoothSerial SerialBT;

#define SerialToLCD Serial2
AccelerometerLib acc(12345);

#define EEPROM_SIZE 8
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
  String topic1 = "client/response/flight/short/" + GSEID;
  String topic2 = "client/flight/check/";
  String topic3 = "client/vehiclestatus/2P8045";
  String topic4 = "client/aerosensebox/" + GSEID;
  String topics[] = {topic1, topic2, topic3, topic4};
  for (int i = 0; i < sizeof(topics) / sizeof(topics[0]); i++)
  {
    mqttClient.subscribe(topics[i].c_str());
    Serial.print("Subscribed to topic: ");
    Serial.println(topics[i]);
  }
  isSubscribeTopics = true;
}

hw_timer_t *My_timer = NULL;
void IRAM_ATTR onTimer()
{
  LCD.timer_flag = 1;
  LCD.Datetime = rtc.getTime("%Y-%m-%d %H:%M:%S");
  if (LCD.page == 4)
  {
    if (LCD.job_step < 3)
    {
      LCD.job_step++;
    }
  }
  else
  {
    LCD.job_step = 0;
  }
  Serial.print(F("Free heap: "));
  Serial.println(ESP.getFreeHeap());
  Serial.println(ESP.getMaxAllocHeap());
}

void setup()
{
  Serial.begin(115200);
  SerialToLCD.begin(115200, SERIAL_8N1, LCD_RX, LCD_TX);
  SerialBT.begin("AeroSense");
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
  LCD.is4G = (Network.getDeviceIP() != IPAddress(0, 0, 0, 0));
  LCD.isSensor = acc.isAccReady();
  LCD.isServer = mqttClient.isConnected();
  LCD.isRFID = (digitalRead(pincr1) == HIGH || digitalRead(pincr2) == HIGH);
  LCD.isTemp = (SHT40.readTemperature() != 0.0);
  LCD.isGPS = GPS.available();
  LCD.Username = "";
  LCD.GSEId = "";
  LCD.Datetime = "";
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 5 * 1000000, true);
  timerAlarmEnable(My_timer);
}

void Sensors(void *pvParameters)
{

  for (;;)
  {
    serial_receive();

    switch (LCD.page)
    {
    case 0:
      LCD.page0();
      break;
    case 1:
      LCD.page1();
      break;
    case 2:
      LCD.page2();
      break;
    case 3:
      LCD.page3();
      break;
    case 4:
      LCD.page4();
      break;
    case 5:
      LCD.page5();
      break;
    default:
      LCD.page0();
    }

    if (SerialBT.connected())
    {
      String cmd = SerialBT.readStringUntil('\n');
      Serial.println(cmd);
      bt.getDataFromClientDevice(cmd);
      if (bt.isReadyToSend == true)
      {
        switch (bt.page)
        {
        case 1:
          SerialBT.println(bt.req_flight);
          break;
        case 2:
          SerialBT.println(bt.sel_flight);
          break;
        case 3:
          SerialBT.println(bt.job_state);
          break;
        default:
          Serial.println("bt");
        }
        bt.isReadyToSend = false;
      }
    }

    // Engine Start-Stop
    long now = millis();
    engine_flag = digitalRead(ADC4_pin);
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
          Serial.println("GPS");
          if (GPS.preLocationlat != 0.0 && GPS.preLocationlng != 0.0)
          {
            double distance = GPS.haversine(GPS.preLocationlat, GPS.preLocationlng, GPS.curLocationlat, GPS.curLocationlng);
            if (distance > GPS.DISTANCE_THRESHOLD)
            {
              Serial.print("Distance (km): ");
              Serial.println(distance, 3);
              box.distanceCount = box.distanceCount + distance;
              Serial.print("box.distanceCount (km): ");
              Serial.println(box.distanceCount, 3);
              box.writeDoubleIntoEEPROM(box.DISTANCE_ADDRESS, box.distanceCount);
            }
            else
            {
              Serial.println("Small movement detected. Ignoring.");
            }
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
      if (LCD.page == 3)
      {
        LCD.popup_loading_on();
        if (httpClient.postDriverAPI(box.rfid, LCD.GSEId))
        {
          LCD.Driver = httpClient.driverName;
          LCD.driverLoginFailed = false;
          LCD.isLogin = true;
        }
        else
        {
          LCD.driverLoginFailed = true;
        }
        LCD.popup_loading_off();
      }

      if (bt.page == 2)
      {
        SerialBT.println(bt.selectFlightWithRFID());
        bt.isReadyToSend = false;
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

    if (!httpClient.setTime)
    {
      httpClient.getTimeFromAPI();
      if (httpClient.unixtime)
      {
        rtc.setTime(httpClient.unixtime);
        httpClient.setTime = true;
      }
    }
    if (LCD.GSEId == "")
    {
      httpClient.getVehicleAPI(mqtt_id);
      if (httpClient.vehicleID)
      {
        LCD.GSEId = httpClient.vehicleID;
        subscribeTopics(LCD.GSEId);
        String topic = "server/request/flight/short/" + LCD.GSEId;
        mqttClient.publish(topic.c_str(), "");
        SerialBT.end();
        SerialBT.begin(LCD.GSEId);
      }
    }
    if (!isSubscribeTopics && LCD.GSEId != "")
    {
      subscribeTopics(LCD.GSEId);
    }
    if (LCD.recheck_flight_list && LCD.GSEId != "" && LCD.page == 2)
    {
      String topic = "server/request/flight/short/" + LCD.GSEId;
      mqttClient.publish(topic.c_str(), "");
      LCD.recheck_flight_list = false;
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
      box.distance = String(int(box.distanceCount));
      if (acc.isAccReady())
      {
        double gySum = acc.getGySum();
        box.acceleration = String(gySum);
      }
      String mqttPayload = box.getMqttPayload();
      mqttClient.publish("updatestatus", mqttPayload.c_str());
      Serial.println(mqttPayload);
      // client.publish("updatebox", "Hello world");
      delay(100);
    }
  }
  mqttClient.loop();
}
