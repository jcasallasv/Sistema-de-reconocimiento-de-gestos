#include <Wire.h>

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint32_t I2C_CLOCK_HZ = 100000;

void scanI2CBus()
{
  uint8_t detectedDevices = 0;

  Serial.println();
  Serial.println("Scanning I2C bus...");

  for (uint8_t address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    const uint8_t error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.printf("I2C device detected at address 0x%02X\n", address);
      detectedDevices++;
    }
    else if (error == 4)
    {
      Serial.printf("Unknown I2C error at address 0x%02X\n", address);
    }
  }

  Serial.printf("Detected devices: %u\n", detectedDevices);
  Serial.println("Scan completed.");
}

void setup()
{
  Serial.begin(115200);
  delay(1500);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);

  scanI2CBus();
}

void loop()
{
}