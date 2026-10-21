"""
SleepSense AI - Database Manager (SQLite / Relational)
"""

import sqlite3
import os
import json
from datetime import datetime

class Database:
    def __init__(self, db_path):
        self.db_path = db_path
        self.init_db()

    def get_connection(self):
        conn = sqlite3.connect(self.db_path)
        conn.row_factory = sqlite3.Row
        return conn

    def init_db(self):
        os.makedirs(os.path.dirname(os.path.abspath(self.db_path)), exist_ok=True)
        with self.get_connection() as conn:
            cursor = conn.cursor()

            # 1. Sessions Table
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS sessions (
                session_id TEXT PRIMARY KEY,
                patient_name TEXT,
                start_time TEXT,
                end_time TEXT,
                status TEXT,
                device_id TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
            """)

            # 2. Unified Sensor Readings Table
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS sensor_readings (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                session_id TEXT,
                device_id TEXT,
                timestamp INTEGER,
                time_iso TEXT,
                audio_db REAL,
                audio_class TEXT,
                audio_quality REAL,
                spo2 REAL,
                heart_rate REAL,
                hrv REAL,
                ppg_quality REAL,
                posture TEXT,
                movement REAL,
                posture_quality REAL,
                temperature REAL,
                humidity REAL,
                env_quality REAL,
                risk_score REAL,
                risk_level TEXT,
                confidence REAL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (session_id) REFERENCES sessions(session_id)
            );
            """)

            # 3. Events Log Table
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS snore_events (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                session_id TEXT,
                timestamp INTEGER,
                event_type TEXT,
                duration_seconds REAL,
                audio_db REAL,
                spo2 REAL,
                posture TEXT,
                confidence REAL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
            """)

            # 4. Risk Predictions & Adaptive Modality Weights Table
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS risk_predictions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                session_id TEXT,
                timestamp INTEGER,
                risk_score REAL,
                risk_level TEXT,
                confidence REAL,
                audio_weight REAL,
                ppg_weight REAL,
                posture_weight REAL,
                env_weight REAL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
            """)

            # 5. Model Registry Table
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS model_versions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                model_name TEXT,
                version TEXT,
                architecture TEXT,
                quantization TEXT,
                accuracy REAL,
                f1_score REAL,
                latency_micros INTEGER,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
            """)

            # Create Indexes
            cursor.execute("CREATE INDEX IF NOT EXISTS idx_readings_session ON sensor_readings(session_id);")
            cursor.execute("CREATE INDEX IF NOT EXISTS idx_readings_time ON sensor_readings(timestamp);")
            cursor.execute("CREATE INDEX IF NOT EXISTS idx_events_session ON snore_events(session_id);")

            # Seed default active session and model metadata if empty
            cursor.execute("SELECT COUNT(*) FROM sessions;")
            if cursor.fetchone()[0] == 0:
                now_str = datetime.utcnow().isoformat() + "Z"
                cursor.execute("""
                INSERT INTO sessions (session_id, patient_name, start_time, status, device_id)
                VALUES ('SESSION-DEMO-001', 'Anonymous Patient (Research Participant)', ?, 'ACTIVE', 'ESP32-DEVKITV1-SLEEP-01');
                """, (now_str,))

            cursor.execute("SELECT COUNT(*) FROM model_versions;")
            if cursor.fetchone()[0] == 0:
                cursor.execute("""
                INSERT INTO model_versions (model_name, version, architecture, quantization, accuracy, f1_score, latency_micros)
                VALUES ('TinyML Audio CNN', 'v2.1-Quantized', 'Dense 13->8->3', 'INT8 Quantized', 0.992, 0.992, 42);
                """)

            conn.commit()

    def insert_reading(self, reading_dict):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("""
            INSERT INTO sensor_readings (
                session_id, device_id, timestamp, time_iso,
                audio_db, audio_class, audio_quality,
                spo2, heart_rate, hrv, ppg_quality,
                posture, movement, posture_quality,
                temperature, humidity, env_quality,
                risk_score, risk_level, confidence
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
            """, (
                reading_dict.get('session_id', 'SESSION-DEMO-001'),
                reading_dict.get('device_id', 'ESP32-DEVKITV1-SLEEP-01'),
                reading_dict.get('timestamp', int(datetime.utcnow().timestamp())),
                reading_dict.get('time_iso', datetime.utcnow().isoformat() + "Z"),
                reading_dict.get('audio_db', 35.0),
                reading_dict.get('audio_class', 'Normal Breathing'),
                reading_dict.get('audio_quality', 0.95),
                reading_dict.get('spo2', 97.0),
                reading_dict.get('heart_rate', 68.0),
                reading_dict.get('hrv', 35.0),
                reading_dict.get('ppg_quality', 0.95),
                reading_dict.get('posture', 'BACK'),
                reading_dict.get('movement', 0.1),
                reading_dict.get('posture_quality', 0.95),
                reading_dict.get('temperature', 24.5),
                reading_dict.get('humidity', 55.0),
                reading_dict.get('env_quality', 0.98),
                reading_dict.get('risk_score', 15.0),
                reading_dict.get('risk_level', 'LOW'),
                reading_dict.get('confidence', 0.92)
            ))

            # Record event if snore or desaturation occurred
            if reading_dict.get('audio_class') in ['Snoring', 'Abnormal Breathing / Apnea'] or reading_dict.get('snore_detected'):
                cursor.execute("""
                INSERT INTO snore_events (session_id, timestamp, event_type, duration_seconds, audio_db, spo2, posture, confidence)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?);
                """, (
                    reading_dict.get('session_id', 'SESSION-DEMO-001'),
                    reading_dict.get('timestamp', int(datetime.utcnow().timestamp())),
                    'SNORING' if reading_dict.get('audio_class') == 'Snoring' else 'ABNORMAL_BREATHING',
                    2.0,
                    reading_dict.get('audio_db', 55.0),
                    reading_dict.get('spo2', 97.0),
                    reading_dict.get('posture', 'BACK'),
                    reading_dict.get('confidence', 0.90)
                ))

            if reading_dict.get('spo2', 100.0) < 92.0 and reading_dict.get('spo2', 0.0) > 0.0:
                cursor.execute("""
                INSERT INTO snore_events (session_id, timestamp, event_type, duration_seconds, audio_db, spo2, posture, confidence)
                VALUES (?, ?, 'OXYGEN_DESATURATION', 2.0, ?, ?, ?, 0.98);
                """, (
                    reading_dict.get('session_id', 'SESSION-DEMO-001'),
                    reading_dict.get('timestamp', int(datetime.utcnow().timestamp())),
                    reading_dict.get('audio_db', 40.0),
                    reading_dict.get('spo2', 90.0),
                    reading_dict.get('posture', 'BACK')
                ))

            conn.commit()

    def get_latest_reading(self, session_id=None):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            if session_id and session_id != 'latest':
                cursor.execute("SELECT * FROM sensor_readings WHERE session_id = ? ORDER BY id DESC LIMIT 1;", (session_id,))
            else:
                cursor.execute("SELECT * FROM sensor_readings ORDER BY id DESC LIMIT 1;")
            row = cursor.fetchone()
            return dict(row) if row else None

    def get_history(self, session_id=None, limit=60):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            if session_id and session_id != 'latest':
                cursor.execute("SELECT * FROM sensor_readings WHERE session_id = ? ORDER BY id DESC LIMIT ?;", (session_id, limit))
            else:
                cursor.execute("SELECT * FROM sensor_readings ORDER BY id DESC LIMIT ?;", (limit,))
            rows = cursor.fetchall()
            return [dict(r) for r in reversed(rows)]

    def get_events(self, session_id=None, limit=50):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            if session_id and session_id != 'latest':
                cursor.execute("SELECT * FROM snore_events WHERE session_id = ? ORDER BY id DESC LIMIT ?;", (session_id, limit))
            else:
                cursor.execute("SELECT * FROM snore_events ORDER BY id DESC LIMIT ?;", (limit,))
            rows = cursor.fetchall()
            return [dict(r) for r in rows]
