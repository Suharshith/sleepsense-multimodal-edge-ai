"""
Connection & Diagnostic Test Utility
Adaptive Multimodal Edge AI Sleep Apnea Monitoring System V2 (ESP32 DevKit V1)

Checks:
1. Local Host Network IPv4 Addresses (for ESP32 config.h setup).
2. Backend API Server status (http://localhost:5000/api/readings/latest).
3. Simulates an ESP32 DevKit V1 telemetry transmission payload.
"""

import socket
import urllib.request
import json
import sys

def get_local_ip_addresses():
    print("=======================================================")
    print("  1. LOCAL NETWORK & IP CONFIGURATION")
    print("=======================================================")
    hostname = socket.gethostname()
    print(f"  Host Machine Name: {hostname}")
    
    ips = []
    try:
        # Get primary outward facing IP
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        primary_ip = s.getsockname()[0]
        s.close()
        ips.append(primary_ip)
        print(f"  -> Recommended Server IP (for config.h): http://{primary_ip}:5000/api/v1/sensor-data")
    except Exception:
        pass

    try:
        addr_info = socket.getaddrinfo(hostname, None, socket.AF_INET)
        for addr in addr_info:
            ip = addr[4][0]
            if ip not in ips and not ip.startswith("127."):
                ips.append(ip)
                print(f"  -> Alternative IP: http://{ip}:5000/api/v1/sensor-data")
    except Exception as e:
        print(f"  Could not list all IPs: {e}")
    
    return ips

def check_backend_server():
    print("\n=======================================================")
    print("  2. CHECKING BACKEND API SERVER STATUS")
    print("=======================================================")
    url = "http://localhost:5000/api/v1/health"
    try:
        req = urllib.request.Request(url, headers={'User-Agent': 'ConnectionChecker/1.0'})
        with urllib.request.urlopen(req, timeout=3) as response:
            if response.status == 200:
                body = json.loads(response.read().decode('utf-8'))
                print("  [SUCCESS] Backend API Server is UP and RUNNING on Port 5000!")
                print(f"  Status: {body.get('status')} | System: {body.get('system')}")
                return True
    except Exception as e:
        print(f"  [OFFLINE / NOT RUNNING] Could not reach backend server at http://localhost:5000/api/v1/health")
        print(f"  Reason: {e}")
        print("  -> Start the server by running: run.bat (or python backend/app.py)")
        return False

def test_telemetry_post():
    print("\n=======================================================")
    print("  3. SIMULATING ESP32 DEVKIT V1 TELEMETRY TRANSMISSION")
    print("=======================================================")
    url = "http://localhost:5000/api/v1/sensor-data"
    payload = {
        "device_id": "ESP32-DEVKITV1-SLEEP-01",
        "timestamp": 1718000000,
        "audio": {
            "estimated_db": 58.4,
            "snore_detected": True,
            "class": "Snoring",
            "quality": 0.95
        },
        "ppg": {
            "spo2": 95.0,
            "bpm": 68.0,
            "hrv": 35.0,
            "quality": 0.95
        },
        "posture": {
            "class": "BACK",
            "movement": 0.12,
            "quality": 0.95
        },
        "environment": {
            "temperature": 24.5,
            "humidity": 55.0,
            "quality": 0.98
        }
    }

    try:
        data = json.dumps(payload).encode('utf-8')
        req = urllib.request.Request(url, data=data, headers={'Content-Type': 'application/json'})
        with urllib.request.urlopen(req, timeout=3) as response:
            if response.status == 201 or response.status == 200:
                res = json.loads(response.read().decode('utf-8'))
                print("  [SUCCESS] Telemetry Payload successfully accepted by Backend!")
                print(f"  Status: {res.get('status')}")
                print(f"  Risk Score: {res['fusion']['risk_score']}/100")
                print(f"  Risk Level: {res['fusion']['risk_level']}")
                print(f"  Confidence: {res['fusion']['confidence']}")
                return True
    except Exception as e:
        print(f"  [FAILED] Could not send test payload to {url}: {e}")
        return False

if __name__ == "__main__":
    get_local_ip_addresses()
    backend_up = check_backend_server()
    if backend_up:
        test_telemetry_post()
    print("\n=======================================================")
    print("  DIAGNOSTIC TEST COMPLETE")
    print("=======================================================\n")
