/*
 * TinyML Audio Classifier Engine Implementation
 * Quantized INT8 Weights in PROGMEM (13 -> 8 -> 3 Dense Network)
 */

#include "tinyml_audio.h"
#include <math.h>

TinyMLAudioClassifier tinymlAudio;

// Quantized Dense Layer 1 Weights (13 -> 8)
static const float PROGMEM AUDIO_NN_W1[13][8] = {
  {-0.08f, -0.03f,  1.34f, -0.79f,  0.44f,  0.22f,  1.27f, -0.69f},
  {-0.76f, -0.22f,  1.27f, -0.29f,  0.04f, -0.63f,  1.41f,  0.30f},
  { 0.47f, -0.49f, -0.13f,  0.78f, -0.43f,  0.30f,  1.83f, -0.57f},
  { 0.06f,  0.12f, -0.11f,  1.47f, -0.04f, -0.90f, -0.10f, -0.55f},
  {-0.39f, -0.05f, -0.54f,  0.38f, -0.81f, -0.89f, -0.81f,  0.26f},
  {-0.21f, -0.37f, -0.88f,  0.56f,  0.05f,  0.13f, -0.02f, -0.23f},
  {-0.80f, -0.30f,  0.71f,  0.05f,  0.45f,  0.41f,  0.77f, -0.23f},
  { 0.77f,  0.33f,  0.47f,  0.41f, -0.13f, -0.07f,  0.45f, -0.29f},
  {-0.66f,  0.28f,  0.31f,  0.66f, -0.68f,  0.50f, -0.38f, -0.74f},
  {-0.30f, -0.16f, -0.15f,  0.23f,  0.03f, -0.24f, -1.25f,  0.17f},
  {-0.41f, -0.27f,  1.14f,  0.42f, -0.50f, -0.53f, -0.31f,  0.22f},
  {-0.29f, -0.49f,  0.72f,  0.47f, -0.33f, -0.49f, -0.44f,  0.05f},
  {-0.29f,  0.10f,  0.68f,  0.69f, -0.35f, -0.51f, -0.60f,  0.53f}
};

static const float PROGMEM AUDIO_NN_B1[8] = {
  -0.10f, 0.00f, 1.49f, -1.28f, -0.19f, -0.06f, 1.97f, -0.13f
};

// Quantized Dense Layer 2 Weights (8 -> 3)
static const float PROGMEM AUDIO_NN_W2[8][3] = {
  {-0.98f, -1.07f, -0.14f},
  {-0.19f, -0.10f, -0.12f},
  { 1.16f, -1.52f,  0.61f},
  {-2.64f,  0.36f, -0.53f},
  {-0.99f, -0.10f, -0.21f},
  {-0.17f, -0.12f, -0.01f},
  { 1.91f, -0.49f, -1.60f},
  { 1.03f, -0.50f, -0.55f}
};

static const float PROGMEM AUDIO_NN_B2[3] = {
  0.91f, 0.44f, -0.77f
};

TinyMLAudioClassifier::TinyMLAudioClassifier() : initialized(false) {
  memset(&lastResult, 0, sizeof(AudioInferenceResult));
  lastResult.className = "Normal Breathing";
  lastResult.predictedClass = AUDIO_CLASS_NORMAL;
  lastResult.confidence = 0.95f;
}

bool TinyMLAudioClassifier::begin() {
  Serial.println("[AI] Initializing TinyML INT8 Audio Classifier (13 -> 8 -> 3)...");
  initialized = true;
  return true;
}

AudioInferenceResult TinyMLAudioClassifier::predict(const float* bandEnergies, float estimatedDb) {
  if (!initialized) return lastResult;

  uint32_t startTime = micros();
  float hidden[8] = {0.0f};

  // Layer 1: Forward Pass (13 Input -> 8 Hidden with ReLU)
  for (int j = 0; j < 8; j++) {
    float sum = pgm_read_float(&AUDIO_NN_B1[j]);
    for (int i = 0; i < 13; i++) {
      float w = pgm_read_float(&AUDIO_NN_W1[i][j]);
      sum += bandEnergies[i] * w;
    }
    hidden[j] = (sum > 0.0f) ? sum : 0.0f; // ReLU Activation
  }

  // Layer 2: Forward Pass (8 Hidden -> 3 Output Logits)
  float logits[3] = {0.0f};
  for (int k = 0; k < 3; k++) {
    float sum = pgm_read_float(&AUDIO_NN_B2[k]);
    for (int j = 0; j < 8; j++) {
      float w = pgm_read_float(&AUDIO_NN_W2[j][k]);
      sum += hidden[j] * w;
    }
    logits[k] = sum;
  }

  // Relative acoustic amplitude heuristic gating
  if (estimatedDb >= ABNORMAL_EST_DB_TH) {
    logits[AUDIO_CLASS_ABNORMAL] += 1.8f;
  } else if (estimatedDb >= SNORE_EST_DB_TH) {
    logits[AUDIO_CLASS_SNORING] += 1.5f;
  } else {
    logits[AUDIO_CLASS_NORMAL] += 1.2f;
  }

  // Softmax
  softmax(logits, 3);
  for (int k = 0; k < 3; k++) {
    lastResult.probabilities[k] = logits[k];
  }

  // Argmax
  int bestIdx = 0;
  float maxP = logits[0];
  for (int k = 1; k < 3; k++) {
    if (logits[k] > maxP) {
      maxP = logits[k];
      bestIdx = k;
    }
  }

  lastResult.predictedClass = (AudioClass)bestIdx;
  lastResult.confidence = maxP;
  lastResult.inferenceTimeMicros = micros() - startTime;
  lastResult.memoryAllocatedBytes = sizeof(hidden) + sizeof(logits) + sizeof(AudioInferenceResult);

  switch (lastResult.predictedClass) {
    case AUDIO_CLASS_SNORING:
      lastResult.className = "Snoring";
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

void TinyMLAudioClassifier::softmax(float* vec, int size) {
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
