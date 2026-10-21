/*
 * Posture & Movement Analytics Header
 * Sensor: MPU6050 6-Axis Motion/IMU (I2C)
 */

#ifndef POSTURE_H
#define POSTURE_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

struct PosturePacket {
  SleepPosture posture;
  const char* postureName;
  float accelX;
  float accelY;
  float accelZ;
  float gyroX;
  float gyroY;
  float gyroZ;
  float movementMagnitude;
  float restlessnessScore;
  bool isMoving;
  bool positionChanged;
  float quality; // Posture Signal Quality Index [0.0 to 1.0]
  bool isValid;
};

class PostureEngine {
private:
  bool initialized;
  PosturePacket currentPacket;
  SleepPosture lastPosture;

  float accelHistory[15];
  uint8_t historyIdx;

  void classifyPosture();
  void calculateRestlessness();
  void evaluateQuality();

public:
  PostureEngine();
  bool initMPU6050();
  bool update();
  PosturePacket getPacket() const { return currentPacket; }
};

extern PostureEngine posture;

#endif // POSTURE_H
