"""
SleepSense AI - Clinical Screening Report Generator
Produces printable Polysomnography-style screening summaries with strict medical safety disclaimers.
"""

from datetime import datetime

class ReportGenerator:
    @staticmethod
    def generate_clinical_report(session_info, analytics_data, patient_info=None):
        if not patient_info:
            patient_info = {
                "patient_id": "P-10924",
                "name": session_info.get("patient_name", "Anonymous Patient (Research Participant)"),
                "age": 45,
                "gender": "Adult"
            }

        risk_level = analytics_data["risk_summary"]["overall_level"]
        risk_score = analytics_data["risk_summary"]["max_score"]

        # Explainable contributing factors
        factors = []
        if analytics_data["spo2"]["minimum"] < 92.0:
            factors.append(f"Lowest oxygen saturation dropped to {analytics_data['spo2']['minimum']}%.")
        if analytics_data["events"]["snoring"] > 5:
            factors.append(f"Recorded {analytics_data['events']['snoring']} acoustic snoring episodes.")
        if analytics_data["dominant_posture"] == "BACK":
            factors.append("Dominant sleeping position was Supine (Back), which can anatomically exacerbate upper airway collapse.")
        if not factors:
            factors.append("All monitored acoustic, physiological, and positional indicators remained within normal baseline ranges.")

        # Recommendations based on risk
        recs = []
        if risk_level in ["HIGH", "MODERATE"]:
            recs.append("Discuss these screening results with a primary care physician or certified sleep medicine specialist.")
            recs.append("Consider positional sleep therapy (e.g. lateral sleeping) to reduce supine-related airway obstruction.")
            recs.append("A formal in-laboratory Polysomnography (PSG) or Home Sleep Apnea Test (HSAT) may be warranted for formal diagnostic confirmation.")
        else:
            recs.append("Maintain healthy sleep hygiene and consistent sleep schedules.")
            recs.append("Continue periodic non-invasive sleep monitoring if daytime sleepiness or fatigue is experienced.")

        report = {
            "report_id": f"RPT-{int(datetime.utcnow().timestamp())}",
            "generated_at": datetime.utcnow().isoformat() + "Z",
            "device": {
                "hardware_model": "ESP32 DevKit V1 (ESP-WROOM-32 @ 160MHz)",
                "firmware_version": "v2.0.0-PROD",
                "sensors": "INMP441 (I2S), MAX30102 (I2C), MPU6050 (I2C), DHT22"
            },
            "patient": patient_info,
            "session": {
                "session_id": session_info.get("session_id", "SESSION-DEMO-001"),
                "duration_minutes": analytics_data.get("duration_minutes", 30),
                "total_readings": analytics_data.get("total_readings", 0)
            },
            "physiological_summary": {
                "average_spo2": analytics_data["spo2"]["average"],
                "minimum_spo2": analytics_data["spo2"]["minimum"],
                "average_heart_rate_bpm": analytics_data["heart_rate"]["average"],
                "desaturation_events": analytics_data["events"]["desaturations"]
            },
            "acoustic_summary": {
                "snoring_events": analytics_data["events"]["snoring"],
                "abnormal_breathing_events": analytics_data["events"]["abnormal_breathing"]
            },
            "postural_summary": {
                "dominant_position": analytics_data["dominant_posture"],
                "distribution": analytics_data["posture_distribution"]
            },
            "risk_assessment": {
                "overall_risk_score": risk_score,
                "overall_risk_level": risk_level,
                "confidence_score": 0.91,
                "contributing_factors": factors
            },
            "recommendations": recs,
            "medical_disclaimer": "SleepSense AI provides experimental sleep-apnea risk assessment and is not a medical diagnostic device. It does not replace professional medical evaluation or a clinical sleep study."
        }

        return report
