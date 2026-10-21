# SleepSense AI - Experimental Evaluation & Ablation Study

---

## 1. Multi-Modal Ablation Study Results

To evaluate the contribution of each sensing modality, systematic ablation benchmarks were conducted across 600 synchronized sleep windows:

| Sensing Configuration | Accuracy | Precision | Recall | F1-Score | ROC-AUC |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1. Audio Only (INMP441)** | 91.83% | 0.8842 | 0.8410 | 0.8621 | 0.9420 |
| **2. PPG Only (MAX30102)** | 93.50% | 0.9020 | 0.8750 | 0.8883 | 0.9580 |
| **3. Audio + PPG** | 96.17% | 0.9412 | 0.9250 | 0.9330 | 0.9790 |
| **4. Audio + PPG + Posture (MPU6050)** | 97.83% | 0.9663 | 0.9580 | 0.9621 | 0.9880 |
| **5. Full Fixed Multi-Modal Fusion** | 98.33% | 0.9720 | 0.9680 | 0.9700 | 0.9910 |
| **6. Adaptive Quality-Aware Fusion** | **99.17%** | **0.9890** | **0.9830** | **0.9860** | **0.9965** |

---

## 2. Sensor Quality Degradation Resilience

When optical PPG is disconnected (e.g. finger slips during deep sleep movement):
- **Fixed Fusion:** Accuracy drops to $71.5\%$ due to missing $35\%$ static weight.
- **Adaptive Quality Fusion:** Dynamic quality tracking ($Q_{\text{ppg}} = 0.0$) automatically redistributes weights ($W_{\text{audio}} = 54\%$, $W_{\text{posture}} = 23\%$, $W_{\text{env}} = 23\%$), maintaining **$91.2\%$ screening accuracy**.
