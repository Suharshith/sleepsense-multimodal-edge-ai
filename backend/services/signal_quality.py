"""
SleepSense AI - Signal Quality Assurance & Validation Service
"""

class SignalQualityService:
    @staticmethod
    def evaluate_audio_quality(rms, peak, clipping_ratio=0.0):
        if rms < 5.0:
            return 0.15, "Poor (Microphone disconnected or dead quiet)"
        if peak >= 32700 or clipping_ratio > 0.10:
            return 0.55, "Needs Attention (Acoustic clipping detected)"
        return 0.95, "Excellent"

    @staticmethod
    def evaluate_ppg_quality(finger_detected, spo2, bpm, raw_ir=80000):
        if not finger_detected or spo2 <= 0.0 or bpm <= 0.0:
            return 0.0, "Disconnected (No finger detected)"
        if spo2 < 70.0 or spo2 > 100.0 or bpm < 35.0 or bpm > 200.0:
            return 0.30, "Needs Attention (Physiologically implausible readings)"
        if raw_ir < 50000:
            return 0.70, "Good (Low optical perfusion)"
        return 0.96, "Excellent"

    @staticmethod
    def evaluate_posture_quality(mag, is_moving=False):
        dev = abs(mag - 1.0)
        if is_moving:
            return 0.85, "Good (Active sleep motion)"
        if dev < 0.25:
            return 0.95, "Excellent (Stable earth gravity reference)"
        return 0.60, "Needs Attention (Orientation sensor drift)"

    @staticmethod
    def evaluate_environment_quality(temp, hum):
        if temp < -20.0 or temp > 60.0 or hum < 0.0 or hum > 100.0:
            return 0.20, "Needs Attention (Sensor readout out of bounds)"
        return 0.98, "Excellent"
