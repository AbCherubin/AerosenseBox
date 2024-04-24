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
#include <Ab_Wiegand.h>
#include <ArduinoJson.h>
#include <Ab_HTTP.h>
#include <ESP32Time.h>

#define pincr1 32
#define pincr2 33

#define LCD_RX 16
#define LCD_TX 17

Ab_WIEGAND wg;
SerialLCD LCD;
AccelerometerLib acc(12345);

#define EEPROM_SIZE 16
BoxParameters box;

#define SerialToLCD Serial2
ESP32Time rtc(7 * 3600);

// MQTT details
String id = BOX_ID;
const char *mqtt_id = MQTT_CLIENT_ID;
unsigned int MQTT_interval = MQTT_TIMEOUT;
unsigned int port = MQTT_HOST_PORT;
const char *broker = MQTT_GATEWAY;
const char *mqttUsername = MQTT_USER;
const char *mqttPassword = MQTT_PASS;

String SERVER_TOKEN = _SERVER_TOKEN;
unsigned long card_previousMillis = 0;
const long card_duration = 300;
bool isSubscribeTopics = false;

Ab_MQTTClient mqttClient(mqtt_id, broker, port, mqttUsername, mqttPassword);
TaskHandle_t sensors;

Ab_HTTPClient httpClient;

long lastMqttCounter = 0;

long lastEngineCounter = 0;

long lastGPSCounter = 0;

int engineSecondCount = 0;

int engine_flag = 0;

unsigned long sleepStartTime = 0;
bool sleepInProgress = true;
unsigned long sleepDuration = 5 * 60000;

void subscribeTopics(String GSEID)
{
  String boxTopic = "client/aerosensebox/" + GSEID;
  String authenTopic = "client/authentication/" + GSEID;
  // String flights = "client/response/flight/short/" + GSEID;
  String flights = "client/tasklist/" + GSEID;
  String assignment = "client/myassignment/" + GSEID;
  String flightCheck = "client/flight/check/";
  String vehicleStatus = "client/vehiclestatus/2P8045";
  String topics[] = {boxTopic, authenTopic, flights, assignment};
  int maxAttempts = 5;
  int retryDelay = 100;
  for (int i = 0; i < sizeof(topics) / sizeof(topics[0]); i++)
  {
    if (mqttClient.subscribeWithRetry(topics[i].c_str(), maxAttempts, retryDelay))
    {
      Serial.print("Subscribed to topic: ");
      Serial.println(topics[i]);
    }
    else
    {
      isSubscribeTopics = false;
      return;
    }
  }
  isSubscribeTopics = true;
}

hw_timer_t *My_timer = NULL;
void IRAM_ATTR onTimer()
{
  LCD.timer_flag = 1;
  if (LCD.page >= 1)
  {
    Serial.println("timerAlarmDisable");
    timerDetachInterrupt(My_timer); // Detach the interrupt first
    timerAlarmDisable(My_timer);    // Disable the timer
  }

  //  if (LCD.page == 5)
  //  {
  //    if (LCD.job_step < 2)
  //    {
  //      LCD.job_step++;
  //    }
  //  }
  //  else
  //  {
  //    LCD.job_step = 0;
  //  }
}

void setup()
{
  Serial.begin(115200);
  SerialToLCD.begin(115200, SERIAL_8N1, LCD_RX, LCD_TX);
  wg.begin(pincr1, pincr2);
  pinMode(ADC4_pin, INPUT); // Input ADC4 Engine Start-Stop
  Wire.begin();
  SHT40.begin();
  EEPROM.begin(EEPROM_SIZE);
  box.initialize();
  box.id = mqtt_id;

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(BUZZER_LED_PIN, OUTPUT);
  digitalWrite(BUZZER_LED_PIN, LOW);

  // box.writeLongIntoEEPROM(box.ENGINE_HOURS_ADDRESS, 12345678);
  // box.writeDoubleIntoEEPROM(box.DISTANCE_ADDRESS,0);
  acc.setupAccelerometer();
  acc.calibrateSensors(); // Perform calibration once during setup
  box.engineMinCount = box.readLongFromEEPROM(box.ENGINE_HOURS_ADDRESS);
  box.distanceCount = box.readDoubleFromEEPROM(box.DISTANCE_ADDRESS);
  // Initialize the MQTT client
  LCD.popup_reconnecting_on();
  mqttClient.begin();
  if (isnan(box.distanceCount))
  {
    box.writeDoubleIntoEEPROM(box.DISTANCE_ADDRESS, 0);
  }
  if (isnan(box.engineMinCount))
  {
    box.writeLongIntoEEPROM(box.ENGINE_HOURS_ADDRESS, 0);
  }
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

  while (!httpClient.setTime)
  {
    httpClient.getTimeFromAPI();
    if (httpClient.unixtime)
    {
      rtc.setTime(httpClient.unixtime);

      httpClient.setTime = true;
    }
    else
    {
      mqttClient.disconnect();
      ESP.restart();
    }
    delay(100);
  }

  while (box.GSEID == "")
  {
    httpClient.getVehicleAPI(mqtt_id, itafmServerAddress, itafmServerPort, SERVER_TOKEN);
    if (httpClient.vehicleID)
    {
      box.GSEID = httpClient.vehicleID;
      LCD.GSEId = httpClient.vehicleID;
      LCD.unitName = httpClient.unitName;
      subscribeTopics(box.GSEID);
    }
    else
    {
      mqttClient.disconnect();
      ESP.restart();
    }
    delay(100);
  }

  LCD.Datetime = rtc.getTime("%Y-%m-%d %H:%M:%S");
  Serial.println(LCD.Datetime);
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1 * 1000000, true);
  timerAlarmEnable(My_timer);
  String topic_get_task_assignment = "server/request/get_task_assignment/" + box.GSEID;
  mqttClient.publish(topic_get_task_assignment.c_str(), "");
  String topic_tasklist = "server/request/tasklist/" + box.GSEID;
  mqttClient.publish(topic_tasklist.c_str(), "");
  //  Start Time Out
  LCD.timeOutStartTime = millis();
  LCD.timeOutInProgress = true;

  LCD.timer_flag = 1;
}

void Sensors(void *pvParameters)
{

  for (;;)
  {
    serial_receive();
    // Sleep Mode Trigger
    //    if (receive_over_flage == 1)
    //    {
    //      sleepStartTime = millis();
    //      if (sleepInProgress)
    //      {
    //        close_win("Sleep_page");
    //        set_sleep("false");
    //        sleepInProgress = false;
    //      }
    //    }
    //    if (!sleepInProgress && millis() - sleepStartTime >= sleepDuration)
    //    {
    //      sleepInProgress = true;
    //      set_sleep("true");
    //      open_win("Sleep_page");
    //      receive_over_flage = 0;
    //    }
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
    // Engine Start-Stop
    long now = millis();
    engine_flag = digitalRead(ADC4_pin);
    // engine_flag = 1;
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
          double GPS_latitude = GPS.latitude();
          double GPS_longitude = GPS.longitude();
          if (GPS_latitude > GPS.LATITUDE_MIN_THRESHOLD && GPS_longitude > GPS.LONGITUDE_MIN_THRESHOLD &&
              GPS_latitude < GPS.LATITUDE_MAX_THRESHOLD && GPS_longitude < GPS.LONGITUDE_MAX_THRESHOLD)
          {
            lastGPSCounter = now;
            GPS.preLocationlat = GPS.curLocationlat;
            GPS.preLocationlng = GPS.curLocationlng;

            GPS.curLocationlat = GPS_latitude;
            GPS.curLocationlng = GPS_longitude;

            double distance = GPS.haversine(GPS.preLocationlat, GPS.preLocationlng, GPS.curLocationlat, GPS.curLocationlng);
            if (distance > GPS.DISTANCE_MIN_THRESHOLD && distance < GPS.DISTANCE_MAX_THRESHOLD)
            {
              // Serial.print("Distance (km): ");
              // Serial.println(distance, 3);

              // Serial.print("box.distanceCount (km): ");
              // Serial.println(box.distanceCount, 3);

              box.distanceCount += distance;
              box.writeDoubleIntoEEPROM(box.DISTANCE_ADDRESS, box.distanceCount);
            }
            else
            {
              // Serial.println("Small movement detected. Ignoring.");
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
    //  RFID Sound
    //    if (mqttClient.isAuthenSuccess)
    //    {
    //
    //      if (digitalRead(BUZZER_PIN) == LOW)
    //      {
    //        digitalWrite(BUZZER_PIN, HIGH);
    //        card_previousMillis = millis();
    //      }
    //      unsigned long card_currentMillis = millis();
    //      if (card_currentMillis - card_previousMillis >= card_duration)
    //      {
    //        digitalWrite(BUZZER_PIN, LOW);
    //        mqttClient.isAuthenSuccess = false;
    //      }
    //    }
    if (wg.available() && LCD.page == 1)
    {
      String gse = box.GSEID;
      box.rfid = String(wg.getCode(), HEX);
      box.rfid.toUpperCase();
      String rfid = box.rfid;
      Serial.println(box.rfid);
      String payload = "{\"vehicle\":\"" + gse + "\",\"card_no\":\"" + rfid + "\"}";
      int maxAttempts = 3;
      int retryDelay = 100;
      mqttClient.publishWithRetry("server/authentication/", payload.c_str(), maxAttempts, retryDelay);
    }
    if (LCD.timeOutInProgress && millis() - LCD.timeOutStartTime >= LCD.timeOutDuration)
    {
      LCD.timeOutInProgress = false;
      ESP.restart();
    }
    int freeheap = ESP.getFreeHeap();
    if (freeheap <= 100000)
    {
      Serial.print(F("Free heap: "));
      Serial.println(ESP.getFreeHeap());
      ESP.restart();
    }
  }
}

void loop()
{

  if (!mqttClient.isConnected())
  {
    //
    ESP.restart();
    //    mqttClient.begin();
    //    Serial.println("MQTT NOT CONNECTED!");
    //    mqttClient.reconnect();
    //    isSubscribeTopics = false;
  }
  else if (!isSubscribeTopics)
  {
    subscribeTopics(box.GSEID);
  }
  else
  {
    // refresh flight
    if (LCD.recheck_flight_list)
    {
      if (box.GSEID != "" && LCD.page == 3)
      {
        String topic = "server/request/tasklist/" + box.GSEID;
        mqttClient.publish(topic.c_str(), "");
        LCD.recheck_flight_list = false;
        // Start Time Out
        LCD.timeOutStartTime = millis();
        LCD.timeOutInProgress = true;
      }
      else
      {
        Serial.println("refresh flight");
        LCD.recheck_flight_list = false;
      }
    }

    // select flight
    if (LCD.isSelectFlight)
    {
      if (LCD.page == 4 && LCD.taskId != "" && LCD.employeeId != "" && LCD.GSEId != "")
      {
        String taskId = String(LCD.taskId);
        String gse = String(LCD.GSEId);
        String employeeId = String(LCD.employeeId);
        String topic = "server/request/create_task_assignment/";
        String payload = "{\"task_id\":\"" + taskId + "\",\"vehicle\":\"" + gse + "\",\"employee_id\":\"" + employeeId + "\"}";
        Serial.println(payload);
        mqttClient.publish(topic.c_str(), payload.c_str());
        LCD.isSelectFlight = false;
        // Start Time Out
        LCD.timeOutStartTime = millis();
        LCD.timeOutInProgress = true;
      }
      else
      {
        Serial.println("select flight failed");
        String taskId = String(LCD.taskId);
        String gse = String(LCD.GSEId);
        String employeeId = String(LCD.employeeId);
        String topic = "server/request/create_task_assignment/";
        String payload = "{\"task_id\":\"" + taskId + "\",\"vehicle\":\"" + gse + "\",\"employee_id\":\"" + employeeId + "\"}";
        Serial.println(payload);
        LCD.isSelectFlight = false;
      }
    }

    // select action
    if (LCD.isStepAction)
    {
      if (LCD.page == 5 && LCD.unitName != "" && LCD.taskId != "")
      {
        String unitName = String(LCD.unitName);
        String taskId = String(LCD.taskId);
        String step = String(LCD.step);
        String topic = "server/request/create_task_action/";
        String payload = "{\"unit\":\"" + unitName + "\",\"task_assignment_id\":\"" + taskId + "\",\"step\":\"" + step + "\"}";
        Serial.println(payload);
        mqttClient.publish(topic.c_str(), payload.c_str());
        LCD.isStepAction = false;
        // Start Time Out
        LCD.timeOutStartTime = millis();
        LCD.timeOutInProgress = true;
      }
      else
      {
        Serial.println("select action failed");
        String unitName = String(LCD.unitName);
        String taskId = String(LCD.taskId);
        String step = String(LCD.step);
        String topic = "server/request/create_task_action/";
        String payload = "{\"unit\":\"" + unitName + "\",\"task_assignment_id\":\"" + taskId + "\",\"step\":\"" + step + "\"}";
        Serial.println(payload);
        LCD.isStepAction = false;
        // back to flight page
        LCD.isCancelTask_Ok = true;
      }
    }

    // cancel_task_assignment
    if (LCD.isCancelTask)
    {
      if (LCD.page == 5 && LCD.taskId != "")
      {
        String taskId = String(LCD.taskId);
        String topic = "server/request/cancel_task_assignment/";
        String payload = "{\"task_assignment_id\":\"" + taskId + "\"}";
        Serial.println(payload);
        mqttClient.publish(topic.c_str(), payload.c_str());
        LCD.isCancelTask = false;
        // Start Time Out
        LCD.timeOutStartTime = millis();
        LCD.timeOutInProgress = true;
      }
      else
      {
        Serial.println("select action failed");
        String taskId = String(LCD.taskId);
        String topic = "server/request/cancel_task_assignment/";
        String payload = "{\"task_assignment_id\":\"" + taskId + "\"}";
        Serial.println(payload);
        LCD.isCancelTask = false;
        // back to flight page
        LCD.isCancelTask_Ok = true;
      }
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

    if ((now - lastMqttCounter > MQTT_interval || lastMqttCounter == 0))
    {
      lastMqttCounter = now;

      mqttClient.loop();
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
      box.distance = String(int(box.distanceCount));
      if (acc.isAccReady())
      {
        double gySum = acc.getGySum();
        box.acceleration = String(gySum);
      }
      String mqttPayload = box.getMqttPayload();
      mqttClient.publish(box.MQTT_SERVER_MAIN_TOPIC, mqttPayload.c_str());
      Serial.println(mqttPayload);
      Serial.print(F("Free heap: "));
      Serial.println(ESP.getFreeHeap());
    }
  }
  mqttClient.loop();
  delay(10);
}