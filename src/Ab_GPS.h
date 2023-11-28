#ifndef __AB_GPS_H__
#define __AB_GPS_H__

#include "Arduino.h"

class GPSClass
{
private:
    bool isOn();

public:
    GPSClass();

    int begin();
    inline int begin(int mode)
    {
        return begin();
    }
    void end();

    int available();

    float latitude();
    float longitude();
    float speed();     // Speed over the ground in kph
    float course();    // Track angle in degrees
    float variation(); // Magnetic Variation : Not Support
    float altitude();
    int satellites(); // Not support

    unsigned long getTime();
    String getDateTime();

    bool standby();
    bool wakeup();

    double haversine(double lat1, double lon1, double lat2, double lon2);
    double preLocationlat;
    double preLocationlng;
    double pretime;

    double curLocationlat;
    double curLocationlng;
    double curtime;

    float DISTANCE_MIN_THRESHOLD = 0.005;
    float DISTANCE_MAX_THRESHOLD = 0.2;
};

extern GPSClass GPS;

#endif