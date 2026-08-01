MPU6050 and APDS-9960 Synchronization Proof
===========================================

Confirmed:
- MPU6050 and APDS-9960 share the I2C bus.
- Button interrupt starts capture.
- 100 MPU6050 samples are collected.
- Sample rate is approximately 50 Hz.
- Capture duration is approximately 1980 ms.
- APDS-9960 successfully produced the reference label left.
- All six MPU6050 channels showed motion variation.

Observation:
- Simultaneous APDS-9960 labeling sometimes returned none.
- The final dataset collector will use a two-stage protocol:
  1. APDS-9960 selects the reference label.
  2. Button starts the MPU6050 capture for that selected label.

Status:
Functional proof completed.
