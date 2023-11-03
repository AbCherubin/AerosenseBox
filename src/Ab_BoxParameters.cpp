#include "Ab_BoxParameters.h"

void BoxParameters::initialize()
{
  id = "";
  signalStrength = "";
  dateTime = "";
  speed = "";
  acceleration = "";
  accelerationIndex = 0;
  latitude = "";
  longitude = "";
  engineStateADC = "";
  operationSensorADC = "";
  fuelLevelADC = "";
  engineTemperatureADC = "";
  temperature = "";
  engineMinutes = "";
  rfid = "";
  distance = "";
  GSEID = "";
}

String BoxParameters::prepareDataOutput()
{
  DynamicJsonDocument jsonDoc(1024);
  JsonObject root = jsonDoc.to<JsonObject>();
  appendParameter(root, "id", id);
  appendParameter(root, "date_time", dateTime);
  appendParameter(root, "speed", speed);
  appendParameter(root, "acceleration", acceleration);
  appendParameter(root, "latitude", latitude);
  appendParameter(root, "longitude", longitude);
  appendParameter(root, "rssi", signalStrength);
  appendParameter(root, "state", engineStateADC);
  appendParameter(root, "fuel", fuelLevelADC);
  appendParameter(root, "vehicle_status", operationSensorADC);
  appendParameter(root, "sensor_1", engineTemperatureADC);
  appendParameter(root, "operation_time", engineMinutes);
  appendParameter(root, "distance", distance);
  String output;
  serializeJson(root, output);
  return output;
}

void BoxParameters::appendParameter(JsonObject &json, const char *key, const String &value)
{
  if (!value.isEmpty())
  {
    json[key] = value;
  }
}
String BoxParameters::getMqttPayload()
{
  String payload = prepareDataOutput();
  int str_len = payload.length() + 1;
  char char_array[str_len];
  payload.toCharArray(char_array, str_len);
  return payload;
}

void BoxParameters::writeLongIntoEEPROM(int address, long number)
{
  EEPROM.write(address, (number >> 24) & 0xFF);
  EEPROM.write(address + 1, (number >> 16) & 0xFF);
  EEPROM.write(address + 2, (number >> 8) & 0xFF);
  EEPROM.write(address + 3, number & 0xFF);
  EEPROM.commit();
}

long BoxParameters::readLongFromEEPROM(int address)
{
  return ((long)EEPROM.read(address) << 24) +
         ((long)EEPROM.read(address + 1) << 16) +
         ((long)EEPROM.read(address + 2) << 8) +
         (long)EEPROM.read(address + 3);
}