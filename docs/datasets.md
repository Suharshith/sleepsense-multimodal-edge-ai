# SleepSense AI - Datasets & Validation Protocol

---

## 1. Research Datasets Registry

| Dataset Name | Source / Repository | Modality | Sampling Rate | Subjects / Samples | Purpose |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Kaggle Snoring Dataset** | [Kaggle](https://www.kaggle.com/datasets/tareqkhan/snoring-dataset) | Acoustic (Mono PCM) | 16 kHz | 1,000 one-second WAV clips | TinyML Snore Classifier training |
| **Apnea-ECG Database** | [PhysioNet](https://physionet.org/content/apnea-ecg/1.0.0/) | ECG, Respiration, SpO2 | 100 Hz / 1 Hz | 70 Polysomnograms (35 OSA, 35 control) | Heart Rate Variability & Apnea validation |
| **Sleep Heart Health Study (SHHS)** | [NSRR](https://sleepdata.org/datasets/shhs) | Multi-channel PSG | 1 Hz / 125 Hz | 5,804 Participants | SpO2 desaturation duration & AHI correlation |
| **UCI HAR / Sleep Posture** | [UCI Repository](https://archive.ics.uci.edu/dataset/240/) | 6-Axis IMU (Accel + Gyro) | 50 Hz | 30 Volunteers (10,299 instances) | Baseline sleep posture & restlessness model |
| **SleepSense Custom Test Bench** | Hardware Lab | Multi-modal Synchronized | 16kHz / 100Hz / 50Hz | Continuous Multi-session bench recordings | On-device TinyML & Sensor Quality calibration |

---

## 2. Subject-Independent Validation Protocol

To prevent **data leakage**, recordings from the same subject are strictly isolated:
- **Training Set (70% of subjects):** Used exclusively for weight optimization.
- **Validation Set (15% of subjects):** Used for hyperparameter tuning and early stopping.
- **Test Set (15% of subjects):** Held-out for final generalization evaluation.

Cross-validation across subjects guarantees that the TinyML neural network learns invariant acoustic formant representations rather than memorizing individual vocal tract or microphone characteristics.
