/*
 * Sensors Manager Header - Central Sensor Lifecycle & Health Check
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

struct SystemHealth {
  bool inmp441Ok;
  bool max30102Ok;
  bool mpu6050Ok;
  bool tinymlOk;
  bool wifiOk;
  uint32_t uptimeSeconds;
  uint32_t freeHeapBytes;
};

class SensorsManager {
private:
  SystemHealth health;
  bool i2cInitialized;

public:
  SensorsManager();
  bool initI2CBus();
  bool scanI2CDevice(uint8_t address);
  SystemHealth getHealth() const { return health; }
  void setSensorStatus(const char* sensorName, bool status);
  void printSystemStatus();
};

extern SensorsManager sensors;

#endif // SENSORS_H
