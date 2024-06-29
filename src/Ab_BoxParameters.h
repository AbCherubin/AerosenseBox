#ifndef Ab_BoxParameters_h
#define Ab_BoxParameters_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
class BoxParameters
{
public:
  uint8_t ENGINE_HOURS_ADDRESS = 0;
  uint8_t DISTANCE_ADDRESS = 4;
  const char *MQTT_SERVER_MAIN_TOPIC = "updatestatus";

  String id;
  String signalStrength;
  String dateTime;
  String speed;
  String acceleration;
  unsigned int accelerationIndex;
  String latitude;
  String longitude;
  String engineStateADC;       // Engine start-stop
  String operationSensorADC;   // Operation Sensor
  String fuelLevelADC;         // Fuel Level Sensor
  String engineTemperatureADC; // Engine Temperature Sensor
  String temperature;
  String engineMinutes;
  String distance; // KM
  String rfid;
  String buffer_rfid;

  bool mqttStatus = false;

  String GSEID;
  String driver;

  long engineMinCount = 0;
  double distanceCount = 0;

  void initialize();
  void writeLongIntoEEPROM(int address, long number);
  long readLongFromEEPROM(int address);
  void writeDoubleIntoEEPROM(int address, double number);
  double readDoubleFromEEPROM(int address);
  String prepareDataOutput();
  String getMqttPayload();

private:
  void appendParameter(JsonObject &json, const char *key, const String &value);
};

#endif