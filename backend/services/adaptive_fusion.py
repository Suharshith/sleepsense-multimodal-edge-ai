"""
SleepSense AI - Adaptive Signal-Quality Weighted Multimodal Fusion Engine
Core Research Contribution:
Dynamically computes multi-sensor risk indices by weighting modalities based on real-time signal quality.
"""

from config import Config

class AdaptiveFusionEngine:
    def __init__(self):
        self.base_weights = {
            "audio": Config.BASE_WEIGHT_AUDIO,
            "ppg": Config.BASE_WEIGHT_PPG,
            "posture": Config.BASE_WEIGHT_POSTURE,
            "env": Config.BASE_WEIGHT_ENV
        }

    def evaluate(self, audio_data, ppg_data, posture_data, env_data=None):
        """
        Calculates adaptive multimodal risk assessment from multi-sensor telemetry packets.
        Supports 3-sensor hardware (Audio + PPG + Posture).
        """
        if env_data is None:
            env_data = {}

        # 1. Extract Signal Qualities [0.0 to 1.0]
        q_audio = float(audio_data.get("quality", 0.95))
        q_ppg = float(ppg_data.get("quality", 0.95))
        q_posture = float(posture_data.get("quality", 0.95))
        q_env = float(env_data.get("quality", 0.0))

        # 2. Compute Individual Modality Sub-Scores [0.0 to 100.0]
        s_audio = self._compute_audio_subscore(audio_data)
        s_ppg = self._compute_ppg_subscore(ppg_data)
        s_posture = self._compute_posture_subscore(posture_data, audio_data)
        s_env = self._compute_env_subscore(env_data) if env_data else 0.0

        # 3. Dynamic Weight Adaptation based on Quality
        w_audio = self.base_weights["audio"] * q_audio
        w_ppg = self.base_weights["ppg"] * q_ppg
        w_posture = self.base_weights["posture"] * q_posture
        w_env = self.base_weights["env"] * q_env

        sum_w = w_audio + w_ppg + w_posture + w_env

        # 4. Insufficient Data Check
        if sum_w < 0.15:
            return {
                "risk_score": 0.0,
                "risk_level": "INSUFFICIENT_DATA",
                "confidence": 0.0,
                "status_message": "Insufficient sensor data collected for a risk assessment.",
                "weights": {"audio": 0.0, "ppg": 0.0, "posture": 0.0, "env": 0.0},
                "contributions": {"audio": 0.0, "ppg": 0.0, "posture": 0.0, "env": 0.0}
            }

        # Normalize weights to sum to 1.0
        norm_audio = w_audio / sum_w
        norm_ppg = w_ppg / sum_w
        norm_posture = w_posture / sum_w
        norm_env = w_env / sum_w if sum_w > 0 else 0.0

        # 5. Composite Risk Score Calculation
        fused_score = (norm_audio * s_audio) + (norm_ppg * s_ppg) + (norm_posture * s_posture) + (norm_env * s_env)
        fused_score = max(0.0, min(100.0, fused_score))

        # 6. Overall Confidence estimation (Tri-Modal active sensors)
        confidence = (q_audio * 0.45) + (q_ppg * 0.45) + (q_posture * 0.10)

        # 7. Map to Calm, Scientifically Defensible Risk Levels
        if fused_score >= Config.RISK_THRESHOLD_HIGH:
            risk_level = "HIGH"
            msg = "An elevated sleep-apnea risk pattern was detected."
        elif fused_score >= Config.RISK_THRESHOLD_MODERATE:
            risk_level = "MODERATE"
            msg = "A moderate sleep-apnea risk pattern was detected."
        else:
            risk_level = "LOW"
            msg = "No significant concerning sleep-apnea pattern detected so far."

        return {
            "risk_score": round(float(fused_score), 1),
            "risk_level": risk_level,
            "confidence": round(float(confidence), 2),
            "status_message": msg,
            "weights": {
                "audio": round(float(norm_audio), 3),
                "ppg": round(float(norm_ppg), 3),
                "posture": round(float(norm_posture), 3),
                "env": round(float(norm_env), 3)
            },
            "contributions": {
                "audio": round(float(s_audio), 1),
                "ppg": round(float(s_ppg), 1),
                "posture": round(float(s_posture), 1),
                "env": round(float(s_env), 1)
            }
        }

    def _compute_audio_subscore(self, audio):
        audio_class = audio.get("class", audio.get("audio_class", "Normal Breathing"))
        conf = float(audio.get("confidence", audio.get("snore_probability", 0.9)))
        est_db = float(audio.get("estimated_db", audio.get("db", 40.0)))

        if "Abnormal" in audio_class or "Apnea" in audio_class:
            score = 95.0 * conf
        elif "Snoring" in audio_class or audio.get("snore_detected"):
            score = 55.0 * conf
        else:
            score = 10.0

        if est_db > 65.0:
            score += 10.0

        return min(100.0, score)

    def _compute_ppg_subscore(self, ppg):
        spo2 = float(ppg.get("spo2", 97.0))
        bpm = float(ppg.get("bpm", ppg.get("heart_rate", 70.0)))

        if spo2 <= 0.0 or bpm <= 0.0:
            return 0.0 # Sensor disconnected

        score = 0.0
        # SpO2 desaturation penalty
        if spo2 < 92.0:
            score = 30.0 + min(70.0, (92.0 - spo2) * 14.0)
        else:
            score = 5.0

        # Heart rate instability / tachycardia / bradycardia
        if bpm > 100.0 or bpm < 50.0:
            score += 20.0
        elif bpm > 85.0:
            score += 10.0
        else:
            score += 5.0

        return min(100.0, score)

    def _compute_posture_subscore(self, posture, audio):
        pos_class = str(posture.get("class", posture.get("posture", "BACK"))).upper()
        restless = float(posture.get("movement", posture.get("restlessness", 0.05)))
        snoring = audio.get("snore_detected", False) or "Snoring" in str(audio.get("class", ""))

        score = 10.0
        # Supine posture exacerbation when snoring or breathing sounds are elevated
        if "BACK" in pos_class or "SUPINE" in pos_class:
            score += 25.0
            if snoring:
                score += 20.0

        # Restless tossing
        score += min(35.0, restless * 50.0)
        return min(100.0, score)

    def _compute_env_subscore(self, env):
        temp = float(env.get("temperature", 24.0))
        hum = float(env.get("humidity", 50.0))

        score = 10.0
        if temp > 27.0 or temp < 17.0:
            score += 15.0
        if hum > 70.0 or hum < 35.0:
            score += 15.0
        return min(100.0, score)

adaptive_fusion = AdaptiveFusionEngine()
