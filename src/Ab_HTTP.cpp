#include "Ab_HTTP.h"

void Ab_HTTPClient::getTimeFromAPI()
{
    Serial.println("getTimeFromAPI");

    HttpClient client = HttpClient(gsmClient, timeServerAddress, timeServerPort);
    client.setTimeout(1000);
    client.get(timeServerAPI);

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
    client.stop();
    Serial.print("Status Code: ");
    Serial.println(statusCode);

    if (statusCode == 200)
    {
        StaticJsonDocument<100> filter;
        filter["unixtime"] = true;
        // Parse the JSON response

        StaticJsonDocument<400> jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, response, DeserializationOption::Filter(filter));

        if (!error)
        {
            // Extract the "unixtime" value from the JSON
            unixtime = jsonDoc["unixtime"];
        }
        else
        {
            Serial.println("JSON parsing error: " + String(error.c_str()));
        }
    }

    // Print the extracted values
    Serial.print("Unixtime: ");
    Serial.println(unixtime);
}

void Ab_HTTPClient::getVehicleAPI(String id)
{
    Serial.println("getVehicleAPI");

    HttpClient client = HttpClient(gsmClient, itafmServerAddress, itafmServerPort);

    client.setTimeout(3000);
    String apiUrl = get_vehicle_api + id;

    client.beginRequest();
    client.get(apiUrl);

    client.sendHeader("Authorization", "Bearer " + SERVER_TOKEN);
    client.sendHeader("Content-Type", "application/json");
    client.endRequest();
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
    Serial.println(apiUrl);
    Serial.print("Status Code: ");
    Serial.println(statusCode);
    Serial.println(response);
    client.stop();
    if (statusCode == 200)
    {
        // DynamicJsonDocument jsonDoc(2048);
        // DeserializationError error = deserializeJson(jsonDoc, response);
        StaticJsonDocument<100> filter;
        filter["results"][0]["name"] = true;
        // Parse the JSON response

        StaticJsonDocument<200> jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, response, DeserializationOption::Filter(filter));

        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        // Extract data from the JSON response
        JsonObject results = jsonDoc["results"][0]; // Assuming there's only one result

        if (!results.isNull())
        {
            // long id = results["id"];
            // String name = results["name"];
            // String issi = results["issi"];
            // String unitName = results["unit"]["name"];
            String vehicleName = results["vehicle"]["name"];
            // String vehicleStatus = results["vehicle"]["status"];

            // You can now use the extracted data as needed
            // Serial.print("ID: ");
            // Serial.println(id);
            // Serial.print("Name: ");
            // Serial.println(name);
            // Serial.print("ISSI: ");
            // Serial.println(issi);
            // Serial.print("Unit Name: ");
            // Serial.println(unitName);
            Serial.print("Vehicle Name: ");
            Serial.println(vehicleName);
            // Serial.print("Vehicle Status: ");
            // Serial.println(vehicleStatus);
            vehicleID = vehicleName;
        }
        else
        {
            Serial.println("No data found in response.");
        }
    }
}

bool Ab_HTTPClient::postDriverAPI(String card_no, String vehicle)
{
    Serial.println("postDriverAPI");
    StaticJsonDocument<64> payloadDoc;
    JsonObject payload = payloadDoc.to<JsonObject>();
    payload["vehicle"] = vehicle;
    payload["card_no"] = card_no;

    String payloadString;
    serializeJson(payload, payloadString);
    HttpClient client = HttpClient(gsmClient, itafmServerAddress, itafmServerPort);
    String apiUrl = post_vehicle_card_api;
    client.beginRequest();
    client.post(apiUrl);
    client.sendHeader("Authorization", "Bearer " + SERVER_TOKEN);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", payloadString.length());
    client.beginBody();
    client.print(payloadString);
    Serial.println(payloadString);
    client.endRequest();

    Serial.println("endRequest");
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
    client.stop();
    Serial.println(apiUrl);
    Serial.print("Status Code: ");
    Serial.println(statusCode);
    Serial.println(response);

    if (statusCode == 200)
    {
        StaticJsonDocument<200> filter;
        filter["user_profile"]["first_name"] = true;
        filter["user_profile"]["last_name"] = true;

        // Deserialize the document with the filter
        StaticJsonDocument<400> jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, response, DeserializationOption::Filter(filter));

        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return false; // Return false on JSON parse error
        }

        // Extract data from the JSON response
        // int id = jsonDoc["id"];
        // String name = jsonDoc["name"];
        JsonObject user_profile = jsonDoc["user_profile"];
        String first_name = user_profile["first_name"];
        String last_name = user_profile["last_name"];
        // String unit = user_profile["unit"];
        // String employee_id = user_profile["employee_id"];
        String driver = first_name + " " + last_name;
        // // Now you can use the extracted data as needed
        // Serial.print("ID: ");
        // Serial.println(id);
        // Serial.print("Name: ");
        // Serial.println(name);
        // Serial.print("First Name: ");
        // Serial.println(first_name);
        // Serial.print("Last Name: ");
        // Serial.println(last_name);
        // Serial.print("Unit: ");
        // Serial.println(unit);
        // Serial.print("Employee ID: ");
        // Serial.println(employee_id);
        // strlcpy(driverName, driver, sizeof(name));

        return true; // Return true for successful operation
    }
    else
    {
        Serial.println("No data found in response.");
        return false; // Return false on non-200 status code
    }
}

bool Ab_HTTPClient::postTaskAPI(String flight_id, String step, String vehicle)
{
    Serial.println("postTaskAPI");
    StaticJsonDocument<64> payloadDoc;
    JsonObject payload = payloadDoc.to<JsonObject>();
    payload["flight_id"] = flight_id;
    payload["step"] = step;
    payload["vehicle"] = vehicle;

    String payloadString;
    serializeJson(payload, payloadString);
    HttpClient client = HttpClient(gsmClient, itafmServerAddress, itafmServerPort);
    String apiUrl = post_vehicle_task_api;
    client.beginRequest();
    client.post(apiUrl);
    client.sendHeader("Authorization", "Bearer " + SERVER_TOKEN);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", payloadString.length());
    client.beginBody();
    client.print(payloadString);
    client.endRequest();

    Serial.println(payloadString);
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
    client.stop();
    Serial.println(apiUrl);
    Serial.print("Status Code: ");
    Serial.println(statusCode);
    Serial.println(response);

    if (statusCode == 200)
    {

        return true; // Return true for successful operation
    }
    else
    {
        Serial.println("No data found in response.");
        return false; // Return false on non-200 status code
    }
}
