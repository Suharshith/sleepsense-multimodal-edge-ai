/*
 * Timestamp Synchronized Multimodal Risk Fusion Engine Implementation
 */

#include "multimodal_fusion.h"
#include <time.h>

MultimodalFusionEngine::MultimodalFusionEngine() 
  : snoreCounter(0), abnormalAudioCounter(0), desatCounter(0), posChangeCounter(0) {
  memset(&currentAssessment, 0, sizeof(RiskAssessment));
  currentAssessment.levelName = "NORMAL";
}

RiskAssessment MultimodalFusionEngine::evaluate(const AudioFeatures& audio,
                                                const InferenceResult& tinyml,
                                                const PhysiologicalReadings& physio,
                                                const MotionReadings& motion) {
  time_t nowSec;
  time(&nowSec);
  currentAssessment.timestamp = (nowSec > 1000000000) ? (unsigned long)nowSec : millis();

  // 1. Audio Risk Component Calculation
  float audioSubScore = 0.0f;
  if (tinyml.predictedClass == AUDIO_CLASS_ABNORMAL) {
    audioSubScore = 90.0f * tinyml.confidence;
    abnormalAudioCounter++;
  } else if (tinyml.predictedClass == AUDIO_CLASS_SNORING) {
    audioSubScore = 55.0f * tinyml.confidence;
    snoreCounter++;
  } else if (audio.snoreDetected) {
    audioSubScore = 40.0f;
  }
  currentAssessment.audioRiskContrib = audioSubScore;

  // 2. Physiological (SpO2 & HR) Risk Component Calculation
  float spo2SubScore = 0.0f;
  float hrSubScore = 0.0f;
  if (physio.fingerDetected && physio.isValid) {
    if (physio.spo2 < SPO2_DESAT_THRESHOLD) {
      spo2SubScore = (SPO2_DESAT_THRESHOLD - physio.spo2) * 12.0f; // Scale drop
      if (spo2SubScore > 100.0f) spo2SubScore = 100.0f;
      desatCounter++;
    } else {
      spo2SubScore = 10.0f;
    }

    if (physio.heartRate > 100.0f || physio.heartRate < 50.0f) {
      hrSubScore = 45.0f;
    } else {
      hrSubScore = 10.0f;
    }
  } else {
    // If no finger present, ignore physiological risk penalty
    spo2SubScore = 0.0f;
    hrSubScore = 0.0f;
  }
  currentAssessment.spo2RiskContrib = spo2SubScore;
  currentAssessment.hrRiskContrib = hrSubScore;

  // 3. Motion & Posture Risk Component Calculation
  float motionSubScore = 0.0f;
  if (motion.positionChanged) {
    posChangeCounter++;
    motionSubScore += 30.0f;
  }
  if (motion.posture == POSTURE_SUPINE && audio.snoreDetected) {
    motionSubScore += 35.0f; // Supine posture exacerbates sleep apnea risk
  }
  motionSubScore += motion.restlessnessScore * 40.0f;
  if (motionSubScore > 100.0f) motionSubScore = 100.0f;
  currentAssessment.motionRiskContrib = motionSubScore;

  // 4. Multimodal Fusion Weighted Risk Index Calculation
  float fusedScore = (audioSubScore  * WEIGHT_AUDIO_BURDEN) +
                     (spo2SubScore   * WEIGHT_SPO2_DESAT) +
                     (hrSubScore     * WEIGHT_HR_VARIATION) +
                     (motionSubScore * WEIGHT_MOTION_BURDEN);

  if (fusedScore > 100.0f) fusedScore = 100.0f;
  if (fusedScore < 0.0f)   fusedScore = 0.0f;

  currentAssessment.score = fusedScore;

  // Assign Categorical Risk Level
  if (fusedScore >= 70.0f) {
    currentAssessment.level = RISK_LEVEL_ELEVATED;
    currentAssessment.levelName = "ELEVATED";
  } else if (fusedScore >= 45.0f) {
    currentAssessment.level = RISK_LEVEL_MODERATE;
    currentAssessment.levelName = "MODERATE";
  } else if (fusedScore >= 25.0f) {
    currentAssessment.level = RISK_LEVEL_LOW;
    currentAssessment.levelName = "LOW";
  } else {
    currentAssessment.level = RISK_LEVEL_NORMAL;
    currentAssessment.levelName = "NORMAL";
  }

  currentAssessment.snoringEventCount = snoreCounter;
  currentAssessment.abnormalAudioCount = abnormalAudioCounter;
  currentAssessment.desaturationCount = desatCounter;
  currentAssessment.positionChangeCount = posChangeCounter;

  return currentAssessment;
}

void MultimodalFusionEngine::resetCounters() {
  snoreCounter = 0;
  abnormalAudioCounter = 0;
  desatCounter = 0;
  posChangeCounter = 0;
}
