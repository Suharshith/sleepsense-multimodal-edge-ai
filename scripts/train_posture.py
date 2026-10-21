"""
SleepSense AI - Posture Classifier Training Pipeline
Trains Decision Tree and 1D-CNN baselines for 5-class sleep posture categorization.
"""

import os
import numpy as np
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import accuracy_score, classification_report

def train_posture_model():
    print("\n=======================================================")
    print("  Training Posture Classifier Baselines")
    print("=======================================================")

    features_path = "data/features/posture_features.npy"
    labels_path = "data/features/posture_labels.npy"

    if not os.path.exists(features_path):
        from preprocess_posture import generate_posture_dataset
        X, y = generate_posture_dataset()
    else:
        X = np.load(features_path)
        y = np.load(labels_path)

    # 80/20 train/test split
    split = int(0.8 * len(X))
    X_train, y_train = X[:split], y[:split]
    X_test, y_test = X[split:], y[split:]

    clf = DecisionTreeClassifier(max_depth=5, random_state=42)
    clf.fit(X_train, y_train)

    preds = clf.predict(X_test)
    acc = accuracy_score(y_test, preds)

    print(f"[EVALUATION] Posture Model Test Accuracy: {acc*100:.2f}%\n")
    print("Classification Report:")
    print(classification_report(y_test, preds, target_names=["BACK", "LEFT", "RIGHT", "STOMACH", "MOVEMENT"]))

    os.makedirs("ml/models", exist_ok=True)
    import joblib
    joblib.dump(clf, "ml/models/posture_model.joblib")
    print("[INFO] Saved posture model to ml/models/posture_model.joblib\n")

if __name__ == "__main__":
    train_posture_model()
