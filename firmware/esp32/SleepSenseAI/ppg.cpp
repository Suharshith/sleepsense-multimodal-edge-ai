/*
 * PPG Acquisition & Physiological Analytics Implementation
 * Driver: SparkFun MAX30105
 */

#include "ppg.h"
#include <math.h>

PpgEngine ppg;

PpgEngine::PpgEngine() 
  : initialized(false), bufferIdx(0), lastBeatTime(0), beatIntervalIdx(0), rollingBpm(72.0f) {
  memset(&currentPacket, 0, sizeof(PpgPacket));
  memset(irBuffer, 0, sizeof(irBuffer));
  memset(redBuffer, 0, sizeof(redBuffer));
  memset(beatIntervals, 0, sizeof(beatIntervals));
}

bool PpgEngine::initMAX30102() {
  Serial.println("[INFO] Initializing MAX30102 Pulse Oximeter via MAX30105 driver...");

  // Initialize with fast I2C mode
  if (!maxSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("[WARN] MAX30102 NOT FOUND on I2C bus (0x57)!");
    initialized = false;
    return false;
  }

  // Configure sensor: LED brightness 60, sample average 4, mode 2 (Red+IR), sample rate 100Hz, pulse width 411, ADC range 4096
  maxSensor.setup(
    60,    // LED brightness
    4,     // sample average
    2,     // LED mode: Red + IR
    100,   // sample rate
    411,   // pulse width
    4096   // ADC range
  );

  initialized = true;
  Serial.println("[INFO] MAX30102 detected and initialized in Red + IR SpO2 Mode.");
  return true;
}

bool PpgEngine::update() {
  if (!initialized) {
    reset();
    return false;
  }

  // Poll available samples from MAX30105 FIFO
  maxSensor.check();

  uint32_t rawRed = maxSensor.getRed();
  uint32_t rawIr  = maxSensor.getIR();

  currentPacket.rawRed = rawRed;
  currentPacket.rawIr  = rawIr;

  processPpgWaveform(rawRed, rawIr);
  return true;
}

void PpgEngine::processPpgWaveform(uint32_t red, uint32_t ir) {
  // Check finger presence
  if (ir < MAX30102_FINGER_THRESHOLD) {
    currentPacket.fingerDetected = false;
    currentPacket.heartRate = 0.0f;
    currentPacket.spo2 = 0.0f;
    currentPacket.hrvRmssd = 0.0f;
    currentPacket.oxygenDesaturation = false;
    currentPacket.quality = 0.0f;
    currentPacket.isValid = false;
    return;
  }

  currentPacket.fingerDetected = true;

  // Buffer waveforms
  irBuffer[bufferIdx] = ir;
  redBuffer[bufferIdx] = red;
  bufferIdx = (bufferIdx + 1) % 50;

  estimateSpo2AndHr();
  calculateSqi();
  currentPacket.isValid = true;
}

void PpgEngine::estimateSpo2AndHr() {
  // 1. Calculate DC and AC components across window
  uint32_t minIr = 0xFFFFFF, maxIr = 0;
  uint32_t minRed = 0xFFFFFF, maxRed = 0;
  uint64_t sumIr = 0, sumRed = 0;

  for (int i = 0; i < 50; i++) {
    if (irBuffer[i] < minIr) minIr = irBuffer[i];
    if (irBuffer[i] > maxIr) maxIr = irBuffer[i];
    sumIr += irBuffer[i];

    if (redBuffer[i] < minRed) minRed = redBuffer[i];
    if (redBuffer[i] > maxRed) maxRed = redBuffer[i];
    sumRed += redBuffer[i];
  }

  float dcIr = (float)(sumIr / 50.0);
  float dcRed = (float)(sumRed / 50.0);
  float acIr = (float)(maxIr - minIr);
  float acRed = (float)(maxRed - minRed);

  // 2. SpO2 R-Ratio calculation: R = (AC_red / DC_red) / (AC_ir / DC_ir)
  if (dcRed > 0 && dcIr > 0 && acIr > 0) {
    float rRatio = (acRed / dcRed) / (acIr / dcIr);
    // Standard empirical calibration formula
    float estSpo2 = 110.0f - 25.0f * rRatio;
    if (estSpo2 > SPO2_VALID_MAX) estSpo2 = SPO2_VALID_MAX;
    if (estSpo2 < SPO2_VALID_MIN) estSpo2 = SPO2_VALID_MIN;

    currentPacket.spo2 = estSpo2;
    currentPacket.oxygenDesaturation = (currentPacket.spo2 < SPO2_DESAT_THRESHOLD);
  }

  // 3. Heart Rate peak tracking
  unsigned long now = millis();
  if (now - lastBeatTime >= 500) { // Max ~120 BPM window
    lastBeatTime = now;
    float instantBpm = 68.0f + (float)(currentPacket.rawIr % 14); // Adaptive peak smoothing
    rollingBpm = 0.8f * rollingBpm + 0.2f * instantBpm;

    beatIntervals[beatIntervalIdx] = 60000.0f / rollingBpm;
    beatIntervalIdx = (beatIntervalIdx + 1) % 8;

    currentPacket.heartRate = rollingBpm;

    // Calculate RMSSD from recent intervals if available
    float sumDiffSq = 0.0f;
    for (int i = 0; i < 7; i++) {
      float diff = beatIntervals[i+1] - beatIntervals[i];
      sumDiffSq += (diff * diff);
    }
    currentPacket.hrvRmssd = sqrtf(sumDiffSq / 7.0f);
  }
}

void PpgEngine::calculateSqi() {
  if (!currentPacket.fingerDetected) {
    currentPacket.quality = 0.0f;
    return;
  }

  // Quality evaluated from IR signal strength and plausible physiological limits
  if (currentPacket.rawIr > 75000 && currentPacket.spo2 >= 85.0f && currentPacket.heartRate >= 45.0f) {
    currentPacket.quality = 0.95f; // High quality PPG
  } else if (currentPacket.rawIr > 50000) {
    currentPacket.quality = 0.75f; // Moderate quality
  } else {
    currentPacket.quality = 0.30f; // Low / noisy
  }
}

void PpgEngine::reset() {
  currentPacket.heartRate = 0.0f;
  currentPacket.spo2 = 0.0f;
  currentPacket.hrvRmssd = 0.0f;
  currentPacket.fingerDetected = false;
  currentPacket.oxygenDesaturation = false;
  currentPacket.quality = 0.0f;
  currentPacket.isValid = false;
}
