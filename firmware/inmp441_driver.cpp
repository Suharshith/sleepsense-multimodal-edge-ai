/*
 * INMP441 I2S MEMS Microphone Driver Implementation
 */

#include "inmp441_driver.h"
#include <math.h>

INMP441Driver::INMP441Driver() : initialized(false) {
  memset(&currentFeatures, 0, sizeof(AudioFeatures));
  currentFeatures.dbSpl = 30.0f;
}

bool INMP441Driver::begin() {
  Serial.println("[INMP441] Initializing I2S MEMS Microphone...");
  Serial.printf("[INMP441] Pins: SCK=%d, WS=%d, SD=%d\n", I2S_SCK_PIN, I2S_WS_PIN, I2S_SD_PIN);

  // Set pins for ESP32-C3 I2S
  i2s.setPins(I2S_SCK_PIN, I2S_WS_PIN, -1, I2S_SD_PIN);

  if (!i2s.begin(I2S_MODE_STD, AUDIO_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("[INMP441] ERROR: Failed to start I2S driver!");
    initialized = false;
    return false;
  }

  initialized = true;
  Serial.println("[INMP441] Microphone initialized successfully.");
  return true;
}

bool INMP441Driver::readAudioWindow() {
  if (!initialized) return false;

  size_t bytesRead = i2s.readBytes((char*)rawBuffer, sizeof(rawBuffer));
  size_t numSamples = bytesRead / sizeof(int16_t);

  if (numSamples == 0) return false;

  double sumSquare = 0.0;
  int16_t maxVal = 0;

  // Feature accumulation: RMS, Peak, and 13 Sub-band log energies
  float bandSum[AUDIO_FEATURE_NUM] = {0.0f};
  size_t bandSize = numSamples / AUDIO_FEATURE_NUM;
  if (bandSize < 1) bandSize = 1;

  for (size_t i = 0; i < numSamples; i++) {
    int16_t sample = rawBuffer[i];
    double s = (double)sample;
    sumSquare += s * s;

    int16_t absS = abs(sample);
    if (absS > maxVal) maxVal = absS;

    size_t bandIdx = i / bandSize;
    if (bandIdx >= AUDIO_FEATURE_NUM) bandIdx = AUDIO_FEATURE_NUM - 1;
    bandSum[bandIdx] += fabsf((float)sample);
  }

  currentFeatures.rms = (float)sqrt(sumSquare / numSamples);
  currentFeatures.peak = (float)maxVal;

  float rawRms = (currentFeatures.rms < 1.0f) ? 1.0f : currentFeatures.rms;
  currentFeatures.dbSpl = 120.0f + 20.0f * log10f(rawRms / 32767.0f);
  if (currentFeatures.dbSpl < 30.0f) currentFeatures.dbSpl = 30.0f;

  currentFeatures.snoreDetected = (currentFeatures.dbSpl >= SNORE_DB_THRESHOLD);
  currentFeatures.abnormalAudioDetected = (currentFeatures.dbSpl >= ABNORMAL_AUDIO_DB_TH);

  for (size_t b = 0; b < AUDIO_FEATURE_NUM; b++) {
    float avgBand = bandSum[b] / bandSize;
    currentFeatures.bandEnergies[b] = log1f(avgBand + 1.0f);
  }

  return true;
}
