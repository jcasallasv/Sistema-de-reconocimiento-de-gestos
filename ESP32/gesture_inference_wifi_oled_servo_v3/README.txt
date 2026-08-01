Gesture inference firmware v3

Main stability changes:
- Validates that the button remains physically LOW for 30 ms.
- Disables APDS-9960 access in final inference mode.
- Checks MPU6050 health before every capture.
- Clears a stuck I2C bus and reinitializes the MPU6050 when required.
- Detaches the servo after movement to reduce electrical noise.
- Raises the gyroscope motion threshold from 500 to 1200 raw units.

Copy secrets.h from the previous stable firmware folder before compiling.
