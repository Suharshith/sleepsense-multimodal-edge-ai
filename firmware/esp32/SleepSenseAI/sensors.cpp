/*
 * Sensors Manager Implementation
 */

#include "sensors.h"

SensorsManager sensors;

SensorsManager::SensorsManager() : i2cInitialized(false) {
  memset(&health, 0, sizeof(SystemHealth));
}

bool SensorsManager::initI2CBus() {
  if (i2cInitialized) return true;

  Serial.println("[INFO] Initializing shared I2C bus on GPIO21 (SDA) & GPIO22 (SCL)...");
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_CLOCK_SPEED);
  delay(100);

  // Scan I2C bus
  health.max30102Ok = scanI2CDevice(MAX30102_I2C_ADDR);
  health.mpu6050Ok  = scanI2CDevice(MPU6050_I2C_ADDR);

  Serial.printf("[INFO] I2C Scan: MAX30102 (0x57)=%s | MPU6050 (0x68)=%s\n",
                health.max30102Ok ? "FOUND" : "NOT FOUND",
                health.mpu6050Ok  ? "FOUND" : "NOT FOUND");

  i2cInitialized = true;
  return (health.max30102Ok || health.mpu6050Ok);
}

bool SensorsManager::scanI2CDevice(uint8_t address) {
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  return (error == 0);
}

void SensorsManager::setSensorStatus(const char* sensorName, bool status) {
  if (strcmp(sensorName, "INMP441") == 0) health.inmp441Ok = status;
  else if (strcmp(sensorName, "MAX30102") == 0) health.max30102Ok = status;
  else if (strcmp(sensorName, "MPU6050") == 0) health.mpu6050Ok = status;
  else if (strcmp(sensorName, "TinyML") == 0) health.tinymlOk = status;
  else if (strcmp(sensorName, "WiFi") == 0) health.wifiOk = status;
}

void SensorsManager::printSystemStatus() {
  health.uptimeSeconds = millis() / 1000;
  health.freeHeapBytes = ESP.getFreeHeap();

  Serial.println("\n=================== SYSTEM HEALTH STATUS ===================");
  Serial.printf("Device ID    : %s\n", DEFAULT_DEVICE_ID);
  Serial.printf("Uptime       : %u seconds | Free Heap: %u bytes\n", health.uptimeSeconds, health.freeHeapBytes);
  Serial.printf("Active Sens  : INMP441=%s | MAX30102=%s | MPU6050=%s\n",
                health.inmp441Ok ? "ONLINE" : "OFFLINE",
                health.max30102Ok ? "ONLINE" : "OFFLINE",
                health.mpu6050Ok ? "ONLINE" : "OFFLINE");
  Serial.printf("Edge Engines : TinyML Audio=%s | Wi-Fi Client=%s\n",
                health.tinymlOk ? "READY" : "DISABLED",
                health.wifiOk ? "CONNECTED" : "OFFLINE_BUFFERING");
  Serial.println("===========================================================\n");
}
