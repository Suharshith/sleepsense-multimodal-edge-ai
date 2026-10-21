/*
 * SleepSense AI - Central Configuration Header
 * Target Board: ESP32 DevKit V1 (ESP-WROOM-32)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// SYSTEM METADATA & VERSION
// ==========================================
#define SYSTEM_NAME           "SleepSense AI"
#define FIRMWARE_VERSION      "2.0.0-PROD"
#define DEFAULT_DEVICE_ID     "ESP32-DEVKITV1-SLEEP-01"

// ==========================================
// HARDWARE PIN CONFIGURATIONS (ESP32 DevKit V1)
// ==========================================

// INMP441 I2S Digital MEMS Microphone (16 kHz Mono)
#define I2S_SCK_PIN           14  // Serial Clock (BCLK) -> Pin D14
#define I2S_WS_PIN            15  // Word Select (LRCLK) -> Pin D15
#define I2S_SD_PIN            32  // Serial Data (DOUT)  -> Pin D32
#define I2S_PORT_NUM          I2S_NUM_0

// Shared I2C Bus Pins (MAX30102 PPG & MPU6050 IMU)
#define I2C_SDA_PIN           21  // Shared I2C Data (SDA)  -> Pin D21
#define I2C_SCL_PIN           22  // Shared I2C Clock (SCL) -> Pin D22
#define I2C_CLOCK_SPEED       400000 // 400 kHz Fast Mode

// I2C Device Addresses
#define MAX30102_I2C_ADDR     0x57
#define MPU6050_I2C_ADDR      0x68

// Status LED (Built-in)
#define STATUS_LED_PIN        2   // On-board LED

// ==========================================
// WI-FI & FLASK BACKEND CONFIGURATION
// ==========================================
#define DEFAULT_WIFI_SSID     "Hello"
#define DEFAULT_WIFI_PASS     "123456789"
#define DEFAULT_SERVER_URL    "http://192.168.0.103:5000/api/v1/sensor-data"
#define HTTP_TIMEOUT_MS       1500
#define WIFI_RECONNECT_MS     10000

// ==========================================
// AUDIO ACQUISITION & TINYML CONFIG
// ==========================================
#define AUDIO_SAMPLE_RATE     16000   // 16 kHz Standard Mono PCM
#define AUDIO_WINDOW_SAMPLES  512     // Frame size per DMA read
#define AUDIO_FEATURE_NUM     13      // Log-Mel / Sub-band spectral features
#define SNORE_EST_DB_TH       55.0f   // Estimated relative acoustic threshold
#define ABNORMAL_EST_DB_TH    65.0f   // Estimated relative abnormal sound threshold

// Audio Class Definitions
enum AudioClass {
  AUDIO_CLASS_NORMAL   = 0,
  AUDIO_CLASS_SNORING  = 1,
  AUDIO_CLASS_ABNORMAL = 2
};

// ==========================================
// PHYSIOLOGICAL SENSOR CONFIG (MAX30102)
// ==========================================
#define MAX30102_FINGER_THRESHOLD  50000L  // Minimum IR reading for finger presence
#define SPO2_VALID_MIN             70.0f   // Minimum valid SpO2 %
#define SPO2_VALID_MAX             100.0f  // Maximum valid SpO2 %
#define SPO2_DESAT_THRESHOLD       92.0f   // Oxygen desaturation threshold %
#define HEART_RATE_MIN             40.0f   // Minimum physiological BPM
#define HEART_RATE_MAX             180.0f  // Maximum physiological BPM

// ==========================================
// MOTION SENSOR CONFIG (MPU6050)
// ==========================================
#define MPU6050_ACCEL_SCALE        16384.0f // LSB/g for +/-2g scale
#define MPU6050_GYRO_SCALE         131.0f   // LSB/(deg/s) for +/-250 deg/s
#define MPU6050_RESTLESS_TH        0.25f    // Variance threshold for restless movement
#define MPU6050_MOVE_TH            0.15f    // Movement threshold (g)

// Sleep Posture Definitions
enum SleepPosture {
  POSTURE_BACK     = 0,  // Supine (Sleeping on back)
  POSTURE_LEFT     = 1,  // Left Lateral side
  POSTURE_RIGHT    = 2,  // Right Lateral side
  POSTURE_STOMACH  = 3,  // Prone (Sleeping on stomach)
  POSTURE_MOVEMENT = 4,  // Active motion / tossing
  POSTURE_UNKNOWN  = 5   // Uncalibrated / transient
};

// ==========================================
// SENSOR TRANSMISSION & BUFFERING TIMING
// ==========================================
#define SENSOR_POLL_INTERVAL_MS    100   // 10 Hz Sensor loop
#define TELEMETRY_TX_INTERVAL_MS   2000  // Transmit unified packet every 2 seconds
#define SERIAL_LOG_INTERVAL_MS     1000  // Serial monitor update every 1 second
#define OFFLINE_BUFFER_CAPACITY    30    // Maximum buffered readings during Wi-Fi dropout

#endif // CONFIG_H
