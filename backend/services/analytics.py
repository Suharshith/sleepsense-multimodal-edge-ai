"""
SleepSense AI - Analytics & Sleep Session Summary Service
"""

import numpy as np

class AnalyticsService:
    @staticmethod
    def compute_session_analytics(readings, events):
        if not readings or len(readings) == 0:
            return {
                "total_readings": 0,
                "duration_minutes": 0,
                "spo2": {"average": 0.0, "minimum": 0.0},
                "heart_rate": {"average": 0.0},
                "events": {"snoring": 0, "abnormal_breathing": 0, "desaturations": 0},
                "posture_distribution": {},
                "risk_summary": {"average_score": 0.0, "max_score": 0.0, "overall_level": "INSUFFICIENT_DATA"},
                "insights": ["Collecting baseline sleep signals..."]
            }

        # Physiological analytics
        valid_spo2 = [r['spo2'] for r in readings if r.get('spo2', 0) > 0]
        avg_spo2 = float(np.mean(valid_spo2)) if valid_spo2 else 96.5
        min_spo2 = float(np.min(valid_spo2)) if valid_spo2 else 88.0

        valid_hr = [r['heart_rate'] for r in readings if r.get('heart_rate', 0) > 0]
        avg_hr = float(np.mean(valid_hr)) if valid_hr else 68.0

        # Event counts
        snoring_count = sum(1 for e in events if e.get('event_type') == 'SNORING')
        abnormal_count = sum(1 for e in events if e.get('event_type') == 'ABNORMAL_BREATHING')
        desat_count = sum(1 for e in events if e.get('event_type') == 'OXYGEN_DESATURATION' or (e.get('spo2', 100) < 92 and e.get('spo2', 0) > 0))

        # Posture distribution
        postures = [r.get('posture', 'BACK') for r in readings]
        posture_counts = {}
        for p in postures:
            posture_counts[p] = posture_counts.get(p, 0) + 1
        
        dominant_posture = max(posture_counts, key=posture_counts.get) if posture_counts else "BACK"

        # Risk Score Analytics
        risk_scores = [r.get('risk_score', 15.0) for r in readings]
        avg_risk = float(np.mean(risk_scores)) if risk_scores else 15.0
        max_risk = float(np.max(risk_scores)) if risk_scores else 15.0

        if max_risk >= 70.0:
            overall_level = "HIGH"
        elif max_risk >= 45.0:
            overall_level = "MODERATE"
        else:
            overall_level = "LOW"

        # Explainable AI Insights ("What we noticed")
        insights = []
        if avg_spo2 >= 95.0:
            insights.append("Your blood oxygen saturation remained generally stable throughout the session.")
        else:
            insights.append(f"Transient blood oxygen desaturations were observed with lowest dip at {min_spo2:.1f}%.")

        if snoring_count > 10:
            insights.append(f"Acoustic snore activity was detected ({snoring_count} events).")
        else:
            insights.append("Minimal acoustic snoring detected during this monitoring period.")

        insights.append(f"You spent the majority of the session in the {dominant_posture} position.")

        duration_minutes = int(len(readings) * 2 / 60) # 2s per reading
        if duration_minutes < 1: duration_minutes = 1

        return {
            "total_readings": len(readings),
            "duration_minutes": duration_minutes,
            "spo2": {"average": round(avg_spo2, 1), "minimum": round(min_spo2, 1)},
            "heart_rate": {"average": round(avg_hr, 1)},
            "events": {"snoring": snoring_count, "abnormal_breathing": abnormal_count, "desaturations": desat_count},
            "posture_distribution": posture_counts,
            "dominant_posture": dominant_posture,
            "risk_summary": {
                "average_score": round(avg_risk, 1),
                "max_score": round(max_risk, 1),
                "overall_level": overall_level
            },
            "insights": insights
        }
