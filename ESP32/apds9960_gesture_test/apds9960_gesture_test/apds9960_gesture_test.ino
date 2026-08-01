#include <Wire.h>

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint32_t I2C_CLOCK_HZ = 100000;

constexpr uint8_t APDS9960_ADDRESS = 0x39;
constexpr uint8_t APDS9960_ID_REGISTER = 0x92;

bool readRegister(uint8_t deviceAddress, uint8_t registerAddress, uint8_t &value)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(registerAddress);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  if (Wire.requestFrom(deviceAddress, static_cast<uint8_t>(1)) != 1)
  {
    return false;
  }

  value = Wire.read();
  return true;
}

void setup()
{
  Serial.begin(115200);
  delay(1500);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);

  Serial.println();
  Serial.println("APDS-9960 identification test");

  uint8_t deviceId = 0;

  if (!readRegister(APDS9960_ADDRESS, APDS9960_ID_REGISTER, deviceId))
  {
    Serial.println("ERROR: Could not read APDS-9960 ID register.");
    return;
  }

  Serial.printf("APDS-9960 I2C address: 0x%02X\n", APDS9960_ADDRESS);
  Serial.printf("APDS-9960 device ID: 0x%02X\n", deviceId);

  if (deviceId == 0xAB)
  {
    Serial.println("Device type: Standard APDS-9960");
  }
  else if (deviceId == 0x9C)
  {
    Serial.println("Device type: Compatible APDS-9960 variant");
  }
  else if (deviceId == 0xA8)
  {
    Serial.println("Device type: APDS-9960 clone variant");
  }
  else
  {
    Serial.println("Device type: Unknown compatible device");
  }

  Serial.println("Identification test completed.");
}

void loop()
{
}