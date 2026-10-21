/*
 * Master Firmware Sketch - Adaptive Multimodal Edge AI Sleep Apnea Monitoring System V2
 * 
 * Hardware Platform: ESP32-C3 Dev Module
 * Hardware Sensors:
 * - INMP441 I2S MEMS Microphone (SCK=3, WS=2, SD=4)
 * - MAX30102 Pulse Oximeter & HR (I2C: SDA=8, SCL=9)
 * - MPU6050 6-Axis Motion/IMU (I2C: SDA=8, SCL=9)
 */

#include <Arduino.h>
#include "config.h"
#include "inmp441_driver.h"
#include "max30102_driver.h"
#include "mpu6050_driver.h"
#include "tinyml_model.h"
#include "audio_classifier.h"
#include "multimodal_fusion.h"
#include "wifi_offline_manager.h"

// System Instances
INMP441Driver         mic;
MAX30102Driver        oximeter;
MPU6050Driver         imu;
AudioClassifier       tinyml;
MultimodalFusionEngine riskEngine;
WiFiOfflineManager    network;

unsigned long lastTransmissionTime = 0;
unsigned long lastSerialLogTime    = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=======================================================");
  Serial.println("  Adaptive Multimodal Edge AI Sleep Apnea System V2    ");
  Serial.println("=======================================================");

  // Initialize Network Connection & Sync
  network.begin();

  // Initialize Sensors & Edge AI Engine
  bool micOk   = mic.begin();
  bool ppgOk   = oximeter.begin();
  bool imuOk   = imu.begin();
  bool modelOk = tinyml.begin();

  Serial.printf("\n[System Check] Sensors: INMP441=%s | MAX30102=%s | MPU6050=%s | TinyML=%s\n",
                micOk ? "OK" : "FAIL", ppgOk ? "OK" : "FAIL", imuOk ? "OK" : "FAIL", modelOk ? "OK" : "READY");
  Serial.println("System initialized. Starting multimodal real-time monitoring loop...\n");
}

void loop() {
  // Maintain Wi-Fi and process offline sync buffer
  network.keepConnected();

  // 1. Audio Acquisition & TinyML Inference
  bool audioAvailable = mic.readAudioWindow();
  AudioFeatures audioFeat = mic.getFeatures();
  InferenceResult audioPred = tinyml.predict(audioFeat);

  // 2. Physiological Data Update (SpO2 & Heart Rate)
  PhysiologicalReadings physioData = oximeter.update();

  // 3. Motion & Posture Analytics Update
  MotionReadings motionData = imu.update();

  // 4. Timestamp-Synchronized Multimodal Fusion & Risk Engine
  RiskAssessment risk = riskEngine.evaluate(audioFeat, audioPred, physioData, motionData);

  unsigned long currentMillis = millis();

  // 5. Transmit Payload to Backend / Buffer Offline (Every 2 seconds)
  if (currentMillis - lastTransmissionTime >= 2000) {
    lastTransmissionTime = currentMillis;

    OutboundPayload payload;
    payload.timestamp   = risk.timestamp;
    payload.dbSpl       = audioFeat.dbSpl;
    payload.audioClass  = audioPred.className;
    payload.confidence  = audioPred.confidence;
    payload.spo2        = physioData.spo2;
    payload.heartRate   = physioData.heartRate;
    payload.movement    = motionData.movementMagnitude;
    payload.posture     = motionData.postureName;
    payload.riskScore   = risk.score;
    payload.riskLevel   = risk.levelName;

    network.sendPayload(payload);
  }

  // 6. Output Clean Multimodal Diagnostic Dashboard to Serial Monitor (Every 1 second)
  if (currentMillis - lastSerialLogTime >= 1000) {
    lastSerialLogTime = currentMillis;

    Serial.println("\n----------------- EDGE AI DIAGNOSTICS -----------------");
    Serial.printf("AUDIO [INMP441] : Level: %.1f dB SPL | RMS: %.0f | Class: %s (%.0f%% conf) | Latency: %ud us\n",
                  audioFeat.dbSpl, audioFeat.rms, audioPred.className, audioPred.confidence * 100.0f, audioPred.inferenceTimeMicros);
    
    if (physioData.fingerDetected && physioData.isValid) {
      Serial.printf("PHYSIO [MAX30102]: SpO2: %.1f%% %s | Heart Rate: %.1f BPM | Quality: %.0f%%\n",
                    physioData.spo2, physioData.oxygenDesaturation ? "[DESAT ALERT]" : "[NORMAL]",
                    physioData.heartRate, physioData.signalQuality * 100.0f);
    } else {
      Serial.println("PHYSIO [MAX30102]: [No Finger Detected / Invalid Reading]");
    }

    Serial.printf("MOTION [MPU6050] : Mag: %.2fg | Restlessness: %.3f | Posture: %s %s\n",
                  motionData.movementMagnitude, motionData.restlessnessScore, motionData.postureName,
                  motionData.positionChanged ? "[POSITION SHIFT]" : "");

    Serial.printf("RISK ASSESSMENT  : Index: %.1f / 100 | Risk Level: %s\n", risk.score, risk.levelName);
    Serial.printf("EXPLAINABILITY   : Audio: %.0f%% | SpO2: %.0f%% | HR: %.0f%% | Motion: %.0f%%\n",
                  risk.audioRiskContrib, risk.spo2RiskContrib, risk.hrRiskContrib, risk.motionRiskContrib);
    Serial.printf("NETWORK          : Wi-Fi: %s | Buffered Offline Events: %d\n",
                  network.isConnected() ? "CONNECTED" : "OFFLINE", network.getBufferedCount());
    Serial.println("-------------------------------------------------------");
  }

  yield();
}
