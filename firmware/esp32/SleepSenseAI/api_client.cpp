/*
 * API Client Implementation
 */

#include "api_client.h"

ApiClient apiClient;

ApiClient::ApiClient() {}

String ApiClient::serializeJson(const UnifiedPayload& p) {
  String json = "{";
  json += "\"device_id\":\"" + String(DEFAULT_DEVICE_ID) + "\",";
  json += "\"timestamp\":" + String(p.timestamp) + ",";
  json += "\"sequence_number\":" + String(p.sequenceNumber) + ",";
  
  // Audio block
  json += "\"audio\":{";
  json += "\"rms\":" + String(p.audioRms, 1) + ",";
  json += "\"peak\":" + String(p.audioPeak, 1) + ",";
  json += "\"estimated_db\":" + String(p.audioEstimatedDb, 1) + ",";
  json += "\"snore_probability\":" + String(p.audioSnoreProb, 2) + ",";
  json += "\"snore_detected\":" + String(p.audioSnoreDetected ? "true" : "false") + ",";
  json += "\"quality\":" + String(p.audioQuality, 2);
  json += "},";

  // PPG block
  json += "\"ppg\":{";
  json += "\"bpm\":" + String(p.ppgBpm, 1) + ",";
  json += "\"spo2\":" + String(p.ppgSpo2, 1) + ",";
  json += "\"hrv\":" + String(p.ppgHrv, 1) + ",";
  json += "\"quality\":" + String(p.ppgQuality, 2);
  json += "},";

  // Posture block
  json += "\"posture\":{";
  json += "\"class\":\"" + String(p.postureClass) + "\",";
  json += "\"movement\":" + String(p.postureMovement, 2) + ",";
  json += "\"quality\":" + String(p.postureQuality, 2);
  json += "}";

  json += "}";
  return json;
}

bool ApiClient::sendUnifiedPayload(const UnifiedPayload& payload) {
  if (WiFi.status() != WL_CONNECTED) {
    wifiMgr.pushToBuffer(payload);
    return false;
  }

  WiFiClient client;
  HTTPClient http;

  http.begin(client, DEFAULT_SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(HTTP_TIMEOUT_MS);

  String payloadJson = serializeJson(payload);
  int httpCode = http.POST(payloadJson);
  http.end();

  if (httpCode >= 200 && httpCode < 300) {
    return true;
  } else {
    wifiMgr.pushToBuffer(payload);
    return false;
  }
}
