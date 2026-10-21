"""
SleepSense AI - IMU Preprocessing & Sleep Posture Pipeline
Extracts 3D kinematics, angular orientation, and restlessness variance.
"""

import os
import numpy as np

# Posture Labels: 0: BACK (Supine), 1: LEFT_SIDE, 2: RIGHT_SIDE, 3: STOMACH (Prone), 4: MOVEMENT
POSTURE_LABELS = ["BACK", "LEFT_SIDE", "RIGHT_SIDE", "STOMACH", "MOVEMENT"]

def generate_posture_dataset(n_samples=1000, out_dir="data/features"):
    """
    Generates 6-axis IMU acceleration and gyroscope feature records for sleep posture classification.
    """
    os.makedirs(out_dir, exist_ok=True)
    np.random.seed(42)

    X = []
    y = []

    for _ in range(n_samples):
        posture_idx = np.random.choice([0, 1, 2, 3, 4], p=[0.35, 0.25, 0.25, 0.10, 0.05])
        
        # Ax, Ay, Az, Gx, Gy, Gz, Restlessness
        if posture_idx == 0:  # BACK (Supine: +Az dominant ~1.0g)
            ax = np.random.normal(0.02, 0.08)
            ay = np.random.normal(0.01, 0.08)
            az = np.random.normal(0.98, 0.06)
            gx = np.random.normal(0.0, 1.2)
            gy = np.random.normal(0.0, 1.2)
            gz = np.random.normal(0.0, 1.2)
            restless = np.random.uniform(0.01, 0.08)
        elif posture_idx == 1:  # LEFT_SIDE (+Ax dominant ~0.95g)
            ax = np.random.normal(0.95, 0.07)
            ay = np.random.normal(0.02, 0.08)
            az = np.random.normal(0.15, 0.10)
            gx = np.random.normal(0.0, 1.5)
            gy = np.random.normal(0.0, 1.5)
            gz = np.random.normal(0.0, 1.5)
            restless = np.random.uniform(0.01, 0.09)
        elif posture_idx == 2:  # RIGHT_SIDE (-Ax dominant ~-0.95g)
            ax = np.random.normal(-0.95, 0.07)
            ay = np.random.normal(0.02, 0.08)
            az = np.random.normal(0.15, 0.10)
            gx = np.random.normal(0.0, 1.5)
            gy = np.random.normal(0.0, 1.5)
            gz = np.random.normal(0.0, 1.5)
            restless = np.random.uniform(0.01, 0.09)
        elif posture_idx == 3:  # STOMACH (Prone: -Az dominant ~-0.98g)
            ax = np.random.normal(0.02, 0.08)
            ay = np.random.normal(0.01, 0.08)
            az = np.random.normal(-0.98, 0.06)
            gx = np.random.normal(0.0, 1.2)
            gy = np.random.normal(0.0, 1.2)
            gz = np.random.normal(0.0, 1.2)
            restless = np.random.uniform(0.01, 0.08)
        else:  # MOVEMENT (High acceleration variance & angular velocity)
            ax = np.random.normal(0.2, 0.5)
            ay = np.random.normal(0.3, 0.5)
            az = np.random.normal(0.5, 0.4)
            gx = np.random.normal(15.0, 25.0)
            gy = np.random.normal(18.0, 25.0)
            gz = np.random.normal(12.0, 20.0)
            restless = np.random.uniform(0.35, 0.85)

        mag = np.sqrt(ax**2 + ay**2 + az**2)
        feat = [ax, ay, az, gx, gy, gz, mag, restless]
        X.append(feat)
        y.append(posture_idx)

    X = np.array(X, dtype=np.float32)
    y = np.array(y, dtype=np.int64)

    np.save(os.path.join(out_dir, "posture_features.npy"), X)
    np.save(os.path.join(out_dir, "posture_labels.npy"), y)
    print(f"[INFO] Generated {len(X)} sleep posture feature vectors across {len(POSTURE_LABELS)} classes.")
    return X, y

if __name__ == "__main__":
    generate_posture_dataset()
