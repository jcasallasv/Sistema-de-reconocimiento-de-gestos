#include <Wire.h>

constexpr uint8_t MPU6050_ADDRESS = 0x68;
constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;

constexpr uint8_t POWER_MANAGEMENT_REGISTER = 0x6B;
constexpr uint8_t ACCEL_DATA_REGISTER = 0x3B;

void writeRegister(uint8_t registerAddress, uint8_t value) {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);
  Wire.write(value);
  Wire.endTransmission();
}

bool readMotionData(
  int16_t &accelX,
  int16_t &accelY,
  int16_t &accelZ,
  int16_t &gyroX,
  int16_t &gyroY,
  int16_t &gyroZ
) {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(ACCEL_DATA_REGISTER);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  const uint8_t bytesRequested = 14;
  const uint8_t bytesReceived =
    Wire.requestFrom(MPU6050_ADDRESS, bytesRequested, true);

  if (bytesReceived != bytesRequested) {
    return false;
  }

  accelX = (Wire.read() << 8) | Wire.read();
  accelY = (Wire.read() << 8) | Wire.read();
  accelZ = (Wire.read() << 8) | Wire.read();

  Wire.read();
  Wire.read();

  gyroX = (Wire.read() << 8) | Wire.read();
  gyroY = (Wire.read() << 8) | Wire.read();
  gyroZ = (Wire.read() << 8) | Wire.read();

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);

  writeRegister(POWER_MANAGEMENT_REGISTER, 0x00);
  delay(100);

  Serial.println();
  Serial.println("MPU6050 motion data test");
}

void loop() {
  int16_t rawAccelX;
  int16_t rawAccelY;
  int16_t rawAccelZ;
  int16_t rawGyroX;
  int16_t rawGyroY;
  int16_t rawGyroZ;

  if (!readMotionData(
        rawAccelX,
        rawAccelY,
        rawAccelZ,
        rawGyroX,
        rawGyroY,
        rawGyroZ
      )) {
    Serial.println("Error reading MPU6050 data.");
    delay(500);
    return;
  }

  const float accelX = rawAccelX / 16384.0f;
  const float accelY = rawAccelY / 16384.0f;
  const float accelZ = rawAccelZ / 16384.0f;

  const float gyroX = rawGyroX / 131.0f;
  const float gyroY = rawGyroY / 131.0f;
  const float gyroZ = rawGyroZ / 131.0f;

  Serial.print("AX: ");
  Serial.print(accelX, 3);
  Serial.print(" g | AY: ");
  Serial.print(accelY, 3);
  Serial.print(" g | AZ: ");
  Serial.print(accelZ, 3);

  Serial.print(" g | GX: ");
  Serial.print(gyroX, 2);
  Serial.print(" deg/s | GY: ");
  Serial.print(gyroY, 2);
  Serial.print(" deg/s | GZ: ");
  Serial.print(gyroZ, 2);
  Serial.println(" deg/s");

  delay(200);
}