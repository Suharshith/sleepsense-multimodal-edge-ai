/*
 * Timestamp Synchronized Multimodal Risk Fusion Engine Header
 */

#ifndef MULTIMODAL_FUSION_H
#define MULTIMODAL_FUSION_H

#include <Arduino.h>
#include "config.h"
#include "inmp441_driver.h"
#include "max30102_driver.h"
#include "mpu6050_driver.h"
#include "audio_classifier.h"

struct RiskAssessment {
  float score; // 0 to 100
  RiskLevel level;
  const char* levelName;
  
  // Explainable Contributing Factors (0.0 to 100.0%)
  float audioRiskContrib;
  float spo2RiskContrib;
  float hrRiskContrib;
  float motionRiskContrib;

  // Event Counters & Timestamps
  unsigned long timestamp;
  uint16_t snoringEventCount;
  uint16_t abnormalAudioCount;
  uint16_t desaturationCount;
  uint16_t positionChangeCount;
};

class MultimodalFusionEngine {
private:
  RiskAssessment currentAssessment;
  uint16_t snoreCounter;
  uint16_t abnormalAudioCounter;
  uint16_t desatCounter;
  uint16_t posChangeCounter;

public:
  MultimodalFusionEngine();
  RiskAssessment evaluate(const AudioFeatures& audio,
                          const InferenceResult& tinyml,
                          const PhysiologicalReadings& physio,
                          const MotionReadings& motion);

  RiskAssessment getAssessment() const { return currentAssessment; }
  void resetCounters();
};

#endif // MULTIMODAL_FUSION_H
