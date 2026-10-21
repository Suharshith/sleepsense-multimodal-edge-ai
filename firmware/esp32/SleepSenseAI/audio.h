/*
 * Audio Acquisition & Acoustic Feature Extraction Header
 * Sensor: INMP441 I2S MEMS Digital Microphone (16 kHz Mono)
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "config.h"

struct AudioPacket {
  float rms;
  float peak;
  float estimatedDb;
  float bandEnergies[AUDIO_FEATURE_NUM];
  float snoreProbability;
  bool snoreDetected;
  uint32_t snoreDurationMs;
  float quality; // Audio Signal Quality Index [0.0 to 1.0]
  bool isValid;
};

class AudioEngine {
private:
  bool initialized;
  int16_t sampleBuffer[AUDIO_WINDOW_SAMPLES];
  AudioPacket currentPacket;

  // Snore debouncing & temporal hysteresis
  unsigned long snoreStartTime;
  bool inSnoreEvent;
  uint32_t consecutiveSnoreWindows;

  void calculateFeatures(size_t samplesRead);
  void evaluateSignalQuality();

public:
  AudioEngine();
  bool initI2S();
  bool captureAndProcess();
  AudioPacket getPacket() const { return currentPacket; }
  void setSnoreProbability(float prob);
};

extern AudioEngine audio;

#endif // AUDIO_H
