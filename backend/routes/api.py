"""
SleepSense AI - REST API Blueprints & Route Handlers
"""

from flask import Blueprint, request, jsonify
from datetime import datetime
from services.adaptive_fusion import adaptive_fusion
from services.analytics import AnalyticsService
from services.report_generator import ReportGenerator

api_bp = Blueprint('api', __name__, url_prefix='/api/v1')

def get_db():
    from flask import current_app
    return current_app.config['DB']

# ----------------------------------------------------
# 1. Telemetry Ingestion Endpoints (Supports POST & GET)
# ----------------------------------------------------
@api_bp.route('/sensor-data', methods=['GET', 'POST'])
@api_bp.route('/readings', methods=['GET', 'POST'])
def ingest_unified_sensor_data():
    """
    Primary endpoint for ESP32 DevKit V1 unified multi-modal telemetry.
    POST: Ingests real-time telemetry from ESP32, runs Adaptive Fusion, saves to DB.
    GET:  Returns endpoint health status, instructions, and latest reading summary.
    """
    db = get_db()

    if request.method == 'GET':
        latest = db.get_latest_reading()
        return jsonify({
            "status": "online",
            "endpoint": "/api/v1/sensor-data",
            "system": "SleepSense AI Telemetry Ingestion Gateway",
            "message": "Gateway is online and ready for ESP32 telemetry POST requests.",
            "protocol": "HTTP/1.1 POST application/json",
            "target_hardware": "ESP32 DevKit V1",
            "latest_reading": latest
        }), 200

    payload = request.get_json(silent=True)
    if not payload:
        return jsonify({"status": "error", "message": "Invalid JSON body"}), 400

    device_id = payload.get("device_id", payload.get("deviceId", "ESP32-DEVKITV1-SLEEP-01"))
    timestamp = payload.get("timestamp", int(datetime.utcnow().timestamp()))

    audio = payload.get("audio", {})
    ppg = payload.get("ppg", {})
    posture = payload.get("posture", {})
    env = payload.get("environment", {})

    # Execute Real-Time Adaptive Quality Fusion
    fusion_result = adaptive_fusion.evaluate(audio, ppg, posture, env)

    # Determine Audio Class
    audio_class = "Normal Breathing"
    if audio.get("snore_detected") or audio.get("class") == "Snoring" or audio.get("snore_probability", 0) > 0.65:
        audio_class = "Snoring"
    elif "Abnormal" in str(audio.get("class", "")):
        audio_class = "Abnormal Breathing / Apnea"

    reading_record = {
        "session_id": "SESSION-DEMO-001",
        "device_id": device_id,
        "timestamp": timestamp,
        "time_iso": datetime.utcnow().isoformat() + "Z",
        "audio_db": float(audio.get("estimated_db", audio.get("db", 35.0))),
        "audio_class": audio_class,
        "audio_quality": float(audio.get("quality", 0.95)),
        "snore_detected": bool(audio.get("snore_detected", False)),
        "spo2": float(ppg.get("spo2", 97.0)),
        "heart_rate": float(ppg.get("bpm", ppg.get("heart_rate", 68.0))),
        "hrv": float(ppg.get("hrv", 35.0)),
        "ppg_quality": float(ppg.get("quality", 0.95)),
        "posture": str(posture.get("class", posture.get("posture", "BACK"))),
        "movement": float(posture.get("movement", 0.1)),
        "posture_quality": float(posture.get("quality", 0.95)),
        "temperature": float(env.get("temperature", 24.5)),
        "humidity": float(env.get("humidity", 55.0)),
        "env_quality": float(env.get("quality", 0.98)),
        "risk_score": fusion_result["risk_score"],
        "risk_level": fusion_result["risk_level"],
        "confidence": fusion_result["confidence"]
    }

    db.insert_reading(reading_record)

    return jsonify({
        "status": "success",
        "timestamp": timestamp,
        "fusion": fusion_result
    }), 201

# ----------------------------------------------------
# 2. Latest & History Query Endpoints
# ----------------------------------------------------
@api_bp.route('/latest', methods=['GET'])
def get_latest():
    db = get_db()
    session_id = request.args.get("session_id", "SESSION-DEMO-001")
    latest = db.get_latest_reading(session_id)
    if not latest:
        return jsonify({
            "status": "no_data",
            "is_live": False,
            "message": "Waiting for ESP32 telemetry transmission..."
        }), 200
    
    # Calculate telemetry freshness in seconds
    now_epoch = int(datetime.utcnow().timestamp())
    reading_time = latest.get("timestamp", 0)
    age_seconds = max(0, now_epoch - reading_time)
    
    # Consider live if packet arrived within last 6 seconds (ESP32 transmits every 2s)
    latest["age_seconds"] = age_seconds
    latest["is_live"] = (age_seconds <= 6)
    latest["server_epoch"] = now_epoch
    return jsonify(latest)

@api_bp.route('/history', methods=['GET'])
def get_history():
    db = get_db()
    session_id = request.args.get("session_id", "SESSION-DEMO-001")
    limit = int(request.args.get("limit", 60))
    history = db.get_history(session_id, limit)
    return jsonify(history)

@api_bp.route('/events', methods=['GET'])
def get_events():
    db = get_db()
    session_id = request.args.get("session_id", "SESSION-DEMO-001")
    events = db.get_events(session_id)
    return jsonify(events)

@api_bp.route('/risk', methods=['GET'])
def get_risk():
    db = get_db()
    session_id = request.args.get("session_id", "SESSION-DEMO-001")
    latest = db.get_latest_reading(session_id)
    if not latest:
        return jsonify({
            "risk_score": 0.0,
            "risk_level": "INSUFFICIENT_DATA",
            "confidence": 0.0,
            "status_message": "Collecting baseline sleep signals..."
        })
    return jsonify({
        "risk_score": latest["risk_score"],
        "risk_level": latest["risk_level"],
        "confidence": latest["confidence"]
    })

# ----------------------------------------------------
# 3. Analytics & Clinical Report Endpoints
# ----------------------------------------------------
@api_bp.route('/analytics', methods=['GET'])
def get_analytics():
    db = get_db()
    session_id = request.args.get("session_id", "SESSION-DEMO-001")
    readings = db.get_history(session_id, limit=500)
    events = db.get_events(session_id, limit=200)
    analytics = AnalyticsService.compute_session_analytics(readings, events)
    return jsonify(analytics)

@api_bp.route('/report/<session_id>', methods=['GET'])
def get_report(session_id):
    db = get_db()
    readings = db.get_history(session_id, limit=500)
    events = db.get_events(session_id, limit=200)
    analytics = AnalyticsService.compute_session_analytics(readings, events)
    session_info = {"session_id": session_id, "patient_name": "Anonymous Patient (Research Participant)"}
    report = ReportGenerator.generate_clinical_report(session_info, analytics)
    return jsonify(report)

# ----------------------------------------------------
# 4. Model Registry & Health Endpoints
# ----------------------------------------------------
@api_bp.route('/model-info', methods=['GET'])
def get_model_info():
    return jsonify({
        "model_name": "SleepSense TinyML Audio CNN",
        "version": "v2.1-Quantized-INT8",
        "architecture": "Dense 13 -> 8 -> 3 (ReLU + Softmax)",
        "quantization": "INT8 Quantized C-Array (PROGMEM)",
        "accuracy": 0.992,
        "f1_score": 0.9922,
        "inference_latency_micros": 42,
        "ram_allocated_bytes": 128,
        "flash_footprint_bytes": 136,
        "target_hardware": "ESP32 DevKit V1 (ESP-WROOM-32 @ 160MHz)",
        "dataset": "Kaggle Snoring Dataset (16kHz Normalized)"
    })

@api_bp.route('/health', methods=['GET'])
def health_check():
    return jsonify({
        "status": "healthy",
        "system": "SleepSense AI Backend",
        "timestamp": datetime.utcnow().isoformat() + "Z"
    })
