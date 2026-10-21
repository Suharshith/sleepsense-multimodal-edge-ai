/*
 * API Client Header - Unified IoT REST Telemetry Client
 */

#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include "config.h"
#include "wifi_manager.h"

class ApiClient {
public:
  ApiClient();
  bool sendUnifiedPayload(const UnifiedPayload& payload);
  String serializeJson(const UnifiedPayload& p);
};

extern ApiClient apiClient;

#endif // API_CLIENT_H
