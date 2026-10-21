/*
 * PPG Acquisition & Physiological Analytics Header
 * Sensor: MAX30102 Pulse Oximeter & Heart Rate Monitor (I2C)
 * Driver: SparkFun MAX30105
 */

#ifndef PPG_H
#define PPG_H

#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "config.h"

struct PpgPacket {
  float heartRate;
  float spo2;
  float hrvRmssd;
  uint32_t rawRed;
  uint32_t rawIr;
  bool fingerDetected;
  bool oxygenDesaturation;
  float quality; // PPG Signal Quality Index (PPG_SQI) [0.0 to 1.0]
  bool isValid;
};

class PpgEngine {
private:
  MAX30105 maxSensor;
  bool initialized;
  PpgPacket currentPacket;

  // Signal processing buffers
  uint32_t irBuffer[50];
  uint32_t redBuffer[50];
  uint8_t bufferIdx;

  // Peak detection & BPM calculation
  unsigned long lastBeatTime;
  float beatIntervals[8];
  uint8_t beatIntervalIdx;
  float rollingBpm;

  void processPpgWaveform(uint32_t red, uint32_t ir);
  void estimateSpo2AndHr();
  void calculateSqi();

public:
  PpgEngine();
  bool initMAX30102();
  bool update();
  PpgPacket getPacket() const { return currentPacket; }
  void reset();
};

extern PpgEngine ppg;

#endif // PPG_H
