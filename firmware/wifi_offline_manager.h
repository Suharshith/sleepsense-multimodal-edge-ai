/*
 * Wi-Fi Reconnect & Offline Event Buffering Manager Header
 */

#ifndef WIFI_OFFLINE_MANAGER_H
#define WIFI_OFFLINE_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"

struct OutboundPayload {
  unsigned long timestamp;
  float dbSpl;
  const char* audioClass;
  float confidence;
  float spo2;
  float heartRate;
  float movement;
  const char* posture;
  float riskScore;
  const char* riskLevel;
};

class WiFiOfflineManager {
private:
  OutboundPayload buffer[OFFLINE_BUFFER_CAPACITY];
  uint8_t head;
  uint8_t tail;
  uint8_t count;
  unsigned long lastWiFiRetry;

public:
  WiFiOfflineManager();
  void begin();
  void keepConnected();
  bool sendPayload(const OutboundPayload& payload);
  void bufferPayload(const OutboundPayload& payload);
  void syncBufferedPayloads();
  bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
  uint8_t getBufferedCount() const { return count; }
};

#endif // WIFI_OFFLINE_MANAGER_H
