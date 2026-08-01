APDS-9960 Gesture Sensor Working Backup
=======================================

Hardware:
- Sensor: APDS-9960 RGB/Gesture
- I2C address: 0x39
- Device ID: 0xA8
- ESP32 SDA: GPIO 21
- ESP32 SCL: GPIO 22
- Supply voltage: 3.3 V

Library modification:
- Added APDS9960_ID_3 with value 0xA8.
- Updated device ID validation to accept APDS9960_ID_3.

Confirmed functions:
- APDS-9960 initialization.
- LEFT gesture.
- RIGHT gesture.
- UP gesture.
- DOWN gesture.
- FAR gesture.

Status:
Stable and tested.
