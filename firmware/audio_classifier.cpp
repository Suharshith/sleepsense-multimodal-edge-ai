/*
 * TinyML Audio Classifier Engine Implementation
 */

#include "audio_classifier.h"
#include <math.h>

AudioClassifier::AudioClassifier() : loaded(false) {
  memset(&lastResult, 0, sizeof(InferenceResult));
  lastResult.className = "Normal Breathing";
}

bool AudioClassifier::begin() {
  Serial.println("[TinyML] Initializing TensorFlow Lite Micro / C-Array Classifier...");
  Serial.printf("[TinyML] Model Version: %s | Features: %d | Classes: %d\n", 
                TINYML_MODEL_VERSION, TINYML_INPUT_FEATURES, TINYML_NUM_CLASSES);
  loaded = true;
  return true;
}

InferenceResult AudioClassifier::predict(const AudioFeatures& features) {
  if (!loaded) return lastResult;

  uint32_t startTime = micros();

  float hidden[8] = {0.0f};

  // Layer 1 Forward Pass: Input (13) -> Hidden (8) with ReLU Activation
  for (int j = 0; j < 8; j++) {
    float sum = pgm_read_float(&TINYML_B1[j]);
    for (int i = 0; i < 13; i++) {
      float w = pgm_read_float(&TINYML_W1[i][j]);
      sum += features.bandEnergies[i] * w;
    }
    // ReLU Activation
    hidden[j] = (sum > 0.0f) ? sum : 0.0f;
  }

  // Layer 2 Forward Pass: Hidden (8) -> Output (3)
  float logits[3] = {0.0f};
  for (int k = 0; k < 3; k++) {
    float sum = pgm_read_float(&TINYML_B2[k]);
    for (int j = 0; j < 8; j++) {
      float w = pgm_read_float(&TINYML_W2[j][k]);
      sum += hidden[j] * w;
    }
    logits[k] = sum;
  }

  // Apply SPL volume heuristic adjustment to output logits
  if (features.dbSpl > ABNORMAL_AUDIO_DB_TH) {
    logits[AUDIO_CLASS_ABNORMAL] += 1.8f;
  } else if (features.dbSpl > SNORE_DB_THRESHOLD) {
    logits[AUDIO_CLASS_SNORING] += 1.5f;
  } else {
    logits[AUDIO_CLASS_NORMAL] += 1.2f;
  }

  // Softmax Probabilities
  softmax(logits, 3);
  for (int k = 0; k < 3; k++) {
    lastResult.probabilities[k] = logits[k];
  }

  // Find predicted class with max probability
  int maxIdx = 0;
  float maxProb = logits[0];
  for (int k = 1; k < 3; k++) {
    if (logits[k] > maxProb) {
      maxProb = logits[k];
      maxIdx = k;
    }
  }

  lastResult.predictedClass = (AudioClass)maxIdx;
  lastResult.confidence = maxProb;
  lastResult.inferenceTimeMicros = micros() - startTime;
  lastResult.memoryAllocatedBytes = sizeof(hidden) + sizeof(logits) + sizeof(InferenceResult);

  switch (lastResult.predictedClass) {
    case AUDIO_CLASS_SNORING:
      lastResult.className = "Snoring Detected";
      break;
    case AUDIO_CLASS_ABNORMAL:
      lastResult.className = "Abnormal Breathing / Apnea";
      break;
    case AUDIO_CLASS_NORMAL:
    default:
      lastResult.className = "Normal Breathing";
      break;
  }

  return lastResult;
}

void AudioClassifier::softmax(float* vec, int size) {
  float maxVal = vec[0];
  for (int i = 1; i < size; i++) {
    if (vec[i] > maxVal) maxVal = vec[i];
  }
  float sumExp = 0.0f;
  for (int i = 0; i < size; i++) {
    vec[i] = expf(vec[i] - maxVal);
    sumExp += vec[i];
  }
  for (int i = 0; i < size; i++) {
    vec[i] /= sumExp;
  }
}
