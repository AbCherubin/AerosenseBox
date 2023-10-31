#ifndef Ab_HTTP_h
#define Ab_HTTP_h

#include <GSMClient.h>
#include <PubSubClient.h>
#include <SIM76xx.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>

class Ab_HTTPClient
{
public:
    void getTimeFromAPI();
    void getVehicleAPI(String id);

    bool postDriverAPI(String card_no, String vehicle);
    bool postTaskAPI(String flight_id, String step, String vehicle);
    void printLocalTime();
    String vehicleID = "";
    String driverName = "";
    long unixtime = 0;
    bool setTime = false;

private:
    GSMClient gsmClient;

    const char *timeServerAddress = "213.188.196.246";
    int timeServerPort = 80;
    const char *timeServerAPI = "/api/timezone/Asia/Bangkok";

    const char *itafmServerAddress = "110.77.148.104";
    int itafmServerPort = 14111;
    const char *get_vehicle_api = "/api/asset/?name__icontains=";
    const char *post_vehicle_card_api = "/api/vehicle-card/";
    const char *post_vehicle_task_api = "/api/vehicle-task/";
    //  String SERVER_TOKEN = "voK6xlHqH49roiFhrqq9WCpxXetiOu"; //BAC
    String SERVER_TOKEN = "6qLpIYnEMk71czcVbSeDuLrTFf9uGQ"; // PB
};

#endif
