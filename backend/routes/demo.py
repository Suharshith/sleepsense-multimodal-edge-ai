"""
SleepSense AI - Demo Mode Generator Blueprint
Generates realistic multi-modal telemetry streams for demonstration and offline evaluation.
"""

from flask import Blueprint, jsonify, request
from datetime import datetime, timedelta
import random
import math

demo_bp = Blueprint('demo', __name__, url_prefix='/api/v1/demo')

def get_db():
    from flask import current_app
    return current_app.config['DB']

@demo_bp.route('/seed', methods=['POST'])
def seed_demo_telemetry():
    """
    Seeds a continuous 30-minute realistic multi-modal sleep session with:
    - Normal baseline breathing phase
    - Acoustic snoring phase
    - Obstructive Apnea episode (acoustic silence + SpO2 desaturation dip < 92%)
    - Recovery and posture shifting
    """
    db = get_db()
    now = datetime.utcnow()
    start_time = now - timedelta(minutes=30)
    
    postures = ["BACK", "LEFT_SIDE", "RIGHT_SIDE", "STOMACH"]
    current_spo2 = 98.0
    current_hr = 64.0
    
    readings_count = 90 # 90 frames @ 20s interval = 30 mins

    for i in range(readings_count):
        t = start_time + timedelta(seconds=i * 20)
        timestamp = int(t.timestamp())

        # Determine Sleep Phase Pattern
        if 20 <= i <= 35:
            # Phase 2: Snoring Phase
            audio_class = "Snoring"
            est_db = 58.0 + random.uniform(2.0, 10.0)
            snore_prob = 0.92 + random.uniform(0.01, 0.06)
            snore_detected = True
            current_spo2 = 96.0 + random.uniform(-1.0, 1.5)
            current_hr = 66.0 + random.uniform(-2.0, 4.0)
            posture = "BACK"
            risk_score = 42.0 + random.uniform(-3.0, 5.0)
            risk_level = "LOW"
        elif 50 <= i <= 65:
            # Phase 3: Obstructive Sleep Apnea Episode with SpO2 Desaturation
            audio_class = "Abnormal Breathing / Apnea"
            est_db = 68.0 + random.uniform(3.0, 12.0)
            snore_prob = 0.95
            snore_detected = True
            current_spo2 = 88.5 + random.uniform(-2.0, 2.5) # Desaturation < 92%
            current_hr = 78.0 + random.uniform(4.0, 14.0)   # Tachycardic compensation
            posture = "BACK"
            risk_score = 74.5 + random.uniform(-4.0, 8.0)
            risk_level = "HIGH"
        else:
            # Normal Sleep Phase
            audio_class = "Normal Breathing"
            est_db = 36.0 + random.uniform(0.5, 6.0)
            snore_prob = 0.12
            snore_detected = False
            current_spo2 = 97.5 + random.uniform(-0.8, 1.5)
            current_hr = 62.0 + random.uniform(-2.0, 3.0)
            posture = postures[(i // 25) % len(postures)]
            risk_score = 14.0 + random.uniform(-2.0, 4.0)
            risk_level = "LOW"

        reading = {
            "session_id": "SESSION-DEMO-001",
            "device_id": "ESP32-DEVKITV1-SLEEP-01",
            "timestamp": timestamp,
            "time_iso": t.isoformat() + "Z",
            "audio_db": round(est_db, 1),
            "audio_class": audio_class,
            "audio_quality": 0.95,
            "snore_detected": snore_detected,
            "spo2": round(min(100.0, max(75.0, current_spo2)), 1),
            "heart_rate": round(current_hr, 1),
            "hrv": round(35.0 + random.uniform(-5.0, 12.0), 1),
            "ppg_quality": 0.96,
            "posture": posture,
            "movement": round(0.08 + random.uniform(0.01, 0.15), 2),
            "posture_quality": 0.95,
            "temperature": 24.2,
            "humidity": 56.0,
            "env_quality": 0.98,
            "risk_score": round(risk_score, 1),
            "risk_level": risk_level,
            "confidence": 0.92
        }

        db.insert_reading(reading)

    return jsonify({
        "status": "success",
        "message": f"Seeded {readings_count} realistic multi-modal readings into active session."
    })
