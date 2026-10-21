/*
 * MPU6050 6-Axis Motion & Sleep Posture Driver Implementation
 */

#include "mpu6050_driver.h"
#include <math.h>

MPU6050Driver::MPU6050Driver(uint8_t address) 
  : initialized(false), i2cAddress(address), lastPosture(POSTURE_SUPINE), historyIdx(0) {
  memset(&currentReadings, 0, sizeof(MotionReadings));
  currentReadings.posture = POSTURE_SUPINE;
  currentReadings.postureName = "Supine (Back)";
}

bool MPU6050Driver::begin() {
  Serial.println("[MPU6050] Initializing I2C 6-Axis Motion Sensor...");
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Wake up MPU6050 (PWR_MGMT_1 register 0x6B -> write 0)
  Wire.beginTransmission(i2cAddress);
  Wire.write(0x6B);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) {
    Serial.println("[MPU6050] WARNING: Sensor not responding on I2C bus 0x68!");
    initialized = false;
    return false;
  }

  initialized = true;
  Serial.println("[MPU6050] Motion sensor initialized successfully.");
  return true;
}

MotionReadings MPU6050Driver::update() {
  if (!initialized) return currentReadings;

  Wire.beginTransmission(i2cAddress);
  Wire.write(0x3B); // Starting register for Accelerometer & Gyro measurements
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom((int)i2cAddress, 14) != 14) {
    return currentReadings;
  }

  int16_t rawAx = (Wire.read() << 8) | Wire.read();
  int16_t rawAy = (Wire.read() << 8) | Wire.read();
  int16_t rawAz = (Wire.read() << 8) | Wire.read();
  int16_t rawTemp = (Wire.read() << 8) | Wire.read(); (void)rawTemp;
  int16_t rawGx = (Wire.read() << 8) | Wire.read();
  int16_t rawGy = (Wire.read() << 8) | Wire.read();
  int16_t rawGz = (Wire.read() << 8) | Wire.read();

  // Convert raw values to physical units
  currentReadings.accelX = (float)rawAx / MPU6050_ACCEL_SCALE;
  currentReadings.accelY = (float)rawAy / MPU6050_ACCEL_SCALE;
  currentReadings.accelZ = (float)rawAz / MPU6050_ACCEL_SCALE;
  currentReadings.gyroX  = (float)rawGx / 131.0f;
  currentReadings.gyroY  = (float)rawGy / 131.0f;
  currentReadings.gyroZ  = (float)rawGz / 131.0f;

  // Calculate overall acceleration vector magnitude
  float mag = sqrtf(currentReadings.accelX * currentReadings.accelX +
                    currentReadings.accelY * currentReadings.accelY +
                    currentReadings.accelZ * currentReadings.accelZ);
  currentReadings.movementMagnitude = mag;

  // Track restlessness variance
  accelHistory[historyIdx] = mag;
  historyIdx = (historyIdx + 1) % 10;
  float sumMag = 0.0f;
  for (int i = 0; i < 10; i++) sumMag += accelHistory[i];
  float meanMag = sumMag / 10.0f;
  float varMag = 0.0f;
  for (int i = 0; i < 10; i++) varMag += fabsf(accelHistory[i] - meanMag);
  currentReadings.restlessnessScore = varMag / 10.0f;

  currentReadings.isMoving = (currentReadings.restlessnessScore > MPU6050_MOVE_THRESHOLD);

  // Body posture estimation logic
  determinePosture(currentReadings.accelX, currentReadings.accelY, currentReadings.accelZ);

  currentReadings.positionChanged = (currentReadings.posture != lastPosture);
  if (currentReadings.positionChanged) {
    lastPosture = currentReadings.posture;
  }

  return currentReadings;
}

void MPU6050Driver::determinePosture(float ax, float ay, float az) {
  // Posture estimation based on dominant gravity vector orientation
  if (az > 0.6f) {
    currentReadings.posture = POSTURE_SUPINE;
    currentReadings.postureName = "Supine (Back)";
  } else if (az < -0.6f) {
    currentReadings.posture = POSTURE_PRONE;
    currentReadings.postureName = "Prone (Stomach)";
  } else if (ax > 0.5f) {
    currentReadings.posture = POSTURE_LEFT;
    currentReadings.postureName = "Left Side";
  } else if (ax < -0.5f) {
    currentReadings.posture = POSTURE_RIGHT;
    currentReadings.postureName = "Right Side";
  } else {
    currentReadings.posture = POSTURE_UNKNOWN;
    currentReadings.postureName = "In Motion";
  }
}
