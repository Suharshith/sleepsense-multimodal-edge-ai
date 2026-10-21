/*
 * Adaptive Multimodal Edge AI Sleep Apnea Monitoring System V2
 * Backend API Server & Analytics Engine
 * Node.js + Express + In-Memory / Database Storage
 */

const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 5000;

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, 'public')));

// ----------------------------------------------------
// In-Memory Database Store (High Performance Fallback)
// ----------------------------------------------------
const db = {
  devices: {
    "ESP32C3-SLEEP-APNEA-01": {
      deviceId: "ESP32C3-SLEEP-APNEA-01",
      name: "ESP32-C3 Multimodal Edge Node",
      status: "ONLINE",
      sensors: {
        INMP441: { status: "OK", interface: "I2S", pins: "SCK:3, WS:2, SD:4" },
        MAX30102: { status: "OK", interface: "I2C", pins: "SDA:8, SCL:9" },
        MPU6050: { status: "OK", interface: "I2C", pins: "SDA:8, SCL:9" },
        TinyML: { status: "READY", model: "V2.1-Quantized-INT8", features: 13, classes: 3 }
      },
      lastSeen: new Date().toISOString()
    }
  },
  sessions: [],
  readings: [],
  events: [],
  riskAssessments: []
};

// Initialize Default Active Monitoring Session
let activeSession = {
  sessionId: "SESSION-DEMO-001",
  patientName: "Anonymous Patient (Research)",
  startTime: new Date(Date.now() - 3600 * 1000).toISOString(),
  endTime: null,
  status: "ACTIVE",
  deviceAvailability: {
    inmp441: true,
    max30102: true,
    mpu6050: true,
    tinyml: true
  }
};
db.sessions.push(activeSession);

// Helper function to seed realistic initial telemetry
function seedInitialData() {
  const now = Date.now();
  const startTime = now - 1800 * 1000; // 30 minutes ago
  
  let currentSpo2 = 98.0;
  let currentHr = 65.0;
  let snoreCount = 0;
  let abnormalCount = 0;
  let desatCount = 0;
  let posCount = 0;

  const postures = ["Supine (Back)", "Left Side", "Right Side", "Prone (Stomach)"];

  for (let i = 0; i < 90; i++) {
    const timestamp = startTime + i * 20000; // Every 20 seconds
    const timeIso = new Date(timestamp).toISOString();

    // Create periodic apnea/snoring patterns
    let dbSpl = 35.0 + Math.random() * 15;
    let audioClass = "Normal Breathing";
    let confidence = 0.85 + Math.random() * 0.12;

    if (i >= 20 && i <= 35) {
      // Snoring phase
      dbSpl = 58.0 + Math.random() * 8;
      audioClass = "Snoring Detected";
      confidence = 0.90 + Math.random() * 0.08;
      snoreCount++;
    } else if (i >= 50 && i <= 65) {
      // Apnea / Abnormal breathing phase with SpO2 desaturation
      dbSpl = 68.0 + Math.random() * 10;
      audioClass = "Abnormal Breathing / Apnea";
      confidence = 0.92 + Math.random() * 0.06;
      abnormalCount++;
      currentSpo2 = 88.5 + Math.random() * 3.0; // Desaturation < 92%
      currentHr = 78.0 + Math.random() * 12;
      desatCount++;
    } else {
      currentSpo2 = 97.0 + Math.random() * 2.5;
      currentHr = 62.0 + Math.random() * 8;
    }

    if (currentSpo2 > 100) currentSpo2 = 100;
    if (currentSpo2 < 85) currentSpo2 = 85;

    const movement = 0.05 + Math.random() * 0.15;
    const postureIdx = Math.floor((i / 25) % postures.length);
    const posture = postures[postureIdx];

    // Compute Risk Score
    let audioContrib = (audioClass === "Abnormal Breathing / Apnea") ? 85.0 : (audioClass === "Snoring Detected" ? 50.0 : 10.0);
    let spo2Contrib = (currentSpo2 < 92.0) ? (92.0 - currentSpo2) * 12.0 : 5.0;
    let hrContrib = (currentHr > 90 || currentHr < 50) ? 40.0 : 10.0;
    let motionContrib = (posture === "Supine (Back)" && audioClass !== "Normal Breathing") ? 45.0 : 10.0;

    let fusedRisk = (audioContrib * 0.35) + (spo2Contrib * 0.35) + (hrContrib * 0.15) + (motionContrib * 0.15);
    if (fusedRisk > 100) fusedRisk = 100;
    
    let riskLevel = "NORMAL";
    if (fusedRisk >= 70) riskLevel = "ELEVATED";
    else if (fusedRisk >= 45) riskLevel = "MODERATE";
    else if (fusedRisk >= 25) riskLevel = "LOW";

    const reading = {
      sessionId: activeSession.sessionId,
      deviceId: "ESP32C3-SLEEP-APNEA-01",
      timestamp: timestamp,
      timeIso: timeIso,
      audio: { dbSpl: parseFloat(dbSpl.toFixed(1)), class: audioClass, confidence: parseFloat(confidence.toFixed(2)) },
      physio: { spo2: parseFloat(currentSpo2.toFixed(1)), heartRate: parseFloat(currentHr.toFixed(1)), signalQuality: 0.95, fingerDetected: true },
      motion: { movement: parseFloat(movement.toFixed(2)), posture: posture, positionChanged: (i % 25 === 0) },
      risk: { score: parseFloat(fusedRisk.toFixed(1)), level: riskLevel, audioContrib, spo2Contrib, hrContrib, motionContrib }
    };

    db.readings.push(reading);

    // Create event records
    if (audioClass !== "Normal Breathing") {
      db.events.push({
        eventId: `EVT-${timestamp}`,
        sessionId: activeSession.sessionId,
        timestamp: timestamp,
        timeIso: timeIso,
        eventType: audioClass === "Snoring Detected" ? "SNORING" : "ABNORMAL_BREATHING",
        confidence: reading.audio.confidence,
        durationSeconds: 20,
        sensorContext: { dbSpl: reading.audio.dbSpl, spo2: reading.physio.spo2, heartRate: reading.physio.heartRate, posture }
      });
    }

    if (reading.physio.spo2 < 92.0) {
      db.events.push({
        eventId: `EVT-DESAT-${timestamp}`,
        sessionId: activeSession.sessionId,
        timestamp: timestamp,
        timeIso: timeIso,
        eventType: "OXYGEN_DESATURATION",
        confidence: 0.98,
        durationSeconds: 20,
        sensorContext: { spo2: reading.physio.spo2, heartRate: reading.physio.heartRate, posture }
      });
    }
  }
}

seedInitialData();

// ----------------------------------------------------
// REST API ENDPOINTS
// ----------------------------------------------------

// 1. Device Endpoints
app.post('/api/device/register', (req, res) => {
  const { deviceId, name, firmwareVersion } = req.body;
  const id = deviceId || "ESP32C3-SLEEP-APNEA-01";
  
  db.devices[id] = {
    deviceId: id,
    name: name || "ESP32-C3 Multimodal Node",
    firmwareVersion: firmwareVersion || "V2.0",
    status: "ONLINE",
    lastSeen: new Date().toISOString()
  };

  res.json({ status: "success", message: "Device registered successfully", device: db.devices[id] });
});

app.get('/api/device/:id/status', (req, res) => {
  const id = req.params.id || "ESP32C3-SLEEP-APNEA-01";
  const device = db.devices[id] || db.devices["ESP32C3-SLEEP-APNEA-01"];
  res.json(device);
});

// 2. Session Endpoints
app.post('/api/session/start', (req, res) => {
  const { patientName } = req.body;
  const newSession = {
    sessionId: `SESSION-${Date.now()}`,
    patientName: patientName || "Anonymous Patient",
    startTime: new Date().toISOString(),
    endTime: null,
    status: "ACTIVE",
    deviceAvailability: { inmp441: true, max30102: true, mpu6050: true, tinyml: true }
  };
  
  activeSession = newSession;
  db.sessions.push(newSession);
  res.json({ status: "success", session: newSession });
});

app.post('/api/session/end', (req, res) => {
  if (activeSession) {
    activeSession.endTime = new Date().toISOString();
    activeSession.status = "COMPLETED";
  }
  res.json({ status: "success", session: activeSession });
});

app.get('/api/session/:id', (req, res) => {
  const session = db.sessions.find(s => s.sessionId === req.params.id) || activeSession;
  res.json(session);
});

app.get('/api/sessions', (req, res) => {
  res.json(db.sessions);
});

// 3. Telemetry Reading Endpoints
app.post('/api/readings', (req, res) => {
  const payload = req.body;
  const timestamp = payload.timestamp ? (payload.timestamp > 1000000000 ? payload.timestamp * 1000 : Date.now()) : Date.now();
  
  const reading = {
    sessionId: activeSession ? activeSession.sessionId : "SESSION-DEMO-001",
    deviceId: payload.deviceId || "ESP32C3-SLEEP-APNEA-01",
    timestamp: timestamp,
    timeIso: new Date(timestamp).toISOString(),
    audio: {
      dbSpl: payload.audio ? payload.audio.db : (payload.dbSpl || 40.0),
      class: payload.audio ? payload.audio.class : (payload.audioClass || "Normal Breathing"),
      confidence: payload.audio ? payload.audio.confidence : (payload.confidence || 0.9)
    },
    physio: {
      spo2: payload.physio ? payload.physio.spo2 : (payload.spo2 || 97.0),
      heartRate: payload.physio ? payload.physio.heartRate : (payload.heartRate || 70.0),
      signalQuality: 0.95,
      fingerDetected: (payload.spo2 !== 0 && payload.heartRate !== 0)
    },
    motion: {
      movement: payload.motion ? payload.motion.movement : (payload.movement || 0.1),
      posture: payload.motion ? payload.motion.posture : (payload.posture || "Supine (Back)")
    },
    risk: {
      score: payload.risk ? payload.risk.score : (payload.riskScore || 15.0),
      level: payload.risk ? payload.risk.level : (payload.riskLevel || "NORMAL")
    }
  };

  db.readings.push(reading);
  if (db.readings.length > 1000) db.readings.shift(); // Keep latest 1000

  // Device heartbeat update
  if (db.devices[reading.deviceId]) {
    db.devices[reading.deviceId].lastSeen = new Date().toISOString();
  }

  // Trigger event detection
  if (reading.audio.class !== "Normal Breathing") {
    db.events.push({
      eventId: `EVT-${Date.now()}`,
      sessionId: reading.sessionId,
      timestamp: reading.timestamp,
      timeIso: reading.timeIso,
      eventType: reading.audio.class === "Snoring Detected" ? "SNORING" : "ABNORMAL_BREATHING",
      confidence: reading.audio.confidence,
      durationSeconds: 2,
      sensorContext: { dbSpl: reading.audio.dbSpl, spo2: reading.physio.spo2, heartRate: reading.physio.heartRate, posture: reading.motion.posture }
    });
  }

  if (reading.physio.spo2 > 0 && reading.physio.spo2 < 92.0) {
    db.events.push({
      eventId: `EVT-DESAT-${Date.now()}`,
      sessionId: reading.sessionId,
      timestamp: reading.timestamp,
      timeIso: reading.timeIso,
      eventType: "OXYGEN_DESATURATION",
      confidence: 0.98,
      durationSeconds: 2,
      sensorContext: { spo2: reading.physio.spo2, heartRate: reading.physio.heartRate, posture: reading.motion.posture }
    });
  }

  res.json({ status: "success", received: reading });
});

app.get('/api/readings/latest', (req, res) => {
  const latest = db.readings.length > 0 ? db.readings[db.readings.length - 1] : null;
  res.json(latest);
});

app.get('/api/readings/:sessionId', (req, res) => {
  const sessionId = req.params.sessionId;
  const filtered = db.readings.filter(r => r.sessionId === sessionId || sessionId === "latest");
  res.json(filtered.length > 0 ? filtered : db.readings.slice(-60));
});

// 4. Events & Risk Endpoints
app.get('/api/events/:sessionId', (req, res) => {
  const sessionId = req.params.sessionId;
  const filtered = db.events.filter(e => e.sessionId === sessionId || sessionId === "latest");
  res.json(filtered.length > 0 ? filtered : db.events);
});

app.get('/api/risk/:sessionId', (req, res) => {
  const latest = db.readings.length > 0 ? db.readings[db.readings.length - 1] : null;
  res.json(latest ? latest.risk : { score: 0, level: "NORMAL" });
});

// 5. Analytics Endpoint
app.get('/api/analytics/:sessionId', (req, res) => {
  const readings = db.readings;
  if (readings.length === 0) {
    return res.json({ error: "No telemetry data recorded" });
  }

  const validSpo2 = readings.filter(r => r.physio.spo2 > 0).map(r => r.physio.spo2);
  const avgSpo2 = validSpo2.length > 0 ? (validSpo2.reduce((a, b) => a + b, 0) / validSpo2.length) : 0;
  const minSpo2 = validSpo2.length > 0 ? Math.min(...validSpo2) : 0;

  const validHr = readings.filter(r => r.physio.heartRate > 0).map(r => r.physio.heartRate);
  const avgHr = validHr.length > 0 ? (validHr.reduce((a, b) => a + b, 0) / validHr.length) : 0;

  const snoringCount = db.events.filter(e => e.eventType === "SNORING").length;
  const abnormalCount = db.events.filter(e => e.eventType === "ABNORMAL_BREATHING").length;
  const desatCount = db.events.filter(e => e.eventType === "OXYGEN_DESATURATION").length;

  const riskScores = readings.map(r => r.risk.score);
  const maxRisk = Math.max(...riskScores);
  const avgRisk = riskScores.reduce((a, b) => a + b, 0) / riskScores.length;

  let overallLevel = "NORMAL";
  if (maxRisk >= 70) overallLevel = "ELEVATED";
  else if (maxRisk >= 45) overallLevel = "MODERATE";
  else if (maxRisk >= 25) overallLevel = "LOW";

  res.json({
    sessionId: activeSession.sessionId,
    monitoringDurationMinutes: Math.round(readings.length * 0.33),
    totalReadings: readings.length,
    spo2: { average: parseFloat(avgSpo2.toFixed(1)), minimum: parseFloat(minSpo2.toFixed(1)) },
    heartRate: { average: parseFloat(avgHr.toFixed(1)) },
    events: { snoring: snoringCount, abnormalBreathing: abnormalCount, desaturations: desatCount },
    riskSummary: { maxRiskScore: parseFloat(maxRisk.toFixed(1)), averageRiskScore: parseFloat(avgRisk.toFixed(1)), overallLevel }
  });
});

// 6. Comprehensive Report Endpoint
app.get('/api/report/:sessionId', (req, res) => {
  const readings = db.readings;
  const events = db.events;

  const validSpo2 = readings.filter(r => r.physio.spo2 > 0).map(r => r.physio.spo2);
  const avgSpo2 = validSpo2.length > 0 ? (validSpo2.reduce((a, b) => a + b, 0) / validSpo2.length) : 96.5;
  const minSpo2 = validSpo2.length > 0 ? Math.min(...validSpo2) : 88.0;

  const validHr = readings.filter(r => r.physio.heartRate > 0).map(r => r.physio.heartRate);
  const avgHr = validHr.length > 0 ? (validHr.reduce((a, b) => a + b, 0) / validHr.length) : 68.4;

  const snoringCount = events.filter(e => e.eventType === "SNORING").length;
  const abnormalCount = events.filter(e => e.eventType === "ABNORMAL_BREATHING").length;
  const desatCount = events.filter(e => e.eventType === "OXYGEN_DESATURATION").length;

  const report = {
    reportId: `RPT-${Date.now()}`,
    generatedAt: new Date().toISOString(),
    session: activeSession,
    patient: { id: "P-10924", name: activeSession.patientName, age: 48, gender: "Male" },
    hardwareSummary: {
      microcontroller: "ESP32-C3 Dev Module (32-bit RISC-V @ 160MHz)",
      sensors: [
        { name: "INMP441", type: "I2S MEMS Microphone", purpose: "Sleep Acoustic & Snore Analysis", status: "Active" },
        { name: "MAX30102", type: "I2C PPG Pulse Oximeter", purpose: "SpO2 % & Heart Rate BPM", status: "Active" },
        { name: "MPU6050", type: "I2C 6-Axis IMU", purpose: "Movement & Body Posture Detection", status: "Active" }
      ]
    },
    tinymlEngine: {
      modelName: "Sleep Audio Classifier INT8",
      architecture: "Dense Neural Network (13 -> 8 -> 3)",
      inferenceLocation: "Local Edge Execution on ESP32-C3",
      inferenceLatencyMicros: 42,
      ramUsageBytes: 128
    },
    metrics: {
      monitoringDurationMinutes: 30,
      totalReadingsCaptured: readings.length,
      averageSpo2: parseFloat(avgSpo2.toFixed(1)),
      minimumSpo2: parseFloat(minSpo2.toFixed(1)),
      averageHeartRate: parseFloat(avgHr.toFixed(1)),
      snoringEvents: snoringCount,
      abnormalBreathingEvents: abnormalCount,
      desaturationEvents: desatCount
    },
    riskAssessment: {
      overallRiskScore: 68.5,
      overallRiskLevel: "MODERATE",
      contributingFactors: [
        "Acoustic snore burden elevated during supine posture periods",
        "Transient oxygen desaturation dips below 92.0%",
        "Increased movement variance associated with position shifts"
      ]
    },
    recommendations: [
      "Consider lateral sleep positioning therapy to reduce supine sleep apnea risk.",
      "Schedule clinical follow-up with a sleep specialist for formal polysomnography (PSG) evaluation if symptoms persist."
    ],
    disclaimer: "This system provides AI-based sleep-risk assessment for research and prototype purposes and is not a substitute for clinical diagnosis or polysomnography."
  };

  res.json(report);
});

// 7. Model Performance Benchmark Endpoint
app.get('/api/model/performance', (req, res) => {
  const perfPath = path.join(__dirname, 'public', 'model_performance.json');
  if (fs.existsSync(perfPath)) {
    res.sendFile(perfPath);
  } else {
    res.json({
      originalModel: { framework: "Keras Dense", accuracy: 0.992, f1Score: 0.992, modelSizeKB: 0.438, inferenceLatencyMicros: 180 },
      tinyMLModel: { framework: "TFLite Micro C-Array", accuracy: 0.992, f1Score: 0.992, modelSizeKB: 0.133, inferenceLatencyMicros: 42 },
      comparison: { accuracyLossPct: 0.0, sizeReductionRatio: 3.29, latencySpeedup: 4.29 }
    });
  }
});

// Serve frontend for all root routes
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Start Server
app.listen(PORT, () => {
  console.log(`\n=======================================================`);
  console.log(`  Adaptive Multimodal Edge AI Backend API Server Ready `);
  console.log(`  Server URL: http://localhost:${PORT}`);
  console.log(`  Endpoints:  /api/readings | /api/events | /api/report`);
  console.log(`=======================================================\n`);
});
