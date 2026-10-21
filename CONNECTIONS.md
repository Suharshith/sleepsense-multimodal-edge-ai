# Complete Hardware & Network Connection Guide (Pinout & Wiring)

### **Adaptive Multimodal Edge AI Sleep Apnea Monitoring System V2**
**Target Board:** **ESP32 DevKit V1** (ESP-WROOM-32 / 30-pin & 38-pin)

---

## 1. ESP32 DevKit V1 Master Pinout Connection Table

| ESP32 DevKit V1 Pin | Target Sensor | Sensor Pin | Protocol / Function | Voltage Level | Notes / Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **3V3 (3.3V)** | INMP441, MAX30102, MPU6050 | **VDD / VCC / VIN** | Power Supply | **+3.3V DC** | ⚠️ Connect ONLY to 3.3V (Do NOT use 5V/VIN) |
| **GND** | INMP441, MAX30102, MPU6050 | **GND** | Common Ground | **0V (GND)** | Shared common ground rail |
| **GPIO 14 (D14)** | INMP441 Mic | **SCK / BCLK** | I2S Bit Clock | 3.3V Digital | Serial Clock for digital audio |
| **GPIO 15 (D15)** | INMP441 Mic | **WS / LRCLK** | I2S Word Select | 3.3V Digital | Left/Right channel framing clock |
| **GPIO 32 (D32)** | INMP441 Mic | **SD / DOUT** | I2S Serial Data | 3.3V Digital | Digital microphone audio data stream |
| **GND** | INMP441 Mic | **L/R** | Channel Select | 0V (GND) | Hardwire to GND to select Left Channel |
| **GPIO 21 (D21)** | MAX30102 & MPU6050 | **SDA** | I2C Serial Data | 3.3V (Shared Bus) | Shared I2C Data line (0x57 & 0x68) |
| **GPIO 22 (D22)** | MAX30102 & MPU6050 | **SCL** | I2C Serial Clock | 3.3V (Shared Bus) | Shared I2C Clock line (400 kHz) |
| **GND** | MPU6050 IMU | **AD0** | I2C Address Pin | 0V (GND) | Hardwire to GND for default address `0x68` |

---

## 2. Sensor-by-Sensor Detailed Wiring

### A. INMP441 I2S MEMS Digital Microphone (Acoustic Breath Analysis)
```
       INMP441                           ESP32 DevKit V1
   +---------------+                    +-----------------+
   |           VDD |------------------->| 3V3 (3.3V Power)|
   |           GND |------------------->| GND             |
   |           SD  |------------------->| GPIO 32  (D32)  |
   |           WS  |------------------->| GPIO 15  (D15)  |
   |           SCK |------------------->| GPIO 14  (D14)  |
   |           L/R |------------------->| GND (Left Ch)   |
   +---------------+                    +-----------------+
```

### B. MAX30102 Pulse Oximeter & PPG Sensor (SpO2 % & Heart Rate BPM)
```
       MAX30102                          ESP32 DevKit V1
   +---------------+                    +-----------------+
   |           VIN |------------------->| 3V3 (3.3V Power)|
   |           GND |------------------->| GND             |
   |           SDA |------------------->| GPIO 21  (D21)  |
   |           SCL |------------------->| GPIO 22  (D22)  |
   |           INT |------------------->| NC (Not needed) |
   +---------------+                    +-----------------+
   Default I2C Address: 0x57
```

### C. MPU6050 6-Axis Motion Sensor (Sleep Posture & Movement Tracking)
```
       MPU6050                           ESP32 DevKit V1
   +---------------+                    +-----------------+
   |           VCC |------------------->| 3V3 (3.3V Power)|
   |           GND |------------------->| GND             |
   |           SDA |------------------->| GPIO 21  (D21)  |
   |           SCL |------------------->| GPIO 22  (D22)  |
   |           AD0 |------------------->| GND (Sets 0x68) |
   |           INT |------------------->| NC (Not needed) |
   +---------------+                    +-----------------+
   Default I2C Address: 0x68
```

---

## 3. Breadboard & Power Rail Distribution Diagram

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

---

## 4. Network & Server Connection Configuration

The Wi-Fi network and backend API destination settings are configured in `firmware/config.h`:

```cpp
// ==========================================
// WI-FI & FLASK BACKEND CONFIGURATION
// ==========================================
#define DEFAULT_WIFI_SSID     "Hello"
#define DEFAULT_WIFI_PASS     "123456789"
#define DEFAULT_SERVER_URL    "http://192.168.0.103:5000/api/v1/sensor-data"
#define DEFAULT_DEVICE_ID     "ESP32-DEVKITV1-SLEEP-01"
```

### Steps to establish the PC <-> ESP32 Connection:
1. **Verify your computer's local IPv4 address**:
   - Run `python check_connection.py` to confirm your local IP is `192.168.0.103`.
2. **Start the backend server on your PC**:
   - Double-click `run.bat` (or run `python backend/app.py`).
3. **Verify backend server reachability**:
   - Open your browser to `http://localhost:5000`.
4. **Firmware Configuration**:
   - The settings in `firmware/esp32/SleepSenseAI/config.h` are pre-configured for Wi-Fi SSID `Hello` and Password `123456789`.
5. **Flash the ESP32 DevKit V1**:
   - In Arduino IDE: Select Board **DOIT ESP32 DEVKIT V1** (or **ESP32 Dev Module**).
   - Port: Select your ESP32 COM Port.
   - Upload Baud Rate: `115200`.

---

## 5. Hardware Troubleshooting & Sanity Check

| Issue / Symptom | Possible Cause | Verification & Fix |
| :--- | :--- | :--- |
| `[MAX30102] Sensor not detected` | Loose I2C wiring or incorrect pin | Check SDA is on **GPIO 21** and SCL is on **GPIO 22**. |
| `[MPU6050] Sensor not responding` | AD0 pin floating | Hardwire the **AD0** pin of MPU6050 to **GND** to enforce address `0x68`. |
| `[INMP441] Failed to start I2S` | Pin mismatch | Ensure SCK = **GPIO 14**, WS = **GPIO 15**, SD = **GPIO 32**, and L/R = **GND**. |
| `[Wi-Fi] Timeout / Offline Buffer active` | 5GHz Wi-Fi network | ESP32 DevKit V1 only supports **2.4 GHz Wi-Fi**. Connect PC and ESP32 to the same 2.4 GHz network. |
| Board resets or sensors hot | Over-voltage (5V) | Disconnect immediately! Power all 3 modules strictly from the **3V3** pin. |
