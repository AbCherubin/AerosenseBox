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
    void getVehicleAPI(String id, char *itafmServerAddress, int itafmServerPort, String SERVER_TOKEN);

    bool postDriverAPI(String card_no, String vehicle, char *itafmServerAddress, int itafmServerPort, String SERVER_TOKEN);
    bool postTaskAPI(String flight_id, String step, String vehicle, char *itafmServerAddress, int itafmServerPort, String SERVER_TOKEN);
    void printLocalTime();
    String vehicleID = "";
    String driverName = "";
    String unitName = "";
    long unixtime = 0;
    bool setTime = false;

private:
    const char *timeServerAddress = "213.188.196.246";
    int timeServerPort = 80;
    const char *timeServerAPI = "/api/timezone/Asia/Bangkok";

    const char *get_vehicle_api = "/api/asset/?name__icontains=";
    const char *post_vehicle_card_api = "/api/vehicle-card/";
    const char *post_vehicle_task_api = "/api/vehicle-task/";
};

#endif
