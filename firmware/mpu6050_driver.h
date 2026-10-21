/*
 * MPU6050 6-Axis Motion & Sleep Posture Driver Header
 */

#ifndef MPU6050_DRIVER_H
#define MPU6050_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

struct MotionReadings {
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float movementMagnitude; // Instantaneous acceleration vector magnitude (g)
  float restlessnessScore; // Rolling variance index
  BodyPosture posture;
  const char* postureName;
  bool positionChanged;
  bool isMoving;
};

class MPU6050Driver {
private:
  bool initialized;
  uint8_t i2cAddress;
  MotionReadings currentReadings;
  BodyPosture lastPosture;
  float accelHistory[10];
  uint8_t historyIdx;

  void determinePosture(float ax, float ay, float az);

public:
  MPU6050Driver(uint8_t address = 0x68);
  bool begin();
  MotionReadings update();
  MotionReadings getReadings() const { return currentReadings; }
  bool isInitialized() const { return initialized; }
};

#endif // MPU6050_DRIVER_H
