#include "Ab_AccelerometerLib.h"

AccelerometerLib::AccelerometerLib(uint8_t sensorID) : accel(sensorID) {}

bool AccelerometerLib::setupAccelerometer() {
  if (!accel.begin()) {
    Serial.println("No ADXL345 detected. Check your wiring!");
    return false;
  }
  accel.setRange(ADXL345_RANGE_16_G);
  return true;
}

bool AccelerometerLib::calibrateSensors() {
  for (int i = 0; i < numReadings; i++) {
    sensors_event_t event;
    accel.getEvent(&event);

    readingsx[i] = event.acceleration.x;
    readingsy[i] = event.acceleration.y;
    readingsz[i] = event.acceleration.z;
    totalx += readingsx[i];
    totaly += readingsy[i];
    totalz += readingsz[i];
    delay(10);
  }

  averagex = totalx / numReadings;
  averagey = totaly / numReadings;
  averagez = totalz / numReadings;

  Serial.println("Calibration complete!");
  Serial.println(averagex);
  Serial.println(averagey);
  Serial.println(averagez);

  calibrated = true;
  return true;
}
bool AccelerometerLib::isAccReady() {
  return calibrated;
}
double AccelerometerLib::getGySum() {
  if (!calibrated) {
    return 0.0;
  }

  sensors_event_t event;
  accel.getEvent(&event);

  double diffx = event.acceleration.x - averagex;
  double diffy = event.acceleration.y - averagey;
  double diffz = event.acceleration.z - averagez;
  gySum = sqrt(diffx * diffx + diffy * diffy + diffz * diffz);
  return gySum;
}