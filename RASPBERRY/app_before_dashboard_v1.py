from datetime import datetime, timezone
import importlib.util
import time
from pathlib import Path

import numpy as np
from flask import Flask, jsonify, request

from dataset_manager import save_gesture_capture


PROJECT_DIRECTORY = Path(__file__).resolve().parent

MODEL_DIRECTORY = (
    PROJECT_DIRECTORY
    / "models"
    / "gesture_model_real_v1"
)

INFERENCE_MODULE_PATH = (
    MODEL_DIRECTORY
    / "numpy_gesture_inference.py"
)

WEIGHTS_PATH = (
    MODEL_DIRECTORY
    / "gesture_cnn_numpy_weights.npz"
)

NORMALIZATION_PATH = (
    MODEL_DIRECTORY
    / "normalization_parameters.json"
)

LABELS_PATH = (
    MODEL_DIRECTORY
    / "gesture_labels.json"
)

SENSOR_CHANNELS = [
    "ax",
    "ay",
    "az",
    "gx",
    "gy",
    "gz"
]

EXPECTED_SAMPLES = 100
MODEL_ENGINE = "numpy_cnn_1d"


def load_inference_module():
    required_files = [
        INFERENCE_MODULE_PATH,
        WEIGHTS_PATH,
        NORMALIZATION_PATH,
        LABELS_PATH
    ]

    for required_file in required_files:
        if not required_file.is_file():
            raise FileNotFoundError(
                f"Required model file not found: {required_file}"
            )

    module_specification = importlib.util.spec_from_file_location(
        "numpy_gesture_inference",
        INFERENCE_MODULE_PATH
    )

    if (
        module_specification is None
        or module_specification.loader is None
    ):
        raise ImportError(
            "Unable to load the NumPy inference module."
        )

    inference_module = importlib.util.module_from_spec(
        module_specification
    )

    module_specification.loader.exec_module(
        inference_module
    )

    return inference_module


inference_module = load_inference_module()

gesture_classifier = inference_module.GestureClassifier(
    weights_path=WEIGHTS_PATH,
    normalization_path=NORMALIZATION_PATH,
    labels_path=LABELS_PATH
)


def samples_to_array(samples):
    if not isinstance(samples, list):
        raise ValueError(
            "The samples field must be a list."
        )

    if len(samples) != EXPECTED_SAMPLES:
        raise ValueError(
            f"Exactly {EXPECTED_SAMPLES} samples are required. "
            f"Received: {len(samples)}."
        )

    capture_rows = []

    for sample_index, sample in enumerate(samples):
        if not isinstance(sample, dict):
            raise ValueError(
                f"Sample {sample_index} must be a JSON object."
            )

        missing_channels = [
            channel_name
            for channel_name in SENSOR_CHANNELS
            if channel_name not in sample
        ]

        if missing_channels:
            raise ValueError(
                f"Sample {sample_index} is missing channels: "
                f"{missing_channels}."
            )

        try:
            row = [
                float(sample[channel_name])
                for channel_name in SENSOR_CHANNELS
            ]
        except (TypeError, ValueError) as error:
            raise ValueError(
                f"Sample {sample_index} contains "
                "non-numeric sensor values."
            ) from error

        if not np.isfinite(row).all():
            raise ValueError(
                f"Sample {sample_index} contains "
                "non-finite sensor values."
            )

        capture_rows.append(row)

    capture_array = np.asarray(
        capture_rows,
        dtype=np.float32
    )

    expected_shape = (
        EXPECTED_SAMPLES,
        len(SENSOR_CHANNELS)
    )

    if capture_array.shape != expected_shape:
        raise ValueError(
            f"Invalid capture shape: {capture_array.shape}. "
            f"Expected: {expected_shape}."
        )

    return capture_array


app = Flask(__name__)

system_state = {
    "status": "ready",
    "gesture": "none",
    "confidence": 0.0,
    "received_samples": 0,
    "last_update": "No data received yet",
    "model_engine": MODEL_ENGINE,
    "inference_time_ms": 0.0
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

    try:
        capture_array = samples_to_array(samples)
    except ValueError as error:
        return jsonify({
            "error": str(error)
        }), 400

    inference_start_time = time.perf_counter()

    try:
        inference_result = gesture_classifier.predict(
            capture_array
        )
    except Exception:
        app.logger.exception(
            "Gesture inference failed."
        )

        system_state["status"] = "inference_error"

        return jsonify({
            "error": "Gesture inference failed."
        }), 500

    inference_time_ms = (
        time.perf_counter()
        - inference_start_time
    ) * 1000.0

    system_state["status"] = "processed"
    system_state["gesture"] = inference_result["gesture"]
    system_state["confidence"] = inference_result["confidence"]
    system_state["received_samples"] = len(samples)
    system_state["last_update"] = datetime.now(
        timezone.utc
    ).isoformat()
    system_state["model_engine"] = MODEL_ENGINE
    system_state["inference_time_ms"] = round(
        inference_time_ms,
        3
    )

    return jsonify({
        "success": True,
        "gesture": system_state["gesture"],
        "confidence": system_state["confidence"],
        "probabilities": inference_result["probabilities"],
        "received_samples": system_state["received_samples"],
        "model_engine": MODEL_ENGINE,
        "inference_time_ms": system_state["inference_time_ms"]
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
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=False
    )
