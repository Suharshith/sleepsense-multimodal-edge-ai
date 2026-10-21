/*
 * Central Configuration Header - Adaptive Multimodal Edge AI Sleep Apnea Monitoring System V2
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// HARDWARE PIN CONFIGURATIONS
// ==========================================

// Board Selection: ESP32 DevKit V1 (ESP-WROOM-32)
#if defined(CONFIG_IDF_TARGET_ESP32C3)
  // ESP32-C3 Pinout
  #define I2S_SCK_PIN     3   // Serial Clock (BCLK)
  #define I2S_WS_PIN      2   // Word Select (LRCLK)
  #define I2S_SD_PIN      4   // Serial Data (DOUT)
  #define I2C_SDA_PIN     8   // I2C Data (SDA)
  #define I2C_SCL_PIN     9   // I2C Clock (SCL)
#else
  // Standard ESP32 DevKit V1 (ESP-WROOM-32) Pinout
  #define I2S_SCK_PIN     14  // Serial Clock (BCLK) -> Pin D14
  #define I2S_WS_PIN      15  // Word Select (LRCLK) -> Pin D15
  #define I2S_SD_PIN      32  // Serial Data (DOUT)  -> Pin D32
  #define I2C_SDA_PIN     21  // Shared I2C Data (SDA) -> Pin D21
  #define I2C_SCL_PIN     22  // Shared I2C Clock (SCL) -> Pin D22
#endif

#define I2C_CLOCK_SPEED 400000 // 400 kHz Fast Mode

// ==========================================
// WI-FI & BACKEND API CONFIGURATION
// ==========================================
#define DEFAULT_WIFI_SSID     "Hello"
#define DEFAULT_WIFI_PASS     "123456789"
#define DEFAULT_SERVER_URL    "http://192.168.0.103:5000/api/readings"
#define DEFAULT_DEVICE_ID     "ESP32-DEVKITV1-SLEEP-01"

// ==========================================
// AUDIO ACQUISITION & TINYML CONFIG
// ==========================================
#define AUDIO_SAMPLE_RATE     16000   // 16 kHz Audio Sampling
#define AUDIO_WINDOW_SAMPLES  512     // Audio block size per read
#define AUDIO_FEATURE_NUM     13      // Band energy / MFCC features
#define SNORE_DB_THRESHOLD    55.0f   // Snoring sound level threshold (dB SPL)
#define ABNORMAL_AUDIO_DB_TH  65.0f   // Abnormal breathing audio threshold (dB SPL)

// ==========================================
// PHYSIOLOGICAL SENSOR CONFIG (MAX30102)
// ==========================================
#define MAX30102_FINGER_THRESHOLD  50000L  // Minimum IR value for finger presence
#define SPO2_VALID_MIN             70.0f   // Minimum plausible SpO2 %
#define SPO2_VALID_MAX             100.0f  // Maximum plausible SpO2 %
#define SPO2_DESAT_THRESHOLD       92.0f   // Oxygen desaturation threshold %
#define HEART_RATE_MIN             40.0f   // Minimum valid BPM
#define HEART_RATE_MAX             180.0f  // Maximum valid BPM

// ==========================================
// MOTION SENSOR CONFIG (MPU6050)
// ==========================================
#define MPU6050_ACCEL_SCALE        16384.0f // LSB/g for +-2g scale
#define MPU6050_RESTLESS_THRESHOLD 0.35f    // Acceleration variation for restlessness
#define MPU6050_MOVE_THRESHOLD     0.15f    // Movement threshold (g)

// Posture Definitions
enum BodyPosture {
  POSTURE_SUPINE = 0,  // Sleeping on back
  POSTURE_PRONE  = 1,  // Sleeping on stomach
  POSTURE_LEFT   = 2,  // Sleeping on left side
  POSTURE_RIGHT  = 3,  // Sleeping on right side
  POSTURE_UNKNOWN= 4   // Motion / uncalibrated
};

// ==========================================
// MULTIMODAL RISK ASSESSMENT WEIGHTS
// ==========================================
#define WEIGHT_AUDIO_BURDEN   0.35f  // Snoring & abnormal breathing burden
#define WEIGHT_SPO2_DESAT     0.35f  // Oxygen desaturation burden
#define WEIGHT_HR_VARIATION   0.15f  // Heart rate instability burden
#define WEIGHT_MOTION_BURDEN  0.15f  // Restlessness & position shifts

// Risk Levels
enum RiskLevel {
  RISK_LEVEL_NORMAL   = 0,
  RISK_LEVEL_LOW      = 1,
  RISK_LEVEL_MODERATE = 2,
  RISK_LEVEL_ELEVATED = 3
};

// ==========================================
// OFFLINE BUFFERING CONFIG
// ==========================================
#define OFFLINE_BUFFER_CAPACITY 30   // Maximum buffered readings during Wi-Fi drop

#endif // CONFIG_H
