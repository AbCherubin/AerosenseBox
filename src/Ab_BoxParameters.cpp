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

String BoxParameters::authenticationJsonObject(String id, String type, String cardNo)
{
  const size_t capacity = JSON_OBJECT_SIZE(5);
  DynamicJsonDocument doc(capacity);
  doc["id"] = id;
  doc["type"] = type;
  doc["card_no"] = cardNo;
  String jsonStr;
  serializeJson(doc, jsonStr);
  return jsonStr;
}