/*
 * Wi-Fi Reconnect & Offline Event Buffering Manager Implementation
 */

#include "wifi_offline_manager.h"
#include <time.h>

WiFiOfflineManager::WiFiOfflineManager() : head(0), tail(0), count(0), lastWiFiRetry(0) {
  memset(buffer, 0, sizeof(buffer));
}

void WiFiOfflineManager::begin() {
  Serial.print("[Wi-Fi] Connecting to SSID: ");
  Serial.println(DEFAULT_WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 8000) {
    delay(400);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[Wi-Fi] Connected successfully!");
    Serial.print("[Wi-Fi] IP Address: ");
    Serial.println(WiFi.localIP());

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  } else {
    Serial.println("\n[Wi-Fi] Timeout. Continuing offline with local storage buffer...");
  }
}

void WiFiOfflineManager::keepConnected() {
  unsigned long now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWiFiRetry >= 10000) {
      lastWiFiRetry = now;
      Serial.println("[Wi-Fi] Reconnecting...");
      WiFi.disconnect();
      WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);
    }
  } else if (count > 0) {
    syncBufferedPayloads();
  }
}

bool WiFiOfflineManager::sendPayload(const OutboundPayload& payload) {
  if (WiFi.status() != WL_CONNECTED) {
    bufferPayload(payload);
    return false;
  }

  WiFiClient client;
  HTTPClient http;

  http.begin(client, DEFAULT_SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(1200);

  String json = "{";
  json += "\"deviceId\":\"" + String(DEFAULT_DEVICE_ID) + "\",";
  json += "\"timestamp\":" + String(payload.timestamp) + ",";
  json += "\"audio\":{\"db\":" + String(payload.dbSpl, 1) + ",\"class\":\"" + String(payload.audioClass) + "\",\"confidence\":" + String(payload.confidence, 2) + "},";
  json += "\"physio\":{\"spo2\":" + String(payload.spo2, 1) + ",\"heartRate\":" + String(payload.heartRate, 1) + "},";
  json += "\"motion\":{\"movement\":" + String(payload.movement, 2) + ",\"posture\":\"" + String(payload.posture) + "\"},";
  json += "\"risk\":{\"score\":" + String(payload.riskScore, 1) + ",\"level\":\"" + String(payload.riskLevel) + "\"}";
  json += "}";

  int code = http.POST(json);
  http.end();

  if (code > 0) {
    return true;
  } else {
    bufferPayload(payload);
    return false;
  }
}

void WiFiOfflineManager::bufferPayload(const OutboundPayload& payload) {
  if (count < OFFLINE_BUFFER_CAPACITY) {
    buffer[head] = payload;
    head = (head + 1) % OFFLINE_BUFFER_CAPACITY;
    count++;
    Serial.printf("[Offline Buffer] Stored event (Total in buffer: %d)\n", count);
  } else {
    // Overwrite oldest entry if buffer full
    buffer[head] = payload;
    head = (head + 1) % OFFLINE_BUFFER_CAPACITY;
    tail = (tail + 1) % OFFLINE_BUFFER_CAPACITY;
    Serial.println("[Offline Buffer] WARNING: Buffer full, overwriting oldest reading!");
  }
}

void WiFiOfflineManager::syncBufferedPayloads() {
  Serial.printf("[Offline Sync] Synchronizing %d buffered events to backend...\n", count);
  while (count > 0 && WiFi.status() == WL_CONNECTED) {
    OutboundPayload p = buffer[tail];
    tail = (tail + 1) % OFFLINE_BUFFER_CAPACITY;
    count--;
    sendPayload(p);
    delay(50);
  }
  Serial.println("[Offline Sync] Synchronization complete.");
}
