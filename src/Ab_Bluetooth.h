#ifndef Ab_Bluetooth_h
#define Ab_Bluetooth_h

#include "Ab_LCD.h"

extern SerialLCD LCD;

class Ab_Bluetooth
{
public:
    void getDataFromClientDevice(String cmd);
    String selectFlightWithRFID();
    unsigned int state = 1;
    unsigned int page = 0;
    bool isReadyToSend = false;
    String req_flight = "";
    String sel_flight = "";
    String job_state = "";
    String scan_state = "";

private:
};

#endif
