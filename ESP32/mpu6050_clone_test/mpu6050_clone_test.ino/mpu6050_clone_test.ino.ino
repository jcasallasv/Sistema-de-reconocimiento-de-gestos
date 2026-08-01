#include <Wire.h>

constexpr uint8_t MPU_ADDRESS = 0x68;
constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;

constexpr uint8_t SIGNAL_PATH_RESET = 0x68;
constexpr uint8_t POWER_MANAGEMENT_1 = 0x6B;
constexpr uint8_t POWER_MANAGEMENT_2 = 0x6C;
constexpr uint8_t WHO_AM_I = 0x75;
constexpr uint8_t ACCEL_DATA_START = 0x3B;

bool writeRegister(uint8_t registerAddress, uint8_t value) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(registerAddress);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}

uint8_t readRegister(uint8_t registerAddress) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(registerAddress);

  if (Wire.endTransmission(false) != 0) {
    return 0xFF;
  }

  if (Wire.requestFrom(
        MPU_ADDRESS,
        static_cast<uint8_t>(1),
        true
      ) != 1) {
    return 0xFF;
  }

  return Wire.read();
}

bool readMotionData(
  int16_t &accelX,
  int16_t &accelY,
  int16_t &accelZ,
  int16_t &gyroX,
  int16_t &gyroY,
  int16_t &gyroZ
) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(ACCEL_DATA_START);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(
        MPU_ADDRESS,
        static_cast<uint8_t>(14),
        true
      ) != 14) {
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

void printHex(const char *label, uint8_t value) {
  Serial.print(label);
  Serial.print("0x");

  if (value < 0x10) {
    Serial.print("0");
  }

  Serial.println(value, HEX);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  Serial.println();
  Serial.println("MPU6050 clone initialization test");

  printHex("WHO_AM_I: ", readRegister(WHO_AM_I));

  printHex(
    "PWR_MGMT_1 before initialization: ",
    readRegister(POWER_MANAGEMENT_1)
  );

  writeRegister(POWER_MANAGEMENT_1, 0x80);
  delay(150);

  writeRegister(SIGNAL_PATH_RESET, 0x07);
  delay(150);

  writeRegister(POWER_MANAGEMENT_1, 0x01);
  writeRegister(POWER_MANAGEMENT_2, 0x00);
  delay(150);

  printHex(
    "PWR_MGMT_1 after initialization: ",
    readRegister(POWER_MANAGEMENT_1)
  );

  Serial.println("Motion data:");
}

void loop() {
  int16_t accelX;
  int16_t accelY;
  int16_t accelZ;
  int16_t gyroX;
  int16_t gyroY;
  int16_t gyroZ;

  if (!readMotionData(
        accelX,
        accelY,
        accelZ,
        gyroX,
        gyroY,
        gyroZ
      )) {
    Serial.println("Motion data read failed.");
    delay(500);
    return;
  }

  Serial.print("AX: ");
  Serial.print(accelX);
  Serial.print(" | AY: ");
  Serial.print(accelY);
  Serial.print(" | AZ: ");
  Serial.print(accelZ);
  Serial.print(" | GX: ");
  Serial.print(gyroX);
  Serial.print(" | GY: ");
  Serial.print(gyroY);
  Serial.print(" | GZ: ");
  Serial.println(gyroZ);

  delay(200);
}