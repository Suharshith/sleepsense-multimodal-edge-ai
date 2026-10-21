/*
 * TinyML Audio Classifier Engine Header
 */

#ifndef AUDIO_CLASSIFIER_H
#define AUDIO_CLASSIFIER_H

#include <Arduino.h>
#include "tinyml_model.h"
#include "inmp441_driver.h"

struct InferenceResult {
  AudioClass predictedClass;
  const char* className;
  float confidence; // 0.0 to 1.0
  float probabilities[TINYML_NUM_CLASSES];
  uint32_t inferenceTimeMicros; // Microseconds spent on local inference
  uint32_t memoryAllocatedBytes;
};

class AudioClassifier {
private:
  InferenceResult lastResult;
  bool loaded;

  void softmax(float* vec, int size);

public:
  AudioClassifier();
  bool begin();
  InferenceResult predict(const AudioFeatures& features);
  InferenceResult getLastResult() const { return lastResult; }
  bool isLoaded() const { return loaded; }
};

#endif // AUDIO_CLASSIFIER_H
