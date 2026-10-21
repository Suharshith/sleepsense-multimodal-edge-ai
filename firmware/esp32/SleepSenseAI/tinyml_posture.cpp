/*
 * TinyML Posture Classifier Implementation
 */

#include "tinyml_posture.h"
#include <math.h>

TinyMLPostureClassifier tinymlPosture;

TinyMLPostureClassifier::TinyMLPostureClassifier() {}

bool TinyMLPostureClassifier::begin() {
  Serial.println("[AI] Initializing TinyML Posture Decision Tree & Orientation Engine...");
  return true;
}

PostureInferenceResult TinyMLPostureClassifier::predict(float ax, float ay, float az, float gx, float gy, float gz, float restless) {
  uint32_t start = micros();
  PostureInferenceResult res;

  if (restless > MPU6050_RESTLESS_TH || fabsf(gx) > 40.0f || fabsf(gy) > 40.0f) {
    res.predictedPosture = POSTURE_MOVEMENT;
    res.postureName = "MOVEMENT";
    res.confidence = 0.92f;
  } else if (az > 0.55f) {
    res.predictedPosture = POSTURE_BACK;
    res.postureName = "BACK";
    res.confidence = 0.95f;
  } else if (az < -0.55f) {
    res.predictedPosture = POSTURE_STOMACH;
    res.postureName = "STOMACH";
    res.confidence = 0.94f;
  } else if (ax > 0.50f) {
    res.predictedPosture = POSTURE_LEFT;
    res.postureName = "LEFT_SIDE";
    res.confidence = 0.96f;
  } else if (ax < -0.50f) {
    res.predictedPosture = POSTURE_RIGHT;
    res.postureName = "RIGHT_SIDE";
    res.confidence = 0.96f;
  } else {
    res.predictedPosture = POSTURE_UNKNOWN;
    res.postureName = "UNKNOWN";
    res.confidence = 0.70f;
  }

  res.latencyMicros = micros() - start;
  return res;
}
