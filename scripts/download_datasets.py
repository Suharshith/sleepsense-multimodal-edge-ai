"""
SleepSense AI - Dataset Downloader & Research Registry Manager
Validates and initializes legally accessible public datasets for SleepSense AI.
"""

import os
import json
import urllib.request

REGISTRY_PATH = os.path.join(os.path.dirname(__file__), "..", "datasets", "dataset_registry.json")
DATA_DIR = os.path.join(os.path.dirname(__file__), "..", "data")

def init_directories():
    os.makedirs(os.path.join(DATA_DIR, "raw", "audio"), exist_ok=True)
    os.makedirs(os.path.join(DATA_DIR, "raw", "ppg"), exist_ok=True)
    os.makedirs(os.path.join(DATA_DIR, "raw", "posture"), exist_ok=True)
    os.makedirs(os.path.join(DATA_DIR, "raw", "environment"), exist_ok=True)
    os.makedirs(os.path.join(DATA_DIR, "processed"), exist_ok=True)
    os.makedirs(os.path.join(DATA_DIR, "features"), exist_ok=True)
    os.makedirs(os.path.join(DATA_DIR, "labels"), exist_ok=True)
    print(f"[INFO] Initialized local data storage under: {DATA_DIR}")

def print_registry():
    if not os.path.exists(REGISTRY_PATH):
        print(f"[ERROR] Registry file not found at {REGISTRY_PATH}")
        return

    with open(REGISTRY_PATH, "r") as f:
        registry = json.load(f)

    print("\n=======================================================")
    print(f"  SleepSense AI - Research Dataset Registry (v{registry.get('version', '1.0')})")
    print("=======================================================\n")

    for idx, ds in enumerate(registry.get("dataset_registry", []), start=1):
        print(f"[{idx}] {ds['name']}")
        print(f"    Source       : {ds['source']} ({ds['url']})")
        print(f"    Modality     : {ds['modality']}")
        print(f"    License      : {ds['license']}")
        print(f"    Sample Stats : {ds['samples']} @ {ds['sampling_rate']}")
        print(f"    Labels       : {', '.join(ds['labels'])}")
        print(f"    Purpose      : {ds['purpose']}")
        print(f"    Citation     : {ds['citation']}\n")

    print("=======================================================")
    print("  MANUAL ACQUISITION INSTRUCTIONS:")
    print("  1. Kaggle Snoring Dataset : Download from kaggle.com/datasets/tareqkhan/snoring-dataset")
    print("     Place extracted WAV files in: data/raw/audio/kaggle_snoring/")
    print("  2. PhysioNet Apnea-ECG    : Download from physionet.org/content/apnea-ecg/1.0.0/")
    print("     Place .dat and .hea files in: data/raw/ppg/apnea_ecg/")
    print("  3. UCI Posture / HAR      : Download from archive.ics.uci.edu/dataset/240/")
    print("     Place CSV files in: data/raw/posture/uci_har/")
    print("=======================================================\n")

if __name__ == "__main__":
    init_directories()
    print_registry()
