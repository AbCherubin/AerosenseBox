#ifndef AB_ACCELEROMETER_LIB_H
#define AB_ACCELEROMETER_LIB_H

#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

class AccelerometerLib {
public:
  AccelerometerLib(uint8_t sensorID);

  bool setupAccelerometer();
  bool calibrateSensors();
  double getGySum();
  bool isAccReady();
private:
  Adafruit_ADXL345_Unified accel;
  static const int numReadings = 100;
  double readingsx[numReadings];
  double readingsy[numReadings];
  double readingsz[numReadings];
  double totalx = 0;
  double totaly = 0;
  double totalz = 0;
  double averagex = 0;
  double averagey = 0;
  double averagez = 0;
  bool calibrated = false;
  double gySum;
};

#endif
