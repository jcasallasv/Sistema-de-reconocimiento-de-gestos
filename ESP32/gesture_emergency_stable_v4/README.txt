Emergency stable firmware v4

Changes from v3:
- I2C clock reduced from 100 kHz to 50 kHz.
- Rejects all-zero and physically implausible MPU6050 samples.
- Detects frozen repeated MPU6050 samples during capture.
- Attempts I2C recovery and MPU6050 reinitialization after frozen data.
- Uses motion validation: accelerometer >= 2500 AND gyroscope >= 2500.
- Keeps APDS-9960 disabled for final inference.
- Keeps servo PWM detached between gesture results.
- Preserves OLED state interface, HTTP JSON format and CNN inference.

Before compiling:
- Copy secrets.h from gesture_inference_wifi_oled_servo_v3.
- Board: DOIT ESP32 DEVKIT V1
- Serial Monitor: 115200 baud
