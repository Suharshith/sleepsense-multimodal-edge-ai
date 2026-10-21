# SleepSense AI - System Architecture

### **Adaptive Multi-Modal Edge AI Framework for Sleep Apnea Risk Assessment**

---

## 1. System Overview & Architecture Diagram

SleepSense AI is an end-to-end, multi-tier healthcare IoT platform combining embedded edge intelligence with web-based real-time telemetry analytics.

```
+-----------------------------------------------------------------------------------------------+
|                            EDGE SENSING TIER (ESP32 DevKit V1 @ 160MHz)                       |
|                                                                                               |
|   +--------------------+   +--------------------+   +--------------------+   +-------------+  |
|   |  INMP441 (I2S)     |   |  MAX30102 (I2C)    |   |  MPU6050 (I2C)     |   | DHT22 (GPIO)|  |
|   |  16 kHz Mono Audio |   |  Red & IR PPG      |   |  50Hz 6-Axis IMU   |   | Temp / Hum  |  |
|   +---------+----------+   +---------+----------+   +---------+----------+   +------+------+  |
|             |                        |                        |                     |         |
|             v                        v                        v                     v         |
|   +--------------------+   +--------------------+   +--------------------+   +-------------+  |
|   | 13-Band Log-Mel    |   | SpO2 & HR Tracker  |   | 3D Posture Model   |   | Comfort Idx |  |
|   | TinyML Audio CNN   |   | PPG SQI [0, 1]     |   | Restlessness Score |   | Env Quality |  |
|   +---------+----------+   +---------+----------+   +---------+----------+   +------+------+  |
|             |                        |                        |                     |         |
|             +------------------------+-----------+------------+---------------------+         |
|                                                  |                                            |
|                                                  v                                            |
|                                   +------------------------------+                            |
|                                   | Unified Telemetry Serializer |                            |
|                                   +--------------+---------------+                            |
|                                                  |                                            |
|                                                  v                                            |
|                                   +------------------------------+                            |
|                                   | Wi-Fi & Offline Ring Buffer  |                            |
|                                   | (30 Packets Non-blocking)    |                            |
|                                   +--------------+---------------+                            |
+--------------------------------------------------|--------------------------------------------+
                                                   | HTTP POST (JSON / REST)
                                                   v
+-----------------------------------------------------------------------------------------------+
|                               BACKEND ANALYTICS TIER (Flask / Python)                         |
|                                                                                               |
|   - Adaptive Multimodal Quality-Aware Fusion Engine                                           |
|   - Relational SQLite / PostgreSQL Telemetry Database                                         |
|   - Polysomnography-Style Automated Clinical Screening Report Generator                       |
+--------------------------------------------------|--------------------------------------------+
                                                   | REST API / Static Assets
                                                   v
+-----------------------------------------------------------------------------------------------+
|                            CONSUMER & CLINICAL WEB DASHBOARD (HTML/CSS/JS)                    |
|                                                                                               |
|   - Overview: Prominent Multi-Modal Risk Gauge (0-100), Status Messages, AI Insights          |
|   - Live Monitor: Synchronized Dual Timelines (SpO2, BPM, Acoustic dB, Posture, Risk)        |
|   - Snoring Analysis & Sleep Posture Kinematics Visualizer                                    |
|   - One-Click Printable Clinical Polysomnography Summary Report                               |
|   - Technical Inspector: TinyML Benchmark, Memory Footprints, Raw Waveform Vectors            |
+-----------------------------------------------------------------------------------------------+
```

---

## 2. Adaptive Quality-Aware Multimodal Fusion (Mathematical Formulation)

The primary research contribution of SleepSense AI is the dynamic weighting of multi-modal sensory inputs based on real-time **Signal Quality Indices ($Q_i \in [0, 1]$)**.

### A. Modality Sub-Score Calculations
1. **Acoustic Burden Sub-Score ($S_{\text{audio}}$):**
   $$S_{\text{audio}} = \begin{cases} 95.0 \cdot C_{\text{audio}} & \text{if Apnea/Abnormal sound} \\ 55.0 \cdot C_{\text{audio}} & \text{if Snoring} \\ 10.0 & \text{if Normal Breathing} \end{cases}$$
   *(with acoustic intensity adjustment for sound pressure levels $> 65\text{ est. dB}$)*.

2. **Physiological Desaturation Sub-Score ($S_{\text{ppg}}$):**
   $$S_{\text{ppg}} = \begin{cases} 30.0 + \min(70.0, (92.0 - \text{SpO}_2) \cdot 14.0) & \text{if } \text{SpO}_2 < 92.0\% \\ 5.0 + \text{HR\_Instability} & \text{otherwise} \end{cases}$$

3. **Postural Exacerbation Sub-Score ($S_{\text{posture}}$):**
   $$S_{\text{posture}} = 10.0 + \mathbf{1}_{\text{Supine}} \cdot (25.0 + 20.0 \cdot \mathbf{1}_{\text{Snoring}}) + \min(35.0, \text{Restlessness} \cdot 50.0)$$

4. **Environmental Context Sub-Score ($S_{\text{env}}$):**
   $$S_{\text{env}} = 10.0 + \mathbf{1}_{\text{Temp/Hum Out of Range}} \cdot 15.0$$

### B. Dynamic Quality Weight Normalization
Effective modality weights are computed dynamically as:
$$w_i = Q_i \cdot \text{base\_weight}_i \quad \text{where } \text{base\_weight} = [0.35, 0.35, 0.15, 0.15]$$

Normalized weights:
$$W_i = \frac{w_i}{\sum_{j=1}^4 w_j}$$

The composite **Sleep Apnea Risk Index ($R \in [0, 100]$)**:
$$R = W_{\text{audio}} S_{\text{audio}} + W_{\text{ppg}} S_{\text{ppg}} + W_{\text{posture}} S_{\text{posture}} + W_{\text{env}} S_{\text{env}}$$

If $\sum_{j=1}^4 w_j < 0.15$, the system safely outputs `risk_level: "INSUFFICIENT_DATA"` rather than fabricating misleading risk metrics.
