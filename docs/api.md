# SleepSense AI - REST API Specification

Base URL: `http://<HOST>:5000/api/v1`

---

## 1. Endpoints Summary

| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `POST` | `/sensor-data` | Ingest unified multi-modal packet from ESP32 DevKit V1 |
| `GET` | `/latest` | Retrieve latest multi-modal reading and fused risk assessment |
| `GET` | `/history` | Query continuous telemetry timeline history (default 60 frames) |
| `GET` | `/events` | Retrieve verified snoring and oxygen desaturation events |
| `GET` | `/risk` | Query current composite sleep apnea risk index |
| `GET` | `/analytics` | Retrieve session statistics, desaturation counts, and posture distribution |
| `GET` | `/report/<session_id>` | Generate structured Polysomnography-style clinical screening report |
| `GET` | `/model-info` | Retrieve on-device TinyML model architecture, latency, and memory specs |
| `POST` | `/demo/seed` | Seed realistic 30-minute multi-modal sleep session for demonstration |
| `GET` | `/health` | System health check endpoint |

---

## 2. Telemetry Ingest Payload (`POST /sensor-data`)

```json
{
  "device_id": "ESP32-DEVKITV1-SLEEP-01",
  "timestamp": 1725880000,
  "sequence_number": 42,
  "audio": {
    "rms": 1200.0,
    "peak": 2800.0,
    "estimated_db": 58.5,
    "snore_probability": 0.94,
    "snore_detected": true,
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
```
