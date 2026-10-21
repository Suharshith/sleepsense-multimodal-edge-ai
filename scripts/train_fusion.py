"""
SleepSense AI - Adaptive Multimodal Fusion Evaluation & Training
Compares:
A. Audio-Only Baseline
B. Physiological-Only Baseline
C. Fixed Multimodal Fusion
D. Adaptive Quality-Aware Multimodal Fusion (Core Research Contribution)
"""

import os
import json
import numpy as np

def run_multimodal_fusion_experiment(n_samples=500):
    print("\n=======================================================")
    print("  Adaptive Multimodal Fusion Comparative Experiment")
    print("=======================================================")
    np.random.seed(42)

    # Generate synthetic multi-modal synchronized test windows
    labels = []
    audio_scores = []
    ppg_scores = []
    posture_scores = []
    env_scores = []

    audio_qualities = []
    ppg_qualities = []
    posture_qualities = []
    env_qualities = []

    for i in range(n_samples):
        # 30% True Apnea Episodes, 70% Normal Sleep
        is_apnea = np.random.choice([0, 1], p=[0.70, 0.30])
        labels.append(is_apnea)

        # Baseline Ground Truth Signal Contributions
        if is_apnea:
            a_score = np.random.normal(82.0, 10.0) # High snore / abnormal acoustic burden
            p_score = np.random.normal(88.0, 8.0)  # Significant SpO2 desaturation
            pos_score = np.random.normal(65.0, 15.0) # Supine exacerbation
            e_score = np.random.normal(20.0, 5.0)
        else:
            a_score = np.random.normal(15.0, 8.0)
            p_score = np.random.normal(12.0, 6.0)
            pos_score = np.random.normal(10.0, 5.0)
            e_score = np.random.normal(15.0, 5.0)

        # Inject realistic noise & sensor dropout scenarios in 20% of test data
        a_qual = np.random.uniform(0.90, 0.98)
        p_qual = np.random.uniform(0.90, 0.98)
        pos_qual = np.random.uniform(0.90, 0.98)
        e_qual = np.random.uniform(0.95, 0.99)

        if i % 5 == 0:  # Sensor Noise / Dropout Condition
            degradation_type = i % 3
            if degradation_type == 0:  # Optical PPG disconnected (finger slipped)
                p_qual = 0.0
                p_score = 0.0 # Missing
            elif degradation_type == 1: # High room fan noise (acoustic degradation)
                a_qual = 0.25
                a_score += 45.0 # False acoustic burst
            elif degradation_type == 2: # High restless tossing
                pos_qual = 0.40

        audio_scores.append(np.clip(a_score, 0.0, 100.0))
        ppg_scores.append(np.clip(p_score, 0.0, 100.0))
        posture_scores.append(np.clip(pos_score, 0.0, 100.0))
        env_scores.append(np.clip(e_score, 0.0, 100.0))

        audio_qualities.append(a_qual)
        ppg_qualities.append(p_qual)
        posture_qualities.append(pos_qual)
        env_qualities.append(e_qual)

    labels = np.array(labels)
    audio_scores = np.array(audio_scores)
    ppg_scores = np.array(ppg_scores)
    posture_scores = np.array(posture_scores)
    env_scores = np.array(env_scores)

    # ----------------------------------------------------
    # Model A: Audio-Only
    # ----------------------------------------------------
    preds_audio = (audio_scores > 50.0).astype(int)
    acc_audio = np.mean(preds_audio == labels)
    f1_audio = compute_f1(labels, preds_audio)

    # ----------------------------------------------------
    # Model B: Physiological-Only
    # ----------------------------------------------------
    preds_ppg = (ppg_scores > 50.0).astype(int)
    acc_ppg = np.mean(preds_ppg == labels)
    f1_ppg = compute_f1(labels, preds_ppg)

    # ----------------------------------------------------
    # Model C: Fixed Multimodal Fusion (Static Weights 0.35, 0.35, 0.15, 0.15)
    # ----------------------------------------------------
    fixed_scores = (0.35 * audio_scores) + (0.35 * ppg_scores) + (0.15 * posture_scores) + (0.15 * env_scores)
    preds_fixed = (fixed_scores > 45.0).astype(int)
    acc_fixed = np.mean(preds_fixed == labels)
    f1_fixed = compute_f1(labels, preds_fixed)

    # ----------------------------------------------------
    # Model D: Adaptive Signal-Quality Weighted Multimodal Fusion
    # ----------------------------------------------------
    adaptive_scores = []
    base_w = [0.35, 0.35, 0.15, 0.15]

    for j in range(n_samples):
        q = [audio_qualities[j], ppg_qualities[j], posture_qualities[j], env_qualities[j]]
        eff_w = [base_w[k] * q[k] for k in range(4)]
        sum_w = sum(eff_w)

        if sum_w > 0.05:
            norm_w = [w / sum_w for w in eff_w]
            fused = (norm_w[0] * audio_scores[j]) + (norm_w[1] * ppg_scores[j]) + \
                    (norm_w[2] * posture_scores[j]) + (norm_w[3] * env_scores[j])
        else:
            fused = 0.0 # Insufficient Data

        adaptive_scores.append(fused)

    adaptive_scores = np.array(adaptive_scores)
    preds_adaptive = (adaptive_scores > 45.0).astype(int)
    acc_adaptive = np.mean(preds_adaptive == labels)
    f1_adaptive = compute_f1(labels, preds_adaptive)

    results = {
        "experiment_name": "Multimodal Fusion Comparative Benchmark",
        "sample_size": n_samples,
        "models": {
            "Audio_Only": {"accuracy": round(float(acc_audio), 4), "f1_score": round(float(f1_audio), 4)},
            "Physiological_Only": {"accuracy": round(float(acc_ppg), 4), "f1_score": round(float(f1_ppg), 4)},
            "Fixed_Multimodal_Fusion": {"accuracy": round(float(acc_fixed), 4), "f1_score": round(float(f1_fixed), 4)},
            "Adaptive_Quality_Fusion": {"accuracy": round(float(acc_adaptive), 4), "f1_score": round(float(f1_adaptive), 4)}
        }
    }

    print("\n--- EXPERIMENTAL RESULTS ---")
    print(f"1. Audio-Only Baseline         : Acc = {acc_audio*100:.2f}% | F1 = {f1_audio:.4f}")
    print(f"2. Physiological-Only Baseline : Acc = {acc_ppg*100:.2f}% | F1 = {f1_ppg:.4f}")
    print(f"3. Fixed Multimodal Fusion     : Acc = {acc_fixed*100:.2f}% | F1 = {f1_fixed:.4f}")
    print(f"4. Adaptive Quality Fusion     : Acc = {acc_adaptive*100:.2f}% | F1 = {f1_adaptive:.4f}")
    print("----------------------------\n")

    os.makedirs("ml/evaluation", exist_ok=True)
    with open("ml/evaluation/fusion_benchmark_results.json", "w") as f:
        json.dump(results, f, indent=2)

    return results

def compute_f1(y_true, y_pred):
    tp = np.sum((y_true == 1) & (y_pred == 1))
    fp = np.sum((y_true == 0) & (y_pred == 1))
    fn = np.sum((y_true == 1) & (y_pred == 0))
    prec = tp / (tp + fp) if (tp + fp) > 0 else 0.0
    rec = tp / (tp + fn) if (tp + fn) > 0 else 0.0
    return (2 * prec * rec) / (prec + rec) if (prec + rec) > 0 else 0.0

if __name__ == "__main__":
    run_multimodal_fusion_experiment()
