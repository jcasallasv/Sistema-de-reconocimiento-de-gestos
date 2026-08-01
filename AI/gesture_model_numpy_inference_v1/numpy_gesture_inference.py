
import json
from pathlib import Path

import numpy as np


class GestureClassifier:
    def __init__(
        self,
        weights_path,
        normalization_path,
        labels_path
    ):
        self.weights_path = Path(weights_path)
        self.normalization_path = Path(normalization_path)
        self.labels_path = Path(labels_path)

        self._load_configuration()
        self._load_weights()

    def _load_configuration(self):
        with self.normalization_path.open(
            "r",
            encoding="utf-8"
        ) as normalization_file:
            normalization_data = json.load(
                normalization_file
            )

        with self.labels_path.open(
            "r",
            encoding="utf-8"
        ) as labels_file:
            labels_data = json.load(
                labels_file
            )

        self.channel_names = normalization_data[
            "channel_names"
        ]

        self.channel_mean = np.asarray(
            normalization_data["mean"],
            dtype=np.float32
        ).reshape(1, -1)

        self.channel_std = np.asarray(
            normalization_data[
                "standard_deviation"
            ],
            dtype=np.float32
        ).reshape(1, -1)

        self.labels = labels_data["labels"]

        self.samples_per_capture = 100
        self.sensor_channels = 6

        if len(self.channel_names) != self.sensor_channels:
            raise ValueError(
                "Invalid number of sensor channel names."
            )

        if self.channel_mean.shape != (1, 6):
            raise ValueError(
                "Invalid normalization mean shape."
            )

        if self.channel_std.shape != (1, 6):
            raise ValueError(
                "Invalid normalization standard deviation shape."
            )

        if len(self.labels) != 3:
            raise ValueError(
                "Invalid number of gesture labels."
            )

    def _load_weights(self):
        weights = np.load(
            self.weights_path,
            allow_pickle=False
        )

        self.conv1_kernel = weights[
            "conv1_kernel"
        ].astype(np.float32)

        self.conv1_bias = weights[
            "conv1_bias"
        ].astype(np.float32)

        self.conv2_kernel = weights[
            "conv2_kernel"
        ].astype(np.float32)

        self.conv2_bias = weights[
            "conv2_bias"
        ].astype(np.float32)

        self.dense1_kernel = weights[
            "dense1_kernel"
        ].astype(np.float32)

        self.dense1_bias = weights[
            "dense1_bias"
        ].astype(np.float32)

        self.output_kernel = weights[
            "output_kernel"
        ].astype(np.float32)

        self.output_bias = weights[
            "output_bias"
        ].astype(np.float32)

        expected_shapes = {
            "conv1_kernel": (5, 6, 16),
            "conv1_bias": (16,),
            "conv2_kernel": (3, 16, 32),
            "conv2_bias": (32,),
            "dense1_kernel": (32, 16),
            "dense1_bias": (16,),
            "output_kernel": (16, 3),
            "output_bias": (3,)
        }

        for weight_name, expected_shape in expected_shapes.items():
            actual_shape = getattr(
                self,
                weight_name
            ).shape

            if actual_shape != expected_shape:
                raise ValueError(
                    f"Invalid shape for {weight_name}: "
                    f"{actual_shape}"
                )

    @staticmethod
    def _relu(values):
        return np.maximum(
            values,
            0.0
        ).astype(np.float32)

    @staticmethod
    def _softmax(logits):
        shifted_logits = (
            logits
            - np.max(logits)
        )

        exponential_values = np.exp(
            shifted_logits
        )

        return (
            exponential_values
            / np.sum(exponential_values)
        ).astype(np.float32)

    @staticmethod
    def _conv1d_same(
        input_data,
        kernel,
        bias
    ):
        kernel_size = kernel.shape[0]
        output_channels = kernel.shape[2]

        padding_left = (
            kernel_size - 1
        ) // 2

        padding_right = (
            kernel_size - 1
            - padding_left
        )

        padded_input = np.pad(
            input_data,
            (
                (padding_left, padding_right),
                (0, 0)
            ),
            mode="constant"
        )

        output_data = np.empty(
            (
                input_data.shape[0],
                output_channels
            ),
            dtype=np.float32
        )

        for time_index in range(
            input_data.shape[0]
        ):
            input_window = padded_input[
                time_index:
                time_index + kernel_size
            ]

            output_data[time_index] = (
                np.tensordot(
                    input_window,
                    kernel,
                    axes=(
                        [0, 1],
                        [0, 1]
                    )
                )
                + bias
            )

        return output_data

    @staticmethod
    def _max_pool1d(
        input_data,
        pool_size=2
    ):
        usable_length = (
            input_data.shape[0]
            // pool_size
            * pool_size
        )

        trimmed_input = input_data[
            :usable_length
        ]

        return trimmed_input.reshape(
            -1,
            pool_size,
            input_data.shape[1]
        ).max(
            axis=1
        ).astype(np.float32)

    def normalize_capture(
        self,
        capture
    ):
        capture_array = np.asarray(
            capture,
            dtype=np.float32
        )

        expected_shape = (
            self.samples_per_capture,
            self.sensor_channels
        )

        if capture_array.shape != expected_shape:
            raise ValueError(
                f"Invalid capture shape: "
                f"{capture_array.shape}. "
                f"Expected: {expected_shape}."
            )

        if not np.isfinite(
            capture_array
        ).all():
            raise ValueError(
                "Capture contains non-finite values."
            )

        return (
            (
                capture_array
                - self.channel_mean
            )
            / self.channel_std
        ).astype(np.float32)

    def predict_normalized(
        self,
        normalized_capture
    ):
        layer_output = self._conv1d_same(
            normalized_capture,
            self.conv1_kernel,
            self.conv1_bias
        )

        layer_output = self._relu(
            layer_output
        )

        layer_output = self._max_pool1d(
            layer_output
        )

        layer_output = self._conv1d_same(
            layer_output,
            self.conv2_kernel,
            self.conv2_bias
        )

        layer_output = self._relu(
            layer_output
        )

        layer_output = layer_output.mean(
            axis=0
        ).astype(np.float32)

        layer_output = (
            layer_output
            @ self.dense1_kernel
            + self.dense1_bias
        )

        layer_output = self._relu(
            layer_output
        )

        logits = (
            layer_output
            @ self.output_kernel
            + self.output_bias
        )

        probabilities = self._softmax(
            logits
        )

        predicted_index = int(
            np.argmax(probabilities)
        )

        return {
            "gesture": self.labels[
                predicted_index
            ],
            "confidence": float(
                probabilities[predicted_index]
            ),
            "probabilities": {
                label: float(probability)
                for label, probability in zip(
                    self.labels,
                    probabilities
                )
            }
        }

    def predict(
        self,
        capture
    ):
        normalized_capture = (
            self.normalize_capture(
                capture
            )
        )

        return self.predict_normalized(
            normalized_capture
        )
