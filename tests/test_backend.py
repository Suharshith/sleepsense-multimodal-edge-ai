"""
SleepSense AI - Backend & API Unit Tests
"""

import sys
import os
import unittest
import json

# Add backend directory to sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "backend")))

from app import create_app
from database import Database

class BackendIntegrationTestCase(unittest.TestCase):
    def setUp(self):
        self.app = create_app()
        self.app.config['TESTING'] = True
        self.client = self.app.test_client()

    def test_health_endpoint(self):
        res = self.client.get('/api/v1/health')
        self.assertEqual(res.status_code, 200)
        data = json.loads(res.data)
        self.assertEqual(data['status'], 'healthy')

    def test_model_info_endpoint(self):
        res = self.client.get('/api/v1/model-info')
        self.assertEqual(res.status_code, 200)
        data = json.loads(res.data)
        self.assertEqual(data['version'], 'v2.1-Quantized-INT8')
        self.assertGreater(data['accuracy'], 0.95)

    def test_telemetry_ingest_and_query(self):
        payload = {
            "device_id": "ESP32-DEVKITV1-SLEEP-01",
            "timestamp": 1725880000,
            "sequence_number": 1,
            "audio": {
                "rms": 1200.0,
                "peak": 2800.0,
                "estimated_db": 58.5,
                "snore_probability": 0.94,
                "snore_detected": True,
                "quality": 0.95
            },
            "ppg": {
                "bpm": 72.0,
                "spo2": 96.5,
                "hrv": 38.0,
                "quality": 0.96
            },
            "posture": {
                "class": "LEFT_SIDE",
                "movement": 0.08,
                "quality": 0.95
            },
            "environment": {
                "temperature": 24.2,
                "humidity": 55.0,
                "quality": 0.98
            }
        }

        res = self.client.post('/api/v1/sensor-data',
                               data=json.dumps(payload),
                               content_type='application/json')
        self.assertEqual(res.status_code, 201)
        data = json.loads(res.data)
        self.assertEqual(data['status'], 'success')
        self.assertIn('fusion', data)

        # Test latest reading query
        res_latest = self.client.get('/api/v1/latest')
        self.assertEqual(res_latest.status_code, 200)
        latest = json.loads(res_latest.data)
        self.assertEqual(latest['device_id'], 'ESP32-DEVKITV1-SLEEP-01')

    def test_clinical_report_generation(self):
        res = self.client.get('/api/v1/report/SESSION-DEMO-001')
        self.assertEqual(res.status_code, 200)
        report = json.loads(res.data)
        self.assertIn('physiological_summary', report)
        self.assertIn('risk_assessment', report)
        self.assertIn('medical_disclaimer', report)

if __name__ == '__main__':
    unittest.main()
