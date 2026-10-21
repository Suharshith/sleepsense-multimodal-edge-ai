/*
 * Wi-Fi Manager & Offline Circular Buffer Implementation
 */

#include "wifi_manager.h"
#include "api_client.h"

WiFiManager wifiMgr;

WiFiManager::WiFiManager() 
  : connected(false), lastRetryTime(0), lastRssi(0), head(0), tail(0), bufferCount(0) {
  memset(ringBuffer, 0, sizeof(ringBuffer));
}

void WiFiManager::begin() {
  Serial.println("\n[INFO] Initializing Wi-Fi Station Mode...");
  Serial.printf("[INFO] Target SSID: %s\n", DEFAULT_WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 6000) {
    delay(250);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    lastRssi = WiFi.RSSI();
    Serial.printf("\n[INFO] Wi-Fi Connected! IP: %s | RSSI: %d dBm\n", WiFi.localIP().toString().c_str(), lastRssi);
  } else {
    connected = false;
    Serial.println("\n[WARN] Wi-Fi Connection Timeout. Starting in Offline Mode with local event buffer.");
  }
}

void WiFiManager::keepConnected() {
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    connected = false;
    if (now - lastRetryTime >= WIFI_RECONNECT_MS) {
      lastRetryTime = now;
      Serial.println("[WARN] Wi-Fi Disconnected. Reconnecting...");
      WiFi.disconnect();
      WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);
    }
  } else {
    if (!connected) {
      connected = true;
      Serial.printf("[INFO] Wi-Fi Reconnected! IP: %s\n", WiFi.localIP().toString().c_str());
      flushBufferedPayloads();
    }
    lastRssi = WiFi.RSSI();
  }
}

int8_t WiFiManager::getRssi() {
  if (WiFi.status() == WL_CONNECTED) {
    lastRssi = WiFi.RSSI();
  }
  return lastRssi;
}

void WiFiManager::pushToBuffer(const UnifiedPayload& payload) {
  ringBuffer[head] = payload;
  head = (head + 1) % OFFLINE_BUFFER_CAPACITY;

  if (bufferCount < OFFLINE_BUFFER_CAPACITY) {
    bufferCount++;
  } else {
    // Buffer full: advance tail to drop oldest frame
    tail = (tail + 1) % OFFLINE_BUFFER_CAPACITY;
    Serial.println("[WARN] Offline buffer full (30 frames). Overwriting oldest record.");
  }
}

bool WiFiManager::popFromBuffer(UnifiedPayload& payload) {
  if (bufferCount == 0) return false;

  payload = ringBuffer[tail];
  tail = (tail + 1) % OFFLINE_BUFFER_CAPACITY;
  bufferCount--;
  return true;
}

void WiFiManager::flushBufferedPayloads() {
  if (bufferCount == 0) return;

  Serial.printf("[INFO] Uploading %d buffered offline packets to Flask backend...\n", bufferCount);
  while (bufferCount > 0 && WiFi.status() == WL_CONNECTED) {
    UnifiedPayload p;
    if (popFromBuffer(p)) {
      apiClient.sendUnifiedPayload(p);
      delay(30); // Yield to network stack
    }
  }
  Serial.println("[INFO] Offline buffer sync complete.");
}
