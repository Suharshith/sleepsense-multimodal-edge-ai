/*
 * TinyML Audio Classifier Engine Header
 * Embedded INT8 Quantized Neural Network for ESP32 DevKit V1
 */

#ifndef TINYML_AUDIO_H
#define TINYML_AUDIO_H

#include <Arduino.h>
#include "config.h"

struct AudioInferenceResult {
  AudioClass predictedClass;
  const char* className;
  float confidence;
  float probabilities[3];
  uint32_t inferenceTimeMicros;
  uint16_t memoryAllocatedBytes;
};

class TinyMLAudioClassifier {
private:
  bool initialized;
  AudioInferenceResult lastResult;

  void softmax(float* vec, int size);

public:
  TinyMLAudioClassifier();
  bool begin();
  AudioInferenceResult predict(const float* bandEnergies, float estimatedDb);
  AudioInferenceResult getLastResult() const { return lastResult; }
};

extern TinyMLAudioClassifier tinymlAudio;

#endif // TINYML_AUDIO_H
