/*
 * Audio Acquisition & Acoustic Feature Extraction Implementation
 */

#include "audio.h"
#include <math.h>

AudioEngine audio;

AudioEngine::AudioEngine() 
  : initialized(false), snoreStartTime(0), inSnoreEvent(false), consecutiveSnoreWindows(0) {
  memset(&currentPacket, 0, sizeof(AudioPacket));
  currentPacket.estimatedDb = 35.0f;
  currentPacket.quality = 0.9f;
}

bool AudioEngine::initI2S() {
  Serial.println("[INFO] Initializing INMP441 I2S DMA Digital Audio Driver (16kHz Mono 16-bit)...");
  Serial.printf("[INFO] I2S Pins: SCK/BCLK=%d, WS/LRCLK=%d, SD/DOUT=%d\n", I2S_SCK_PIN, I2S_WS_PIN, I2S_SD_PIN);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = AUDIO_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = AUDIO_WINDOW_SAMPLES,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD_PIN
  };

  esp_err_t err = i2s_driver_install(I2S_PORT_NUM, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Failed to install I2S driver! Error code: %d\n", err);
    initialized = false;
    return false;
  }

  err = i2s_set_pin(I2S_PORT_NUM, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Failed to set I2S pins! Error code: %d\n", err);
    initialized = false;
    return false;
  }

  i2s_zero_dma_buffer(I2S_PORT_NUM);
  initialized = true;
  Serial.println("[INFO] INMP441 Microphone initialized successfully via I2S DMA.");
  return true;
}

bool AudioEngine::captureAndProcess() {
  if (!initialized) {
    currentPacket.isValid = false;
    currentPacket.quality = 0.0f;
    return false;
  }

  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT_NUM, (void*)sampleBuffer, sizeof(sampleBuffer), &bytesRead, pdMS_TO_TICKS(50));

  if (result != ESP_OK || bytesRead == 0) {
    currentPacket.isValid = false;
    currentPacket.quality = 0.1f;
    return false;
  }

  size_t numSamples = bytesRead / sizeof(int16_t);
  calculateFeatures(numSamples);
  evaluateSignalQuality();

  currentPacket.isValid = true;
  return true;
}

void AudioEngine::calculateFeatures(size_t numSamples) {
  if (numSamples == 0) return;

  double sumSquare = 0.0;
  int16_t maxSample = 0;
  int clippingCount = 0;

  float bandAccumulator[AUDIO_FEATURE_NUM] = {0.0f};
  size_t bandSize = numSamples / AUDIO_FEATURE_NUM;
  if (bandSize < 1) bandSize = 1;

  for (size_t i = 0; i < numSamples; i++) {
    int16_t sample = sampleBuffer[i];
    double s = (double)sample;
    sumSquare += (s * s);

    int16_t absVal = abs(sample);
    if (absVal > maxSample) maxSample = absVal;
    if (absVal >= 32700) clippingCount++;

    size_t bandIdx = i / bandSize;
    if (bandIdx >= AUDIO_FEATURE_NUM) bandIdx = AUDIO_FEATURE_NUM - 1;
    bandAccumulator[bandIdx] += fabsf((float)sample);
  }

  // Calculate RMS and Peak
  currentPacket.rms = (float)sqrt(sumSquare / (double)numSamples);
  currentPacket.peak = (float)maxSample;

  // Calculate estimated relative dB (reference normalized to full scale 16-bit)
  float rawRms = (currentPacket.rms < 1.0f) ? 1.0f : currentPacket.rms;
  currentPacket.estimatedDb = 120.0f + 20.0f * log10f(rawRms / 32767.0f);
  if (currentPacket.estimatedDb < 30.0f) currentPacket.estimatedDb = 30.0f;

  // Populate 13 spectral band energies
  for (size_t b = 0; b < AUDIO_FEATURE_NUM; b++) {
    float avgBand = bandAccumulator[b] / (float)bandSize;
    currentPacket.bandEnergies[b] = log1pf(avgBand);
  }

  // Snore detection debouncing with temporal hysteresis
  bool rawSnoreTrigger = (currentPacket.estimatedDb >= SNORE_EST_DB_TH) || (currentPacket.snoreProbability > 0.65f);

  if (rawSnoreTrigger) {
    consecutiveSnoreWindows++;
    if (consecutiveSnoreWindows >= 2) { // Debounce: Must persist >= 2 windows
      if (!inSnoreEvent) {
        inSnoreEvent = true;
        snoreStartTime = millis();
      }
      currentPacket.snoreDetected = true;
      currentPacket.snoreDurationMs = (uint32_t)(millis() - snoreStartTime);
    }
  } else {
    consecutiveSnoreWindows = 0;
    inSnoreEvent = false;
    currentPacket.snoreDetected = false;
    currentPacket.snoreDurationMs = 0;
  }
}

void AudioEngine::evaluateSignalQuality() {
  // Quality estimation: penalize heavy clipping and dead microphones (RMS < 5)
  if (currentPacket.rms < 5.0f) {
    currentPacket.quality = 0.2f; // Dead mic / silence
  } else if (currentPacket.peak >= 32700) {
    currentPacket.quality = 0.6f; // Severe clipping
  } else {
    currentPacket.quality = 0.95f; // Good dynamic range
  }
}

void AudioEngine::setSnoreProbability(float prob) {
  currentPacket.snoreProbability = prob;
}
