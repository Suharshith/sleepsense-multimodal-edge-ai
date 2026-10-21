/*
 * Wi-Fi Manager & Offline Circular Buffer Header
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

struct UnifiedPayload {
  uint32_t timestamp;
  uint32_t sequenceNumber;
  // Audio
  float audioRms;
  float audioPeak;
  float audioEstimatedDb;
  float audioSnoreProb;
  bool audioSnoreDetected;
  float audioQuality;
  // PPG
  float ppgBpm;
  float ppgSpo2;
  float ppgHrv;
  float ppgQuality;
  // Posture
  const char* postureClass;
  float postureMovement;
  float postureQuality;
};

class WiFiManager {
private:
  bool connected;
  unsigned long lastRetryTime;
  int8_t lastRssi;

  // Offline Circular Buffer
  UnifiedPayload ringBuffer[OFFLINE_BUFFER_CAPACITY];
  uint8_t head;
  uint8_t tail;
  uint8_t bufferCount;

public:
  WiFiManager();
  void begin();
  void keepConnected();
  bool isConnected() const { return (WiFi.status() == WL_CONNECTED); }
  int8_t getRssi();

  // Buffer management
  void pushToBuffer(const UnifiedPayload& payload);
  bool popFromBuffer(UnifiedPayload& payload);
  uint8_t getBufferedCount() const { return bufferCount; }
  void flushBufferedPayloads();
};

extern WiFiManager wifiMgr;

#endif // WIFI_MANAGER_H
