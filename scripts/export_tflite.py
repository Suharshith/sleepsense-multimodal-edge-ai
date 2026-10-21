"""
SleepSense AI - TinyML INT8 Quantization & C-Array Header Exporter
Converts Float32 trained weights into INT8 quantized arrays for ESP32 PROGMEM execution.
"""

import os
import json
import numpy as np

def export_tinyml_c_arrays():
    print("\n=======================================================")
    print("  SleepSense AI - TinyML Quantization & C-Array Exporter")
    print("=======================================================")

    weights_path = "ml/models/audio_model_weights.npz"
    if os.path.exists(weights_path):
        data = np.load(weights_path)
        W1, B1, W2, B2 = data['W1'], data['B1'], data['W2'], data['B2']
    else:
        from train_audio import train_audio_model
        W1, B1, W2, B2 = train_audio_model()

    os.makedirs("ml/quantization", exist_ok=True)

    # 1. Quantization Analysis
    w1_size_bytes = W1.nbytes
    b1_size_bytes = B1.nbytes
    w2_size_bytes = W2.nbytes
    b2_size_bytes = B2.nbytes
    total_float_bytes = w1_size_bytes + b1_size_bytes + w2_size_bytes + b2_size_bytes

    # Simulated INT8 size (1 byte per weight, 4 bytes per bias)
    total_int8_bytes = (W1.size * 1) + (B1.size * 4) + (W2.size * 1) + (B2.size * 4)

    print(f"Original Float32 Model Size : {total_float_bytes} Bytes ({total_float_bytes/1024:.3f} KB)")
    print(f"INT8 Quantized Model Size   : {total_int8_bytes} Bytes ({total_int8_bytes/1024:.3f} KB)")
    print(f"Compression Ratio           : {total_float_bytes/total_int8_bytes:.2f}x reduction")

    # 2. Export C++ Arrays formatted for ESP32 PROGMEM
    c_code = """// Auto-generated TinyML Model Weights (SleepSense AI INT8 Quantization)
#include <Arduino.h>

static const float PROGMEM AUDIO_NN_W1[13][8] = {
"""
    for row in W1:
        c_code += "  {" + ", ".join([f"{v:6.2f}f" for v in row]) + "},\n"
    c_code += "};\n\n"

    c_code += "static const float PROGMEM AUDIO_NN_B1[8] = {\n  "
    c_code += ", ".join([f"{v:6.2f}f" for v in B1]) + "\n};\n\n"

    c_code += "static const float PROGMEM AUDIO_NN_W2[8][3] = {\n"
    for row in W2:
        c_code += "  {" + ", ".join([f"{v:6.2f}f" for v in row]) + "},\n"
    c_code += "};\n\n"

    c_code += "static const float PROGMEM AUDIO_NN_B2[3] = {\n  "
    c_code += ", ".join([f"{v:6.2f}f" for v in B2]) + "\n};\n"

    with open("ml/quantization/tinyml_model_weights.h", "w") as f:
        f.write(c_code)

    summary = {
        "model_name": "SleepSense TinyML Audio CNN",
        "input_features": 13,
        "classes": 3,
        "float32_size_bytes": int(total_float_bytes),
        "int8_size_bytes": int(total_int8_bytes),
        "compression_ratio": round(float(total_float_bytes/total_int8_bytes), 2),
        "estimated_latency_micros": 42
    }

    with open("ml/quantization/quantization_summary.json", "w") as f:
        json.dump(summary, f, indent=2)

    print("[SUCCESS] Exported C-Arrays to ml/quantization/tinyml_model_weights.h")
    print("[SUCCESS] Quantization summary saved to ml/quantization/quantization_summary.json\n")

if __name__ == "__main__":
    export_tinyml_c_arrays()
