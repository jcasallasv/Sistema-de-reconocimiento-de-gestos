# Gesture Capture Protocol

## General configuration

- Valid labels: left, right, up.
- Samples per capture: 100.
- Sample rate: 50 Hz.
- Capture duration: approximately 2 seconds.
- The MPU6050 and the hand detected by the APDS-9960 must perform the same physical gesture simultaneously.
- Only one gesture is performed during each capture.

## Sensor position

- The APDS-9960 remains fixed on the breadboard.
- The APDS-9960 gesture sensor must face the moving hand.
- The MPU6050 is held firmly in the same hand that passes in front of the APDS-9960.
- The MPU6050 orientation must remain unchanged between all captures.
- The MPU6050 component side remains facing upward.
- The sensor cables remain oriented toward the wrist.
- Wrist rotation must be minimized.

## Initial position

- Start with the hand centered in front of the APDS-9960.
- Keep the hand approximately 10 to 15 cm from the APDS-9960.
- Keep the hand and MPU6050 still before pressing the button.
- Do not begin moving during the three LED preparation blinks.

## Capture procedure

1. Place the hand in the initial position.
2. Press the capture button once.
3. Wait during the three LED blinks.
4. Start the gesture when the LED remains continuously on.
5. Perform one smooth translation lasting approximately 1 second.
6. Keep the final position until the LED turns off.
7. Do not return the hand to the initial position during the capture.
8. Wait for the Serial Monitor result before starting another capture.

## Gesture definitions

### left

- Use the same physical direction that produced the validated APDS-9960 label `left`.
- Move the complete hand and MPU6050 horizontally in that direction.
- Avoid rotating the wrist.

### right

- Move the complete hand and MPU6050 horizontally in the direction opposite to `left`.
- Avoid rotating the wrist.

### up

- Move the complete hand and MPU6050 vertically upward.
- Avoid moving diagonally or rotating the wrist.

## Acceptance rules

A capture is accepted only when:

- The APDS-9960 label is `left`, `right`, or `up`.
- Exactly 100 MPU6050 samples were captured.
- The server returns HTTP 201.
- The server response reports `success: true`.
- The saved label matches the intended physical gesture.

A capture is rejected when:

- The APDS-9960 reports `none`, `down`, `near`, or `far`.
- The detected label does not match the intended gesture.
- The HTTP request fails.
- The capture contains an accidental second movement.
- The MPU6050 orientation changed significantly.

Rejected captures must not be included in the final dataset.
