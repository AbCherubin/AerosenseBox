#ifndef Ab_RIDParameters_h
#define Ab_RIDParameters_h
#include <stdint.h>
#include <Arduino.h>
#include <ArduinoJson.h>

class RIDParameters {
public:
  String id_type;
  String ua_type;
  String uas_id;
  String operator_id_type;
  String operator_id;
  String description_type;
  String description;
  String status;
  String direction;      
  String speed_horizontal;  
  String speed_vertical;        
  String latitude;
  String longitude;
  String altitude_barometric;
  String altitude_geodetic;
  String height_type;
  String height;
  String horizontal_accuracy;
  String vertical_accuracy;
  String baro_alt_accuracy;
  String speed_accuracy;
  String timestamp;

  void initialize();
  String prepareDataOutput();
  String getMqttPayload();
private:
  void appendParameter(JsonObject& json, const char* key, const String& value);

};

#endif