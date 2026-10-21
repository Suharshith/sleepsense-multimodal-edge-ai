/*
 * TinyML Posture Classifier Header
 */

#ifndef TINYML_POSTURE_H
#define TINYML_POSTURE_H

#include <Arduino.h>
#include "config.h"

struct PostureInferenceResult {
  SleepPosture predictedPosture;
  const char* postureName;
  float confidence;
  uint32_t latencyMicros;
};

class TinyMLPostureClassifier {
public:
  TinyMLPostureClassifier();
  bool begin();
  PostureInferenceResult predict(float ax, float ay, float az, float gx, float gy, float gz, float restless);
};

extern TinyMLPostureClassifier tinymlPosture;

#endif // TINYML_POSTURE_H
