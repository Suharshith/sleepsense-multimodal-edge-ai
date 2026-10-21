/*
 * INMP441 I2S MEMS Microphone Driver Header
 */

#ifndef INMP441_DRIVER_H
#define INMP441_DRIVER_H

#include <Arduino.h>
#include "ESP_I2S.h"
#include "config.h"

struct AudioFeatures {
  float rms;
  float peak;
  float dbSpl;
  bool snoreDetected;
  bool abnormalAudioDetected;
  float bandEnergies[AUDIO_FEATURE_NUM]; // Feature representation vector for TinyML
};

class INMP441Driver {
private:
  I2SClass i2s;
  int16_t rawBuffer[AUDIO_WINDOW_SAMPLES];
  AudioFeatures currentFeatures;
  bool initialized;

public:
  INMP441Driver();
  bool begin();
  bool readAudioWindow();
  AudioFeatures getFeatures() const { return currentFeatures; }
  const int16_t* getRawBuffer() const { return rawBuffer; }
  bool isInitialized() const { return initialized; }
};

#endif // INMP441_DRIVER_H
