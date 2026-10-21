"""
SleepSense AI - Audio Classifier Training Pipeline
Trains a lightweight Neural Network on 13 Log-Mel spectral bands with Subject-Independent splits (70/15/15).
Exports weights for TinyML INT8 deployment on ESP32 DevKit V1.
"""

import os
import json
import numpy as np

def train_audio_model():
    print("\n=======================================================")
    print("  Training TinyML Audio Classifier (13 -> 8 -> 3)")
    print("=======================================================")

    features_path = "data/features/audio_features.npy"
    labels_path = "data/features/audio_labels.npy"
    subjects_path = "data/features/audio_subjects.npy"

    if not os.path.exists(features_path):
        from preprocess_audio import generate_synthetic_audio_dataset
        X, y, subjects = generate_synthetic_audio_dataset()
    else:
        X = np.load(features_path)
        y = np.load(labels_path)
        subjects = np.load(subjects_path)

    # Subject-Independent Split (Prevents data leakage)
    unique_subjs = np.unique(subjects)
    n_subjs = len(unique_subjs)
    
    train_subjs = unique_subjs[:int(0.70 * n_subjs)]
    val_subjs = unique_subjs[int(0.70 * n_subjs):int(0.85 * n_subjs)]
    test_subjs = unique_subjs[int(0.85 * n_subjs):]

    train_mask = np.isin(subjects, train_subjs)
    val_mask = np.isin(subjects, val_subjs)
    test_mask = np.isin(subjects, test_subjs)

    X_train, y_train = X[train_mask], y[train_mask]
    X_val, y_val = X[val_mask], y[val_mask]
    X_test, y_test = X[test_mask], y[test_mask]

    print(f"Dataset Splits (Subject-Independent):")
    print(f"  Train : {len(X_train)} samples ({len(train_subjs)} subjects)")
    print(f"  Val   : {len(X_val)} samples ({len(val_subjs)} subjects)")
    print(f"  Test  : {len(X_test)} samples ({len(test_subjs)} subjects)")

    # Model Weights (He Initialization)
    np.random.seed(42)
    input_dim = 13
    hidden_dim = 8
    output_dim = 3

    W1 = np.random.randn(input_dim, hidden_dim).astype(np.float32) * np.sqrt(2.0 / input_dim)
    B1 = np.zeros(hidden_dim, dtype=np.float32)
    W2 = np.random.randn(hidden_dim, output_dim).astype(np.float32) * np.sqrt(2.0 / hidden_dim)
    B2 = np.zeros(output_dim, dtype=np.float32)

    def softmax(z):
        ez = np.exp(z - np.max(z, axis=-1, keepdims=True))
        return ez / np.sum(ez, axis=-1, keepdims=True)

    def forward(x, w1, b1, w2, b2):
        h = np.maximum(0, np.dot(x, w1) + b1) # ReLU
        logits = np.dot(h, w2) + b2
        return h, softmax(logits)

    # Adam Training Loop
    lr = 0.012
    epochs = 400
    batch_size = 32

    mW1, vW1 = np.zeros_like(W1), np.zeros_like(W1)
    mB1, vB1 = np.zeros_like(B1), np.zeros_like(B1)
    mW2, vW2 = np.zeros_like(W2), np.zeros_like(W2)
    mB2, vB2 = np.zeros_like(B2), np.zeros_like(B2)
    beta1, beta2, eps = 0.9, 0.999, 1e-8

    t = 0
    for epoch in range(epochs):
        perm = np.random.permutation(len(X_train))
        X_shuff = X_train[perm]
        y_shuff = y_train[perm]

        for i in range(0, len(X_train), batch_size):
            xb = X_shuff[i:i+batch_size]
            yb = y_shuff[i:i+batch_size]

            h, probs = forward(xb, W1, B1, W2, B2)

            one_hot = np.zeros((len(yb), output_dim))
            one_hot[np.arange(len(yb)), yb] = 1.0

            dlogits = (probs - one_hot) / len(yb)
            dW2 = np.dot(h.T, dlogits)
            dB2 = np.sum(dlogits, axis=0)

            dh = np.dot(dlogits, W2.T)
            dh[h <= 0] = 0
            dW1 = np.dot(xb.T, dh)
            dB1 = np.sum(dh, axis=0)

            t += 1
            # Update W2, B2
            mW2 = beta1 * mW2 + (1 - beta1) * dW2
            vW2 = beta2 * vW2 + (1 - beta2) * (dW2 ** 2)
            W2 -= lr * (mW2 / (1 - beta1 ** t)) / (np.sqrt(vW2 / (1 - beta2 ** t)) + eps)

            mB2 = beta1 * mB2 + (1 - beta1) * dB2
            vB2 = beta2 * vB2 + (1 - beta2) * (dB2 ** 2)
            B2 -= lr * (mB2 / (1 - beta1 ** t)) / (np.sqrt(vB2 / (1 - beta2 ** t)) + eps)

            # Update W1, B1
            mW1 = beta1 * mW1 + (1 - beta1) * dW1
            vW1 = beta2 * vW1 + (1 - beta2) * (dW1 ** 2)
            W1 -= lr * (mW1 / (1 - beta1 ** t)) / (np.sqrt(vW1 / (1 - beta2 ** t)) + eps)

            mB1 = beta1 * mB1 + (1 - beta1) * dB1
            vB1 = beta2 * vB1 + (1 - beta2) * (dB1 ** 2)
            B1 -= lr * (mB1 / (1 - beta1 ** t)) / (np.sqrt(vB1 / (1 - beta2 ** t)) + eps)

    # Test Evaluation
    _, test_probs = forward(X_test, W1, B1, W2, B2)
    test_preds = np.argmax(test_probs, axis=-1)
    acc = np.mean(test_preds == y_test)

    print(f"\n[EVALUATION] Subject-Independent Test Accuracy: {acc*100:.2f}%")

    os.makedirs("ml/models", exist_ok=True)
    np.savez("ml/models/audio_model_weights.npz", W1=W1, B1=B1, W2=W2, B2=B2)
    print("[INFO] Model weights saved to ml/models/audio_model_weights.npz\n")

    return W1, B1, W2, B2

if __name__ == "__main__":
    train_audio_model()
