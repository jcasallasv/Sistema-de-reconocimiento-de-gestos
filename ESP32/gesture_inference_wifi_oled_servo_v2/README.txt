Firmware v2 changes:
- Prevents queued double captures by disabling the button interrupt during capture/inference.
- Rearms the button only after physical release and an 80 ms stable interval.
- Retries each MPU6050 sample read up to four times.
- Recovers the ESP32 I2C controller after two consecutive read failures.
- Reports recovered MPU6050 reads after each successful capture.

Copy the existing secrets.h from the v1 folder into this folder before compiling.
