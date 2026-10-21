/*
 * MAX30102 Pulse Oximeter & Heart Rate Driver Header
 */

#ifndef MAX30102_DRIVER_H
#define MAX30102_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "config.h"

struct PhysiologicalReadings {
  float spo2;
  float heartRate;
  float signalQuality; // 0.0 to 1.0 signal validity
  bool fingerDetected;
  bool isValid;
  bool oxygenDesaturation;
};

class MAX30102Driver {
private:
  MAX30105 particleSensor;
  PhysiologicalReadings currentReadings;
  bool initialized;
  long lastBeatTime;
  float rateBuffer[4];
  uint8_t rateIdx;

public:
  MAX30102Driver();
  bool begin();
  PhysiologicalReadings update();
  PhysiologicalReadings getReadings() const { return currentReadings; }
  bool isInitialized() const { return initialized; }
  void resetData();
};

#endif // MAX30102_DRIVER_H
