Embedded Gesture Recognition - OLED and Servo Integration v1

Copy the existing secrets.h file from the stable sketch folder into this folder.
Do not share or modify secrets.h.

OLED SPI pins:
MOSI/SDA: GPIO23
CLK/SCL: GPIO18
DC: GPIO2
CS: GPIO5
RESET/RES: GPIO4

Servo signal: GPIO13
Servo power: external regulated 5 V
Ground: common with ESP32

Servo mapping:
right = 0 degrees
up = 90 degrees
left = 180 degrees
