#include <Wire.h>

constexpr uint8_t MPU6050_ADDRESS = 0x68;
constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;

constexpr uint8_t WHO_AM_I_REGISTER = 0x75;
constexpr uint8_t POWER_MANAGEMENT_REGISTER = 0x6B;
constexpr uint8_t ACCEL_X_HIGH_REGISTER = 0x3B;

uint8_t readRegister(uint8_t registerAddress) {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);

  if (Wire.endTransmission(false) != 0) {
    return 0xFF;
  }

  if (Wire.requestFrom(MPU6050_ADDRESS, static_cast<uint8_t>(1), true) != 1) {
    return 0xFF;
  }

  return Wire.read();
}

bool writeRegister(uint8_t registerAddress, uint8_t value) {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}

void printHexValue(const char* label, uint8_t value) {
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

  Serial.println();
  Serial.println("MPU6050 register diagnostic");

  const uint8_t whoAmI = readRegister(WHO_AM_I_REGISTER);
  const uint8_t powerBefore = readRegister(POWER_MANAGEMENT_REGISTER);

  printHexValue("WHO_AM_I before wake-up: ", whoAmI);
  printHexValue("PWR_MGMT_1 before wake-up: ", powerBefore);

  const bool writeSuccessful =
    writeRegister(POWER_MANAGEMENT_REGISTER, 0x00);

  Serial.print("Wake-up write result: ");
  Serial.println(writeSuccessful ? "success" : "failed");

  delay(100);

  const uint8_t powerAfter = readRegister(POWER_MANAGEMENT_REGISTER);
  printHexValue("PWR_MGMT_1 after wake-up: ", powerAfter);

  Serial.println("Raw accelerometer register bytes:");
}

void loop() {
  for (uint8_t index = 0; index < 6; index++) {
    const uint8_t value =
      readRegister(ACCEL_X_HIGH_REGISTER + index);

    Serial.print("0x");

    if (value < 0x10) {
      Serial.print("0");
    }

    Serial.print(value, HEX);
    Serial.print(index < 5 ? " " : "\n");
  }

  delay(500);
}