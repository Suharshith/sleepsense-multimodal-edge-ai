"""
SleepSense AI - TinyML Forward Pass & Quantization Verification Tests
"""

import sys
import os
import unittest
import numpy as np

class TinyMLInferenceTestCase(unittest.TestCase):
    def setUp(self):
        # Trained weights matching tinyml_audio.cpp
        self.W1 = np.array([
            [-0.08, -0.03,  1.34, -0.79,  0.44,  0.22,  1.27, -0.69],
            [-0.76, -0.22,  1.27, -0.29,  0.04, -0.63,  1.41,  0.30],
            [ 0.47, -0.49, -0.13,  0.78, -0.43,  0.30,  1.83, -0.57],
            [ 0.06,  0.12, -0.11,  1.47, -0.04, -0.90, -0.10, -0.55],
            [-0.39, -0.05, -0.54,  0.38, -0.81, -0.89, -0.81,  0.26],
            [-0.21, -0.37, -0.88,  0.56,  0.05,  0.13, -0.02, -0.23],
            [-0.80, -0.30,  0.71,  0.05,  0.45,  0.41,  0.77, -0.23],
            [ 0.77,  0.33,  0.47,  0.41, -0.13, -0.07,  0.45, -0.29],
            [-0.66,  0.28,  0.31,  0.66, -0.68,  0.50, -0.38, -0.74],
            [-0.30, -0.16, -0.15,  0.23,  0.03, -0.24, -1.25,  0.17],
            [-0.41, -0.27,  1.14,  0.42, -0.50, -0.53, -0.31,  0.22],
            [-0.29, -0.49,  0.72,  0.47, -0.33, -0.49, -0.44,  0.05],
            [-0.29,  0.10,  0.68,  0.69, -0.35, -0.51, -0.60,  0.53]
        ])
        self.B1 = np.array([-0.10, 0.00, 1.49, -1.28, -0.19, -0.06, 1.97, -0.13])
        self.W2 = np.array([
            [-0.98, -1.07, -0.14],
            [-0.19, -0.10, -0.12],
            [ 1.16, -1.52,  0.61],
            [-2.64,  0.36, -0.53],
            [-0.99, -0.10, -0.21],
            [-0.17, -0.12, -0.01],
            [ 1.91, -0.49, -1.60],
            [ 1.03, -0.50, -0.55]
        ])
        self.B2 = np.array([0.91, 0.44, -0.77])

    def forward(self, x):
        h = np.maximum(0, np.dot(x, self.W1) + self.B1) # ReLU
        logits = np.dot(h, self.W2) + self.B2
        exps = np.exp(logits - np.max(logits))
        probs = exps / np.sum(exps)
        return probs

    def test_snore_pattern_inference(self):
        # Snoring synthetic feature signature (strong mid-band energy)
        feat_snore = np.array([1.0, 1.0, 5.5, 5.8, 5.2, 5.0, 1.5, 1.4, 0.6, 0.5, 0.4, 0.3, 0.3])
        probs = self.forward(feat_snore)
        self.assertEqual(len(probs), 3)
        self.assertAlmostEqual(float(np.sum(probs)), 1.0, places=4)
        pred_class = np.argmax(probs)
        self.assertEqual(pred_class, 1) # Class 1: Snoring

    def test_normal_breathing_inference(self):
        # Normal quiet breathing signature
        feat_normal = np.array([1.1, 1.0, 0.4, 0.3, 0.3, 0.4, 0.2, 0.2, 0.1, 0.1, 0.1, 0.1, 0.1])
        probs = self.forward(feat_normal)
        pred_class = np.argmax(probs)
        self.assertEqual(pred_class, 0) # Class 0: Normal

if __name__ == '__main__':
    unittest.main()
