#include <Wire.h>
#include <SparkFun_APDS9960.h>

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint32_t I2C_CLOCK_HZ = 100000;

constexpr uint8_t MPU6050_ADDRESS = 0x68;
constexpr uint8_t MPU6050_SIGNAL_PATH_RESET = 0x68;
constexpr uint8_t MPU6050_PWR_MGMT_1 = 0x6B;
constexpr uint8_t MPU6050_PWR_MGMT_2 = 0x6C;
constexpr uint8_t MPU6050_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t MPU6050_WHO_AM_I = 0x75;

constexpr uint8_t BUTTON_PIN = 27;
constexpr uint8_t LED_PIN = 26;

constexpr size_t SAMPLE_COUNT = 100;
constexpr uint32_t SAMPLE_INTERVAL_US = 20000;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 500;
constexpr uint32_t GESTURE_DEBOUNCE_MS = 800;

struct MotionSample
{
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;
};

SparkFun_APDS9960 apds9960;
MotionSample samples[SAMPLE_COUNT];

volatile bool captureRequested = false;

String pendingLabel = "";
uint32_t lastCaptureMs = 0;
uint32_t lastGestureMs = 0;

void IRAM_ATTR handleButtonInterrupt()
{
  captureRequested = true;
}

bool writeMpuRegister(uint8_t registerAddress, uint8_t value)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}

bool readMpuRegister(uint8_t registerAddress, uint8_t &value)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  if (Wire.requestFrom(
        MPU6050_ADDRESS,
        static_cast<uint8_t>(1)) != 1)
  {
    return false;
  }

  value = Wire.read();
  return true;
}

bool initializeMpu6050()
{
  if (!writeMpuRegister(MPU6050_PWR_MGMT_1, 0x80))
  {
    return false;
  }

  delay(100);

  if (!writeMpuRegister(MPU6050_SIGNAL_PATH_RESET, 0x07))
  {
    return false;
  }

  delay(100);

  if (!writeMpuRegister(MPU6050_PWR_MGMT_1, 0x01))
  {
    return false;
  }

  delay(20);

  if (!writeMpuRegister(MPU6050_PWR_MGMT_2, 0x00))
  {
    return false;
  }

  delay(20);

  uint8_t deviceId = 0;

  if (!readMpuRegister(MPU6050_WHO_AM_I, deviceId))
  {
    return false;
  }

  Serial.printf("MPU6050 device ID: 0x%02X\n", deviceId);

  return deviceId != 0x00 && deviceId != 0xFF;
}

bool readMpuSample(MotionSample &sample)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(MPU6050_ACCEL_XOUT_H);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  if (Wire.requestFrom(
        MPU6050_ADDRESS,
        static_cast<uint8_t>(14)) != 14)
  {
    return false;
  }

  sample.ax = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
  sample.ay = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
  sample.az = static_cast<int16_t>((Wire.read() << 8) | Wire.read());

  Wire.read();
  Wire.read();

  sample.gx = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
  sample.gy = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
  sample.gz = static_cast<int16_t>((Wire.read() << 8) | Wire.read());

  return true;
}

String gestureToDatasetLabel(int gesture)
{
  switch (gesture)
  {
    case DIR_LEFT:
      return "left";

    case DIR_RIGHT:
      return "right";

    case DIR_UP:
      return "up";

    default:
      return "";
  }
}

void blinkLed(uint8_t repetitions, uint16_t onTimeMs, uint16_t offTimeMs)
{
  for (uint8_t count = 0; count < repetitions; count++)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(onTimeMs);

    digitalWrite(LED_PIN, LOW);
    delay(offTimeMs);
  }
}

void discardPendingGestures()
{
  for (uint8_t attempt = 0; attempt < 5; attempt++)
  {
    if (!apds9960.isGestureAvailable())
    {
      return;
    }

    apds9960.readGesture();
    delay(20);
  }
}

void printPeakToPeakValues()
{
  int16_t minimumValues[6] = {
    samples[0].ax,
    samples[0].ay,
    samples[0].az,
    samples[0].gx,
    samples[0].gy,
    samples[0].gz
  };

  int16_t maximumValues[6] = {
    samples[0].ax,
    samples[0].ay,
    samples[0].az,
    samples[0].gx,
    samples[0].gy,
    samples[0].gz
  };

  for (size_t index = 1; index < SAMPLE_COUNT; index++)
  {
    const int16_t values[6] = {
      samples[index].ax,
      samples[index].ay,
      samples[index].az,
      samples[index].gx,
      samples[index].gy,
      samples[index].gz
    };

    for (uint8_t channel = 0; channel < 6; channel++)
    {
      if (values[channel] < minimumValues[channel])
      {
        minimumValues[channel] = values[channel];
      }

      if (values[channel] > maximumValues[channel])
      {
        maximumValues[channel] = values[channel];
      }
    }
  }

  const char *channelNames[6] = {
    "AX", "AY", "AZ", "GX", "GY", "GZ"
  };

  Serial.println("MPU6050 peak-to-peak values:");

  for (uint8_t channel = 0; channel < 6; channel++)
  {
    const int32_t peakToPeak =
      static_cast<int32_t>(maximumValues[channel]) -
      static_cast<int32_t>(minimumValues[channel]);

    Serial.printf(
      "  %s: %ld\n",
      channelNames[channel],
      static_cast<long>(peakToPeak)
    );
  }
}

void armReferenceLabel(const String &label)
{
  pendingLabel = label;

  Serial.println();
  Serial.printf(
    "REFERENCE LABEL READY: %s\n",
    pendingLabel.c_str()
  );

  Serial.println(
    "Press the button and perform the same gesture with the MPU6050."
  );

  digitalWrite(LED_PIN, HIGH);
}

void runCapture()
{
  if (pendingLabel.isEmpty())
  {
    Serial.println();
    Serial.println(
      "BUTTON IGNORED: No APDS-9960 reference label is ready."
    );

    Serial.println(
      "Perform LEFT, RIGHT or UP in front of the APDS-9960 first."
    );

    return;
  }

  const String captureLabel = pendingLabel;

  Serial.println();
  Serial.printf(
    "PREPARE CAPTURE FOR LABEL: %s\n",
    captureLabel.c_str()
  );

  digitalWrite(LED_PIN, LOW);
  blinkLed(3, 150, 150);

  discardPendingGestures();
  delay(100);

  Serial.println("CAPTURE START");
  digitalWrite(LED_PIN, HIGH);

  const uint32_t captureStartUs = micros();
  uint32_t nextSampleTimeUs = captureStartUs;

  for (size_t index = 0; index < SAMPLE_COUNT; index++)
  {
    while (
      static_cast<int32_t>(micros() - nextSampleTimeUs) < 0
    )
    {
      delayMicroseconds(100);
    }

    if (!readMpuSample(samples[index]))
    {
      digitalWrite(LED_PIN, LOW);

      Serial.printf(
        "ERROR: MPU6050 read failed at sample %u.\n",
        static_cast<unsigned int>(index)
      );

      return;
    }

    nextSampleTimeUs += SAMPLE_INTERVAL_US;
  }

  const uint32_t captureDurationUs =
    micros() - captureStartUs;

  digitalWrite(LED_PIN, LOW);

  Serial.println("CAPTURE COMPLETED");

  Serial.printf(
    "Reference label: %s\n",
    captureLabel.c_str()
  );

  Serial.printf(
    "Captured samples: %u\n",
    static_cast<unsigned int>(SAMPLE_COUNT)
  );

  Serial.printf(
    "Capture duration: %.1f ms\n",
    captureDurationUs / 1000.0
  );

  printPeakToPeakValues();

  pendingLabel = "";

  Serial.println();
  Serial.println("Capture accepted.");
  Serial.println(
    "Perform another APDS-9960 gesture to prepare the next label."
  );
}

void checkForReferenceGesture()
{
  if (!pendingLabel.isEmpty())
  {
    return;
  }

  if (millis() - lastGestureMs < GESTURE_DEBOUNCE_MS)
  {
    return;
  }

  if (!apds9960.isGestureAvailable())
  {
    return;
  }

  const int detectedGesture = apds9960.readGesture();
  const String detectedLabel =
    gestureToDatasetLabel(detectedGesture);

  lastGestureMs = millis();

  if (detectedLabel.isEmpty())
  {
    Serial.println(
      "APDS-9960 gesture ignored. Use LEFT, RIGHT or UP."
    );

    return;
  }

  armReferenceLabel(detectedLabel);
}

void setup()
{
  Serial.begin(115200);
  delay(1500);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);

  Serial.println();
  Serial.println("Two-stage gesture dataset capture test");

  if (!initializeMpu6050())
  {
    Serial.println("ERROR: MPU6050 initialization failed.");

    while (true)
    {
      delay(1000);
    }
  }

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
    Serial.println("ERROR: APDS-9960 gesture engine failed.");

    while (true)
    {
      delay(1000);
    }
  }

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );

  Serial.println("Both sensors initialized.");
  Serial.println(
    "Perform LEFT, RIGHT or UP in front of the APDS-9960."
  );
}

void loop()
{
  checkForReferenceGesture();

  if (!captureRequested)
  {
    delay(5);
    return;
  }

  noInterrupts();
  captureRequested = false;
  interrupts();

  if (millis() - lastCaptureMs < BUTTON_DEBOUNCE_MS)
  {
    return;
  }

  lastCaptureMs = millis();
  runCapture();
}