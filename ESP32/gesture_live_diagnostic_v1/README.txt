Gesture live diagnostic firmware v1

Purpose:
- Preserve firmware v2 behavior.
- Print all 100 raw MPU6050 samples after each capture.
- Printing occurs only after acquisition, so the 50 Hz capture timing is unchanged.
- Include accepted and rejected stationary captures in the diagnostic output.

Required:
- Copy secrets.h from gesture_inference_wifi_oled_servo_v2 before compiling.
- Board: DOIT ESP32 DEVKIT V1
- Serial Monitor: 115200 baud
