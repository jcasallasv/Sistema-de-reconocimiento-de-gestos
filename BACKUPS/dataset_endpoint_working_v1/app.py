from datetime import datetime, timezone

from flask import Flask, jsonify, request

from dataset_manager import save_gesture_capture

app = Flask(__name__)

system_state = {
    "status": "ready",
    "gesture": "none",
    "confidence": 0.0,
    "received_samples": 0,
    "last_update": "No data received yet"
}


@app.get("/")
def home():
    return """
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Embedded Gesture Recognition</title>

        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #f4f6f8;
                margin: 0;
                padding: 40px;
                color: #1f2933;
            }

            .container {
                max-width: 800px;
                margin: auto;
            }

            h1 {
                color: #123b5d;
            }

            .status-grid {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
                gap: 16px;
                margin-top: 30px;
            }

            .card {
                background-color: white;
                padding: 20px;
                border-radius: 10px;
                box-shadow: 0 3px 10px rgba(0, 0, 0, 0.10);
            }

            .label {
                font-size: 14px;
                color: #657786;
            }

            .value {
                margin-top: 8px;
                font-size: 24px;
                font-weight: bold;
                color: #102a43;
            }

            .footer {
                margin-top: 30px;
                color: #657786;
                font-size: 14px;
            }
        </style>
    </head>

    <body>
        <div class="container">
            <h1>Embedded Gesture Recognition System</h1>
            <p>ESP32 and Raspberry Pi wireless gesture recognition platform.</p>

            <div class="status-grid">
                <div class="card">
                    <div class="label">System Status</div>
                    <div class="value" id="status">Loading...</div>
                </div>

                <div class="card">
                    <div class="label">Last Gesture</div>
                    <div class="value" id="gesture">Loading...</div>
                </div>

                <div class="card">
                    <div class="label">Confidence</div>
                    <div class="value" id="confidence">Loading...</div>
                </div>

                <div class="card">
                    <div class="label">Received Samples</div>
                    <div class="value" id="receivedSamples">Loading...</div>
                </div>
            </div>

            <div class="footer">
                Last update:
                <span id="lastUpdate">Loading...</span>
            </div>
        </div>

        <script>
            async function updateDashboard() {
                try {
                    const response = await fetch("/api/status");
                    const data = await response.json();

                    document.getElementById("status").textContent =
                        data.status;

                    document.getElementById("gesture").textContent =
                        data.gesture;

                    document.getElementById("confidence").textContent =
                        Math.round(data.confidence * 100) + "%";

                    document.getElementById("receivedSamples").textContent =
                        data.received_samples;

                    document.getElementById("lastUpdate").textContent =
                        data.last_update;
                } catch (error) {
                    document.getElementById("status").textContent =
                        "Connection error";
                }
            }

            updateDashboard();
            setInterval(updateDashboard, 1000);
        </script>
    </body>
    </html>
    """


@app.get("/api/status")
def get_status():
    return jsonify(system_state)


@app.post("/api/gesture")
def receive_gesture():
    data = request.get_json(silent=True)

    if not isinstance(data, dict):
        return jsonify({
            "error": "A JSON body is required."
        }), 400

    samples = data.get("samples")

    if not isinstance(samples, list) or len(samples) == 0:
        return jsonify({
            "error": "The samples field must be a non-empty list."
        }), 400

    system_state["status"] = "processed"
    system_state["gesture"] = "simulated_gesture"
    system_state["confidence"] = 0.99
    system_state["received_samples"] = len(samples)
    system_state["last_update"] = datetime.now(
        timezone.utc
    ).isoformat()

    return jsonify({
        "success": True,
        "gesture": system_state["gesture"],
        "confidence": system_state["confidence"],
        "received_samples": system_state["received_samples"]
    })


@app.post("/api/dataset")
def save_dataset_capture():
    data = request.get_json(silent=True)

    if not isinstance(data, dict):
        return jsonify({
            "error": "A JSON body is required."
        }), 400

    label = data.get("label")
    samples = data.get("samples")

    if not isinstance(label, str) or not label.strip():
        return jsonify({
            "error": "The label field must be a non-empty string."
        }), 400

    if not isinstance(samples, list) or len(samples) == 0:
        return jsonify({
            "error": "The samples field must be a non-empty list."
        }), 400

    try:
        saved_file = save_gesture_capture(
            samples=samples,
            label=label.strip()
        )
    except ValueError as error:
        return jsonify({
            "error": str(error)
        }), 400

    return jsonify({
        "success": True,
        "label": label.strip(),
        "sample_count": len(samples),
        "file_name": saved_file.name
    }), 201


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False)
