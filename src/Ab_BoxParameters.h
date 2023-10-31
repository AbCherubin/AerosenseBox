#ifndef Ab_BoxParameters_h
#define Ab_BoxParameters_h

#include <Arduino.h>
#include <ArduinoJson.h>

class BoxParameters
{
public:
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
  String rfid;
  void initialize();
  String prepareDataOutput();
  String getMqttPayload();
  String authenticationJsonObject(String id, String type, String cardNo);

private:
  void appendParameter(JsonObject &json, const char *key, const String &value);
};

#endif