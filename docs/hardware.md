# SleepSense AI - Hardware Pinout & Wiring Manual

### **Target Microcontroller: ESP32 DevKit V1 (ESP-WROOM-32 / 30-pin & 38-pin)**

---

## 1. Master Hardware Pinout Table (3-Sensor Tri-Modal Architecture)

| ESP32 DevKit V1 Pin | Connected Sensor | Sensor Pin | Protocol / Signal | Voltage | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **3V3 (3.3V)** | INMP441, MAX30102, MPU6050 | **VDD / VCC / VIN** | Power Supply | **+3.3V DC** | ⚠️ Connect ONLY to 3.3V (Never use 5V/VIN) |
| **GND** | INMP441, MAX30102, MPU6050 | **GND** | Common Ground | **0V (GND)** | Shared common ground rail |
| **GPIO 14 (D14)** | INMP441 Mic | **SCK / BCLK** | I2S Bit Clock | 3.3V Digital | Serial Clock for digital audio |
| **GPIO 15 (D15)** | INMP441 Mic | **WS / LRCLK** | I2S Word Select | 3.3V Digital | Left/Right frame sync clock |
| **GPIO 32 (D32)** | INMP441 Mic | **SD / DOUT** | I2S Serial Data | 3.3V Digital | Microphone audio stream input |
| **GND** | INMP441 Mic | **L/R** | Channel Select | 0V (GND) | Hardwire to GND for Left Channel |
| **GPIO 21 (D21)** | MAX30102 & MPU6050 | **SDA** | I2C Data Line | 3.3V (Shared Bus) | Shared I2C Data line |
| **GPIO 22 (D22)** | MAX30102 & MPU6050 | **SCL** | I2C Clock Line | 3.3V (Shared Bus) | Shared I2C Clock line (400 kHz) |
| **GND** | MPU6050 IMU | **AD0** | Address Select | 0V (GND) | Hardwire to GND for address `0x68` |

---

## 2. Sensor Specifications

1. **INMP441 I2S MEMS Microphone:**
   - Standard 24-bit/16-bit digital I2S output with bottom-port acoustic inlet.
   - Omnidirectional sensitivity ($-26\text{ dBFS}$ at $1\text{ kHz}, 94\text{ dB SPL}$).
   - Signal-to-noise ratio: $61\text{ dBA}$.
   - TinyML Audio Classification: 16 kHz Mono PCM $\to$ 13 Sub-band Log Energies.

2. **MAX30102 Pulse Oximeter & PPG Sensor:**
   - Integrated red ($660\text{ nm}$) and infrared ($880\text{ nm}$) LEDs with photodetector.
   - High-resolution 18-bit delta-sigma ADC sampling at $100\text{ Hz}$.
   - Hardware I2C address: `0x57`.
   - Vitals Output: Real-time SpO2 %, Heart Rate BPM, and RMSSD Heart Rate Variability.

3. **MPU6050 6-Axis Motion Sensor:**
   - 3-axis accelerometer ($\pm 2g$) and 3-axis gyroscope ($\pm 250^\circ/\text{s}$).
   - Hardware I2C address: `0x68` (AD0 tied to GND).
   - Posture Output: 3D gravitational orientation, restlessness score, and 6-class sleep posture categorization.

---

## 3. Circuit Schematics & Breadboard Layout

```
                                +-----------------------+
                                |    ESP32 DevKit V1    |
                                |                       |
                                | 3V3   GND   D21   D22 |
                                +--+-----+-----+-----+--+
                                   |     |     |     |
            +----------------------+     |     |     +----------------------+
            | 3.3V Power Rail            |     |                            |
            |                            |     | I2C SDA (D21)              | I2C SCL (D22)
            |     +----------------------+     |                            |
            |     | Common GND Rail            |                            |
            |     |                            |                            |
    +-------v-----v--------+             +-----v-----v--------+      +------v-----v--------+
    |   INMP441 (I2S)      |             |  MAX30102 (I2C)    |      |   MPU6050 (I2C)     |
    |                      |             |  (Address 0x57)    |      |   (Address 0x68)    |
    | VDD: 3.3V            |             | VIN: 3.3V          |      | VCC: 3.3V           |
    | GND: GND             |             | GND: GND           |      | GND: GND            |
    | SCK: GPIO 14 (D14)   |             | SDA: GPIO 21 (D21) |      | SDA: GPIO 21 (D21)  |
    | WS : GPIO 15 (D15)   |             | SCL: GPIO 22 (D22) |      | SCL: GPIO 22 (D22)  |
    | SD : GPIO 32 (D32)   |             +--------------------+      | AD0: GND            |
    | L/R: GND (Left Ch)   |                                         +---------------------+
    +----------------------+
```
