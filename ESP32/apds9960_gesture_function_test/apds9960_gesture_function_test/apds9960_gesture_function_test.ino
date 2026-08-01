#include <Wire.h>
#include <SparkFun_APDS9960.h>

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint32_t I2C_CLOCK_HZ = 100000;

SparkFun_APDS9960 apds9960;

void printGesture(int gesture)
{
  switch (gesture)
  {
    case DIR_UP:
      Serial.println("GESTURE: UP");
      break;

    case DIR_DOWN:
      Serial.println("GESTURE: DOWN");
      break;

    case DIR_LEFT:
      Serial.println("GESTURE: LEFT");
      break;

    case DIR_RIGHT:
      Serial.println("GESTURE: RIGHT");
      break;

    case DIR_NEAR:
      Serial.println("GESTURE: NEAR");
      break;

    case DIR_FAR:
      Serial.println("GESTURE: FAR");
      break;

    default:
      Serial.println("GESTURE: UNKNOWN");
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("APDS-9960 gesture function test");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);

  if (!apds9960.init())
  {
    Serial.println("ERROR: APDS-9960 initialization failed.");

    while (true)
    {
      delay(1000);
    }
  }

  Wire.setClock(I2C_CLOCK_HZ);

  if (!apds9960.enableGestureSensor(false))
  {
    Serial.println("ERROR: Gesture engine could not be enabled.");

    while (true)
    {
      delay(1000);
    }
  }

  Serial.println("APDS-9960 initialized.");
  Serial.println("Gesture engine ready.");
  Serial.println("Move your hand across the front of the sensor.");
}

void loop()
{
  if (apds9960.isGestureAvailable())
  {
    const int gesture = apds9960.readGesture();
    printGesture(gesture);
  }

  delay(20);
}