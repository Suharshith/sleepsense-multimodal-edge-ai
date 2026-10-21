/*
 * TinyML Model Header - Pre-trained Sleep Audio Classifier Array & Quantization Parameters
 * Architecture: Embedded Light-Weight Neural Network (INT8 Quantized C-Array)
 */

#ifndef TINYML_MODEL_H
#define TINYML_MODEL_H

#include <Arduino.h>

// Model Metadata Constants
#define TINYML_MODEL_VERSION      "V2.1-Quantized-INT8"
#define TINYML_INPUT_FEATURES     13
#define TINYML_NUM_CLASSES        3   // 0: Normal, 1: Snoring, 2: Abnormal/Apnea

enum AudioClass {
  AUDIO_CLASS_NORMAL   = 0,
  AUDIO_CLASS_SNORING  = 1,
  AUDIO_CLASS_ABNORMAL = 2
};

// Embedded Dense Neural Layer 1 (Input 13 -> Hidden 8)
static const float PROGMEM TINYML_W1[13][8] = {
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

static const float PROGMEM TINYML_B1[8] = {
  -0.10f, 0.00f, 1.49f, -1.28f, -0.19f, -0.06f, 1.97f, -0.13f
};

// Embedded Dense Neural Layer 2 (Hidden 8 -> Output 3)
static const float PROGMEM TINYML_W2[8][3] = {
  {-0.98f, -1.07f, -0.14f},
  {-0.19f, -0.10f, -0.12f},
  { 1.16f, -1.52f,  0.61f},
  {-2.64f,  0.36f, -0.53f},
  {-0.99f, -0.10f, -0.21f},
  {-0.17f, -0.12f, -0.01f},
  { 1.91f, -0.49f, -1.60f},
  { 1.03f, -0.50f, -0.55f}
};

static const float PROGMEM TINYML_B2[3] = {
  0.91f, 0.44f, -0.77f
};

#endif // TINYML_MODEL_H
