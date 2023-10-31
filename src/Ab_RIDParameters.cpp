#include "Ab_RIDParameters.h"


void RIDParameters::initialize() {
  id_type="";
  ua_type="";
  uas_id="";
  operator_id_type="";
  operator_id="";
  description_type="";
  description="";
  status="";
  direction="";      
  speed_horizontal="";  
  speed_vertical="";        
  latitude="";
  longitude="";
  altitude_barometric="";
  altitude_geodetic="";
  height_type="";
  height="";
  horizontal_accuracy="";
  vertical_accuracy="";
  baro_alt_accuracy="";
  speed_accuracy="";
  timestamp="";
}

String RIDParameters::prepareDataOutput() {
  DynamicJsonDocument jsonDoc(5000);
  JsonObject root = jsonDoc.to<JsonObject>();
  appendParameter(root, "id_type", id_type);
  appendParameter(root, "ua_type", ua_type);
  appendParameter(root, "uas_id", uas_id);
  appendParameter(root, "operator_id_type", operator_id_type);
  appendParameter(root, "operator_id", operator_id);
  appendParameter(root, "description_type", description_type);
  appendParameter(root, "description", description);
  appendParameter(root, "status", status);       
  appendParameter(root, "direction", direction);
  appendParameter(root, "speed_horizontal", speed_horizontal);
  appendParameter(root, "speed_vertical", speed_vertical);
  appendParameter(root, "latitude", latitude);
  appendParameter(root, "longitude", longitude);
  appendParameter(root, "altitude_barometric", altitude_barometric);
  appendParameter(root, "altitude_geodetic", altitude_geodetic);
  appendParameter(root, "height_type", height_type);
  appendParameter(root, "height", height);
  appendParameter(root, "horizontal_accuracy", horizontal_accuracy);
  appendParameter(root, "vertical_accuracy", vertical_accuracy);
  appendParameter(root, "baro_alt_accuracy", baro_alt_accuracy);
  appendParameter(root, "speed_accuracy", speed_accuracy);
  appendParameter(root, "timestamp", timestamp);
  String output;
  serializeJson(root, output);
  return output;
}

void RIDParameters::appendParameter(JsonObject& json, const char* key, const String& value) {
  if (!value.isEmpty()) {
    json[key] = value;
  }
}
String RIDParameters::getMqttPayload() {
  String payload = prepareDataOutput();
  int str_len = payload.length() + 1;
  char char_array[str_len]; 
  payload.toCharArray(char_array, str_len);
  return payload;
}

