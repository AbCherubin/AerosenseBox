#include "Ab_Bluetooth.h"

void Ab_Bluetooth::getDataFromClientDevice(String cmd)
{
    cmd.trim();
    if (cmd.isEmpty())
    {
        Serial.println("Empty");
        return;
    }
    Serial.println(cmd);

    DynamicJsonDocument jsonDoc(ESP.getMaxAllocHeap() - 1024);
    DeserializationError error = deserializeJson(jsonDoc, cmd);
    jsonDoc.shrinkToFit();

    if (error)
    {
        Serial.println("Error parsing JSON");
    }
    else
    {
        if (jsonDoc["cmd_code"] == "req_flight")
        {
            Serial.println(LCD.flight_list);

            DynamicJsonDocument flightDataDoc(ESP.getMaxAllocHeap() - 1024); // Adjust the size as needed
            DeserializationError error = deserializeJson(flightDataDoc, LCD.flight_list);

            if (error)
            {
                Serial.print("Error deserializing flight data: ");
                Serial.println(error.c_str());
            }
            else
            {
                flightDataDoc.shrinkToFit();
                JsonArray flight_list = flightDataDoc.as<JsonArray>();
                DynamicJsonDocument responseDoc(ESP.getMaxAllocHeap() - 1024);
                responseDoc["rpl_code"] = "req_flight_ack";

                JsonArray list = responseDoc.createNestedArray("list");

                for (JsonObject flight : flight_list)
                {
                    list.add(flight);
                }

                String _req_flight;
                serializeJson(responseDoc, _req_flight);
                req_flight = _req_flight;
                Serial.println(req_flight);
                isReadyToSend = true;
                page = 1;
            }
        }
        else if (jsonDoc["cmd_code"] == "sel_flight" && jsonDoc["flight"].is<std::string>())
        {
            const char *targetFlightNumber = jsonDoc["flight"];

            DynamicJsonDocument flightDataDoc(ESP.getMaxAllocHeap() - 1024);
            DeserializationError error = deserializeJson(flightDataDoc, LCD.flight_list);

            if (error)
            {
                Serial.print("Error deserializing flight data: ");
                Serial.println(error.c_str());
            }
            else
            {
                flightDataDoc.shrinkToFit();
                JsonArray flight_list = flightDataDoc.as<JsonArray>();
                DynamicJsonDocument selected_flight(ESP.getMaxAllocHeap() - 1024);

                for (JsonObject flight : flight_list)
                {
                    const char *flightNumber = flight["flight"];
                    Serial.println(flightNumber);
                    if (strcmp(flightNumber, targetFlightNumber) == 0)
                    {
                        selected_flight["rpl_code"] = "sel_flight_ack";
                        selected_flight["job_state"] = "0";
                        selected_flight["flight"] = flightNumber;
                        selected_flight["bay"] = flight["bay"];
                        selected_flight["std"] = flight["std"];
                        selected_flight["etd"] = flight["etd"];
                        String _sel_flight;
                        serializeJson(selected_flight, _sel_flight);
                        sel_flight = _sel_flight;
                        Serial.println(sel_flight);
                        isReadyToSend = true;
                        page = 2;
                        break;
                    }
                }
            }
        }
        else if (jsonDoc["cmd_code"] == "rfh_info" && jsonDoc["flight"].is<std::string>())
        {
            DynamicJsonDocument selected_flight(ESP.getMaxAllocHeap() - 1024);
            DeserializationError error = deserializeJson(selected_flight, sel_flight);
            selected_flight.shrinkToFit();
            const char *targetFlightNumber = jsonDoc["flight"];
            const char *flightNumber = selected_flight["flight"];
            Serial.println(targetFlightNumber);
            Serial.println(flightNumber);
            if (state > 3)
            {
                state = 1;
            }
            if (strcmp(flightNumber, targetFlightNumber) == 0)
            {
                selected_flight["job_state"] = String(state);
                selected_flight["rpl_code"] = "rsh_info_ack";
                String _job_state;
                serializeJson(selected_flight, _job_state);
                job_state = _job_state;
                Serial.println(job_state);
                state++;
            }
            isReadyToSend = true;
            page = 3;
        }
        else
        {
            Serial.println("error");
        }
    }
}

String Ab_Bluetooth::selectFlightWithRFID()
{
    DynamicJsonDocument responseDoc1(ESP.getMaxAllocHeap() - 1024);
    responseDoc1["cmd_code"] = "scan_cmp";
    responseDoc1["scan_state"] = "true";
    String _scan_state;
    serializeJson(responseDoc1, _scan_state);
    scan_state = _scan_state;
    Serial.println(scan_state);
    isReadyToSend = true;
    return scan_state;
}