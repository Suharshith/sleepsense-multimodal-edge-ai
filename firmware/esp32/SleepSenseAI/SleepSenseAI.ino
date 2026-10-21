/*
 * SleepSense AI - Master Firmware Sketch
 * Target Board: ESP32 DevKit V1 (ESP-WROOM-32)
 *
 * Hardware Configuration (3 Core Sensing Modalities):
 * - INMP441 I2S Digital MEMS Microphone (SCK:14, WS:15, SD:32)
 * - MAX30102 Pulse Oximeter & Heart Rate (I2C: SDA=21, SCL=22)
 * - MPU6050 6-Axis Motion IMU (I2C: SDA=21, SCL=22)
 *
 * Execution Model: Non-blocking dual-rate scheduling via millis()
 */

#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "audio.h"
#include "ppg.h"
#include "posture.h"
#include "tinyml_audio.h"
#include "tinyml_posture.h"
#include "wifi_manager.h"
#include "api_client.h"

// Runtime Sequence & Timing Counters
static uint32_t packetSeqNum = 0;
static unsigned long lastSensorPollTime = 0;
static unsigned long lastTxTime = 0;
static unsigned long lastSerialLogTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=======================================================");
  Serial.println("   SleepSense AI - Adaptive Multi-Modal Edge AI System ");
  Serial.printf("   Target: %s | Firmware: %s\n", DEFAULT_DEVICE_ID, FIRMWARE_VERSION);
  Serial.println("=======================================================");

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH);

  // 1. Initialize I2C Bus & Scan Peripherals (MAX30102 & MPU6050)
  bool i2cOk = sensors.initI2CBus();

  // 2. Initialize INMP441 I2S Microphone
  bool micOk = audio.initI2S();
  sensors.setSensorStatus("INMP441", micOk);

  // 3. Initialize MAX30102 Pulse Oximeter
  bool ppgOk = ppg.initMAX30102();
  sensors.setSensorStatus("MAX30102", ppgOk);

  // 4. Initialize MPU6050 6-Axis IMU
  bool imuOk = posture.initMPU6050();
  sensors.setSensorStatus("MPU6050", imuOk);

  // 5. Initialize TinyML On-Device Neural Engines
  bool tinymlAudioOk = tinymlAudio.begin();
  bool tinymlPostureOk = tinymlPosture.begin();
  sensors.setSensorStatus("TinyML", tinymlAudioOk && tinymlPostureOk);

  // 6. Initialize Wi-Fi & Offline Ring Buffer
  wifiMgr.begin();
  sensors.setSensorStatus("WiFi", wifiMgr.isConnected());

  // 7. Print Boot Summary
  sensors.printSystemStatus();
  digitalWrite(STATUS_LED_PIN, LOW);
  Serial.println("[INFO] Boot sequence complete. Starting multi-modal acquisition loop...\n");
}

void loop() {
  // Maintain Wi-Fi and sync offline buffer
  wifiMgr.keepConnected();

  unsigned long currentMillis = millis();

  // ----------------------------------------------------
  // Task 1: High-Frequency Sensor Acquisition (10 Hz)
  // ----------------------------------------------------
  if (currentMillis - lastSensorPollTime >= SENSOR_POLL_INTERVAL_MS) {
    lastSensorPollTime = currentMillis;

    // 1. Audio Capture & TinyML Inference
    if (audio.captureAndProcess()) {
      AudioPacket ap = audio.getPacket();
      AudioInferenceResult air = tinymlAudio.predict(ap.bandEnergies, ap.estimatedDb);
      audio.setSnoreProbability(air.probabilities[AUDIO_CLASS_SNORING]);
    }

    // 2. PPG Waveform Update
    ppg.update();

    // 3. Posture & Kinematics Update
    posture.update();
  }

  // ----------------------------------------------------
  // Task 2: Unified Telemetry Transmission (Every 2s)
  // ----------------------------------------------------
  if (currentMillis - lastTxTime >= TELEMETRY_TX_INTERVAL_MS) {
    lastTxTime = currentMillis;
    packetSeqNum++;

    AudioPacket ap = audio.getPacket();
    PpgPacket pp   = ppg.getPacket();
    PosturePacket pos = posture.getPacket();

    UnifiedPayload payload;
    payload.timestamp         = (uint32_t)(millis() / 1000);
    payload.sequenceNumber    = packetSeqNum;
    
    // Audio
    payload.audioRms          = ap.rms;
    payload.audioPeak         = ap.peak;
    payload.audioEstimatedDb  = ap.estimatedDb;
    payload.audioSnoreProb    = ap.snoreProbability;
    payload.audioSnoreDetected= ap.snoreDetected;
    payload.audioQuality      = ap.quality;

    // PPG
    payload.ppgBpm            = pp.heartRate;
    payload.ppgSpo2           = pp.spo2;
    payload.ppgHrv            = pp.hrvRmssd;
    payload.ppgQuality        = pp.quality;

    // Posture
    payload.postureClass      = pos.postureName;
    payload.postureMovement   = pos.movementMagnitude;
    payload.postureQuality    = pos.quality;

    // Transmit to Flask backend or push to offline ring buffer
    bool txOk = apiClient.sendUnifiedPayload(payload);
    digitalWrite(STATUS_LED_PIN, txOk ? HIGH : LOW);
  }

  // ----------------------------------------------------
  // Task 3: Dual Serial Monitoring (JSON Lines & Tagged Logs)
  // ----------------------------------------------------
  if (currentMillis - lastSerialLogTime >= SERIAL_LOG_INTERVAL_MS) {
    lastSerialLogTime = currentMillis;

    AudioPacket ap = audio.getPacket();
    PpgPacket pp   = ppg.getPacket();
    PosturePacket pos = posture.getPacket();
    AudioInferenceResult air = tinymlAudio.getLastResult();

    // 1. Structured Tagged Console Log
    Serial.printf("[INFO] Audio: %4.1f est.dB (RMS:%4.0f) | Snore: %s (%.0f%%) | Q:%.2f\n",
                  ap.estimatedDb, ap.rms, ap.snoreDetected ? "YES" : "NO", air.confidence * 100.0f, ap.quality);

    if (pp.fingerDetected) {
      Serial.printf("[INFO] Physio: SpO2: %4.1f%% %s | BPM: %4.1f | HRV: %4.1f ms | Q:%.2f\n",
                    pp.spo2, pp.oxygenDesaturation ? "[DESAT ALERT]" : "", pp.heartRate, pp.hrvRmssd, pp.quality);
    } else {
      Serial.println("[WARN] Physio: [Finger Not Detected / Disconnected]");
    }

    Serial.printf("[INFO] Posture: %-10s | Mag:%4.2fg | Restless:%5.3f | Q:%.2f\n",
                  pos.postureName, pos.movementMagnitude, pos.restlessnessScore, pos.quality);
    Serial.printf("[AI]   TinyML Audio Class: %s (Latency: %u us)\n",
                  air.className, air.inferenceTimeMicros);
    Serial.printf("[INFO] Network: %s (RSSI: %d dBm) | Buffered: %d pkts\n\n",
                  wifiMgr.isConnected() ? "CONNECTED" : "OFFLINE", wifiMgr.getRssi(), wifiMgr.getBufferedCount());

    // 2. Serial JSON Stream for Live Telemetry Tools
    Serial.print("{\"type\":\"telemetry\",\"seq\":");
    Serial.print(packetSeqNum);
    Serial.print(",\"audio_db\":");
    Serial.print(ap.estimatedDb, 1);
    Serial.print(",\"spo2\":");
    Serial.print(pp.spo2, 1);
    Serial.print(",\"bpm\":");
    Serial.print(pp.heartRate, 1);
    Serial.print(",\"posture\":\"");
    Serial.print(pos.postureName);
    Serial.println("\"}");
  }

  yield();
}
