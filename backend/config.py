"""
SleepSense AI - Backend Configuration
"""

import os

class Config:
    PROJECT_NAME = "SleepSense AI"
    VERSION = "2.0.0"
    HOST = os.environ.get("HOST", "0.0.0.0")
    PORT = int(os.environ.get("PORT", 5000))
    DEBUG = os.environ.get("DEBUG", "False").lower() in ["true", "1"]

    # Database Configuration (SQLite default, PostgreSQL ready)
    BASE_DIR = os.path.abspath(os.path.dirname(__file__))
    DB_PATH = os.environ.get("DB_PATH", os.path.join(BASE_DIR, "sleepsense.db"))
    DATABASE_URI = f"sqlite:///{DB_PATH}"

    # Adaptive Fusion Base Modality Weights (3-Sensor Edge Architecture)
    BASE_WEIGHT_AUDIO = 0.45     # 45% Acoustic Snore/Breathing Burden
    BASE_WEIGHT_PPG = 0.45       # 45% Physiological SpO2 & Heart Rate Stability
    BASE_WEIGHT_POSTURE = 0.10   # 10% Gravitational Sleep Posture & Restlessness
    BASE_WEIGHT_ENV = 0.00       # 0% (DHT22 not in hardware setup)

    # Risk Level Thresholds (0 to 100 Scale)
    RISK_THRESHOLD_LOW = 25.0
    RISK_THRESHOLD_MODERATE = 45.0
    RISK_THRESHOLD_HIGH = 70.0

    # CORS settings
    CORS_ORIGINS = "*"
