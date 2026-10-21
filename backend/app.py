"""
SleepSense AI - Flask Application Server
Modular Web Backend & Multi-Modal IoT Server
"""

import os
from flask import Flask, send_from_directory
from flask_cors import CORS
from config import Config
from database import Database
from routes.api import api_bp
from routes.demo import demo_bp

def create_app():
    # Configure frontend path
    frontend_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "frontend"))
    if not os.path.exists(frontend_dir):
        frontend_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "public"))

    app = Flask(__name__, static_folder=None)
    CORS(app, origins=Config.CORS_ORIGINS)

    # Initialize Database
    db = Database(Config.DB_PATH)
    app.config['DB'] = db

    # Register API Blueprints (Dual prefix support for /api/v1 and /api)
    app.register_blueprint(api_bp)
    app.register_blueprint(api_bp, url_prefix='/api', name='api_legacy')
    app.register_blueprint(demo_bp)

    # Serve Consumer Web Application
    @app.route('/')
    def serve_index():
        return send_from_directory(frontend_dir, 'index.html')

    @app.route('/<path:path>')
    def serve_static(path):
        if path.startswith('api/'):
            return {"status": "error", "message": f"API endpoint /{path} not found"}, 404
        if os.path.exists(os.path.join(frontend_dir, path)):
            return send_from_directory(frontend_dir, path)
        return send_from_directory(frontend_dir, 'index.html')

    # Seed initial telemetry if database empty
    with app.app_context():
        latest = db.get_latest_reading()
        if not latest:
            from routes.demo import seed_demo_telemetry
            with app.test_request_context():
                seed_demo_telemetry()
                print("[INFO] Seeded realistic baseline multi-modal telemetry.")

    return app

if __name__ == '__main__':
    app = create_app()
    print("\n=======================================================")
    print(f"  {Config.PROJECT_NAME} Backend Server v{Config.VERSION}")
    print(f"  Running on : http://{Config.HOST}:{Config.PORT}")
    print(f"  Frontend   : http://localhost:{Config.PORT}")
    print(f"  API Docs   : http://localhost:{Config.PORT}/api/v1/health")
    print("=======================================================\n")
    app.run(host=Config.HOST, port=Config.PORT, debug=Config.DEBUG)
