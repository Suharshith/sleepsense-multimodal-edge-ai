/*
 * Posture & Movement Analytics Implementation
 */

#include "posture.h"
#include <math.h>

PostureEngine posture;

#define MPU6050_REG_PWR_MGMT_1   0x6B
#define MPU6050_REG_ACCEL_XOUT_H 0x3B

PostureEngine::PostureEngine() 
  : initialized(false), lastPosture(POSTURE_BACK), historyIdx(0) {
  memset(&currentPacket, 0, sizeof(PosturePacket));
  memset(accelHistory, 0, sizeof(accelHistory));
  currentPacket.posture = POSTURE_BACK;
  currentPacket.postureName = "BACK";
  currentPacket.quality = 0.95f;
}

bool PostureEngine::initMPU6050() {
  Serial.println("[INFO] Initializing MPU6050 6-Axis Motion Sensor on I2C (Address 0x68)...");

  // Wake up MPU6050 (PWR_MGMT_1 register 0x6B -> write 0)
  Wire.beginTransmission(MPU6050_I2C_ADDR);
  Wire.write(MPU6050_REG_PWR_MGMT_1);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) {
    Serial.println("[WARN] MPU6050 not responding on I2C bus.");
    initialized = false;
    return false;
  }

  initialized = true;
  Serial.println("[INFO] MPU6050 initialized successfully.");
  return true;
}

bool PostureEngine::update() {
  if (!initialized) {
    currentPacket.isValid = false;
    currentPacket.quality = 0.0f;
    return false;
  }

  Wire.beginTransmission(MPU6050_I2C_ADDR);
  Wire.write(MPU6050_REG_ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom((uint8_t)MPU6050_I2C_ADDR, (uint8_t)14) != 14) {
    currentPacket.isValid = false;
    currentPacket.quality = 0.1f;
    return false;
  }

  int16_t rawAx = (Wire.read() << 8) | Wire.read();
  int16_t rawAy = (Wire.read() << 8) | Wire.read();
  int16_t rawAz = (Wire.read() << 8) | Wire.read();
  int16_t rawTemp = (Wire.read() << 8) | Wire.read(); (void)rawTemp;
  int16_t rawGx = (Wire.read() << 8) | Wire.read();
  int16_t rawGy = (Wire.read() << 8) | Wire.read();
  int16_t rawGz = (Wire.read() << 8) | Wire.read();

  // Convert raw readings to physical units (g and deg/s)
  currentPacket.accelX = (float)rawAx / MPU6050_ACCEL_SCALE;
  currentPacket.accelY = (float)rawAy / MPU6050_ACCEL_SCALE;
  currentPacket.accelZ = (float)rawAz / MPU6050_ACCEL_SCALE;
  currentPacket.gyroX  = (float)rawGx / MPU6050_GYRO_SCALE;
  currentPacket.gyroY  = (float)rawGy / MPU6050_GYRO_SCALE;
  currentPacket.gyroZ  = (float)rawGz / MPU6050_GYRO_SCALE;

  // Calculate 3D magnitude
  currentPacket.movementMagnitude = sqrtf(currentPacket.accelX * currentPacket.accelX +
                                          currentPacket.accelY * currentPacket.accelY +
                                          currentPacket.accelZ * currentPacket.accelZ);

  calculateRestlessness();
  classifyPosture();
  evaluateQuality();

  currentPacket.isValid = true;
  return true;
}

void PostureEngine::calculateRestlessness() {
  accelHistory[historyIdx] = currentPacket.movementMagnitude;
  historyIdx = (historyIdx + 1) % 15;

  float sum = 0.0f;
  for (int i = 0; i < 15; i++) sum += accelHistory[i];
  float mean = sum / 15.0f;

  float variance = 0.0f;
  for (int i = 0; i < 15; i++) variance += fabsf(accelHistory[i] - mean);
  currentPacket.restlessnessScore = variance / 15.0f;

  currentPacket.isMoving = (currentPacket.restlessnessScore > MPU6050_MOVE_TH);
}

void PostureEngine::classifyPosture() {
  if (currentPacket.isMoving) {
    currentPacket.posture = POSTURE_MOVEMENT;
    currentPacket.postureName = "MOVEMENT";
    return;
  }

  // Determine static posture from dominant gravity vector orientation
  float az = currentPacket.accelZ;
  float ax = currentPacket.accelX;
  float ay = currentPacket.accelY;

  if (az > 0.55f) {
    currentPacket.posture = POSTURE_BACK;
    currentPacket.postureName = "BACK";
  } else if (az < -0.55f) {
    currentPacket.posture = POSTURE_STOMACH;
    currentPacket.postureName = "STOMACH";
  } else if (ax > 0.50f) {
    currentPacket.posture = POSTURE_LEFT;
    currentPacket.postureName = "LEFT_SIDE";
  } else if (ax < -0.50f) {
    currentPacket.posture = POSTURE_RIGHT;
    currentPacket.postureName = "RIGHT_SIDE";
  } else {
    currentPacket.posture = POSTURE_UNKNOWN;
    currentPacket.postureName = "UNKNOWN";
  }

  currentPacket.positionChanged = (currentPacket.posture != lastPosture && currentPacket.posture != POSTURE_MOVEMENT);
  if (currentPacket.positionChanged) {
    lastPosture = currentPacket.posture;
  }
}

void PostureEngine::evaluateQuality() {
  // If accelerometer magnitude is approximately 1.0g (static earth gravity)
  float dev = fabsf(currentPacket.movementMagnitude - 1.0f);
  if (dev < 0.25f) {
    currentPacket.quality = 0.95f;
  } else if (dev < 0.60f) {
    currentPacket.quality = 0.75f;
  } else {
    currentPacket.quality = 0.40f; // Excessive vibration / freefall
  }
}
