from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


PROJECT_DIR = Path("/home/salo/embedded_gesture_system")
RAW_DATASET_DIR = PROJECT_DIR / "dataset" / "raw"


def sanitize_label(label: str) -> str:
    cleaned_label = "".join(
        character if character.isalnum() or character in ("-", "_") else "_"
        for character in label.strip().lower()
    )

    return cleaned_label or "unlabeled"


def save_gesture_capture(
    samples: list[dict[str, Any]],
    label: str,
    output_dir: Path = RAW_DATASET_DIR,
) -> Path:
    if not isinstance(samples, list) or not samples:
        raise ValueError("The samples list cannot be empty.")

    required_fields = {"ax", "ay", "az", "gx", "gy", "gz"}

    for sample_index, sample in enumerate(samples):
        if not isinstance(sample, dict):
            raise ValueError(
                f"Sample {sample_index} must be a JSON object."
            )

        missing_fields = required_fields.difference(sample.keys())

        if missing_fields:
            raise ValueError(
                f"Sample {sample_index} is missing fields: "
                f"{sorted(missing_fields)}"
            )

    output_dir.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now(timezone.utc)
    safe_label = sanitize_label(label)

    file_name = (
        f"{timestamp.strftime('%Y%m%dT%H%M%S_%fZ')}"
        f"_{safe_label}.json"
    )

    file_path = output_dir / file_name

    capture_data = {
        "label": label,
        "captured_at_utc": timestamp.isoformat(),
        "sample_count": len(samples),
        "samples": samples,
    }

    file_path.write_text(
        json.dumps(capture_data, indent=2),
        encoding="utf-8",
    )

    return file_path


if __name__ == "__main__":
    test_samples = [
        {
            "ax": 0.10,
            "ay": 0.20,
            "az": 9.80,
            "gx": 0.01,
            "gy": 0.02,
            "gz": 0.03,
        },
        {
            "ax": 0.15,
            "ay": 0.25,
            "az": 9.75,
            "gx": 0.02,
            "gy": 0.03,
            "gz": 0.04,
        },
    ]

    test_directory = PROJECT_DIR / "dataset" / "test"

    saved_file = save_gesture_capture(
        samples=test_samples,
        label="test_capture",
        output_dir=test_directory,
    )

    print(f"Saved test capture: {saved_file}")
