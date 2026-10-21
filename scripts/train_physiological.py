"""
SleepSense AI - Physiological Feature Evaluation & Risk Benchmark
Trains physiological risk estimators and evaluates oxygen desaturation predictors.
"""

import os
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, f1_score, roc_auc_score

def train_physiological_model():
    print("\n=======================================================")
    print("  Evaluating Physiological Sleep Apnea Desaturation Model")
    print("=======================================================")

    features_path = "data/features/ppg_features.npy"
    labels_path = "data/features/ppg_labels.npy"

    if not os.path.exists(features_path):
        from preprocess_ppg import generate_physiological_dataset
        X, y = generate_physiological_dataset()
    else:
        X = np.load(features_path)
        y = np.load(labels_path)

    split = int(0.8 * len(X))
    X_train, y_train = X[:split], y[:split]
    X_test, y_test = X[split:], y[split:]

    rf = RandomForestClassifier(n_estimators=50, max_depth=4, random_state=42)
    rf.fit(X_train, y_train)

    preds = rf.predict(X_test)
    probs = rf.predict_proba(X_test)[:, 1]

    acc = accuracy_score(y_test, preds)
    f1 = f1_score(y_test, preds)
    auc = roc_auc_score(y_test, probs)

    print(f"[EVALUATION] Physiological Model Metrics:")
    print(f"  Accuracy : {acc*100:.2f}%")
    print(f"  F1-Score : {f1:.4f}")
    print(f"  ROC-AUC  : {auc:.4f}\n")

    os.makedirs("ml/models", exist_ok=True)
    import joblib
    joblib.dump(rf, "ml/models/ppg_model.joblib")
    print("[INFO] Saved physiological model to ml/models/ppg_model.joblib\n")

if __name__ == "__main__":
    train_physiological_model()
