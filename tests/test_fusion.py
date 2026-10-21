"""
SleepSense AI - Adaptive Multimodal Fusion Algorithm Unit Tests
"""

import sys
import os
import unittest

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "backend")))
from services.adaptive_fusion import adaptive_fusion

class FusionEngineTestCase(unittest.TestCase):
    def test_normal_healthy_sleep_profile(self):
        audio = {"class": "Normal Breathing", "snore_detected": False, "confidence": 0.95, "quality": 0.95}
        ppg = {"spo2": 98.0, "bpm": 62.0, "quality": 0.96}
        posture = {"class": "LEFT_SIDE", "movement": 0.05, "quality": 0.95}

        res = adaptive_fusion.evaluate(audio, ppg, posture)
        self.assertEqual(res["risk_level"], "LOW")
        self.assertLess(res["risk_score"], 25.0)
        self.assertGreater(res["confidence"], 0.85)

    def test_ppg_sensor_disconnected_adaptation(self):
        """When optical pulse oximeter is disconnected, PPG weight drops to 0 and shifts to audio/posture."""
        audio = {"class": "Snoring", "snore_detected": True, "confidence": 0.92, "quality": 0.95}
        ppg = {"spo2": 0.0, "bpm": 0.0, "quality": 0.0} # Disconnected
        posture = {"class": "BACK", "movement": 0.05, "quality": 0.95}

        res = adaptive_fusion.evaluate(audio, ppg, posture)
        self.assertEqual(res["weights"]["ppg"], 0.0) # Zero weight assigned to missing sensor
        self.assertGreater(res["weights"]["audio"], 0.70) # Dynamically shifted to active audio modality

    def test_severe_apnea_desaturation_pattern(self):
        """Acoustic gasping/snoring + severe SpO2 desaturation below 92%."""
        audio = {"class": "Abnormal Breathing / Apnea", "snore_detected": True, "confidence": 0.96, "quality": 0.95}
        ppg = {"spo2": 87.5, "bpm": 88.0, "quality": 0.96} # Desaturation
        posture = {"class": "BACK", "movement": 0.12, "quality": 0.95}

        res = adaptive_fusion.evaluate(audio, ppg, posture)
        self.assertEqual(res["risk_level"], "HIGH")
        self.assertGreater(res["risk_score"], 65.0)

    def test_insufficient_data_state(self):
        """When all sensor qualities are 0 (e.g. startup / disconnected)."""
        audio = {"quality": 0.0}
        ppg = {"quality": 0.0}
        posture = {"quality": 0.0}
        env = {"quality": 0.0}

        res = adaptive_fusion.evaluate(audio, ppg, posture, env)
        self.assertEqual(res["risk_level"], "INSUFFICIENT_DATA")
        self.assertEqual(res["risk_score"], 0.0)

if __name__ == '__main__':
    unittest.main()
