#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>

#include "secrets.h"

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
constexpr uint32_t WIFI_CONNECTION_TIMEOUT_MS = 20000;
constexpr uint32_t HTTP_REQUEST_TIMEOUT_MS = 20000;
constexpr uint32_t GESTURE_WAIT_TIMEOUT_MS = 1500;

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

uint32_t lastCaptureMs = 0;

void IRAM_ATTR handleButtonInterrupt()
{
  captureRequested = true;
}

bool writeMpuRegister(
  uint8_t registerAddress,
  uint8_t value
)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}

bool readMpuRegister(
  uint8_t registerAddress,
  uint8_t &value
)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  if (
    Wire.requestFrom(
      MPU6050_ADDRESS,
      static_cast<uint8_t>(1)
    ) != 1
  )
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

  if (
    !writeMpuRegister(
      MPU6050_SIGNAL_PATH_RESET,
      0x07
    )
  )
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

  Serial.printf(
    "MPU6050 device ID: 0x%02X\n",
    deviceId
  );

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

  if (
    Wire.requestFrom(
      MPU6050_ADDRESS,
      static_cast<uint8_t>(14)
    ) != 14
  )
  {
    return false;
  }

  sample.ax =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.ay =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.az =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  Wire.read();
  Wire.read();

  sample.gx =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.gy =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.gz =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  return true;
}

const char *gestureToDatasetLabel(int gesture)
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
      return nullptr;
  }
}

const char *gestureToText(int gesture)
{
  switch (gesture)
  {
    case DIR_LEFT:
      return "left";

    case DIR_RIGHT:
      return "right";

    case DIR_UP:
      return "up";

    case DIR_DOWN:
      return "down";

    case DIR_NEAR:
      return "near";

    case DIR_FAR:
      return "far";

    default:
      return "none";
  }
}

void blinkLed(
  uint8_t repetitions,
  uint16_t onTimeMs,
  uint16_t offTimeMs
)
{
  for (
    uint8_t repetition = 0;
    repetition < repetitions;
    repetition++
  )
  {
    digitalWrite(LED_PIN, HIGH);
    delay(onTimeMs);

    digitalWrite(LED_PIN, LOW);
    delay(offTimeMs);
  }
}

void discardPendingGestures()
{
  for (
    uint8_t attempt = 0;
    attempt < 5;
    attempt++
  )
  {
    if (!apds9960.isGestureAvailable())
    {
      return;
    }

    apds9960.readGesture();
    delay(20);
  }
}

bool printAndValidatePeakToPeakValues()
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

  for (
    size_t sampleIndex = 1;
    sampleIndex < SAMPLE_COUNT;
    sampleIndex++
  )
  {
    const int16_t values[6] = {
      samples[sampleIndex].ax,
      samples[sampleIndex].ay,
      samples[sampleIndex].az,
      samples[sampleIndex].gx,
      samples[sampleIndex].gy,
      samples[sampleIndex].gz
    };

    for (
      uint8_t channel = 0;
      channel < 6;
      channel++
    )
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
    "AX",
    "AY",
    "AZ",
    "GX",
    "GY",
    "GZ"
  };

  bool validMotionDetected = false;

  Serial.println("MPU6050 peak-to-peak values:");

  for (
    uint8_t channel = 0;
    channel < 6;
    channel++
  )
  {
    const int32_t peakToPeak =
      static_cast<int32_t>(maximumValues[channel]) -
      static_cast<int32_t>(minimumValues[channel]);

    Serial.printf(
      "  %s: %ld\n",
      channelNames[channel],
      static_cast<long>(peakToPeak)
    );

    if (peakToPeak != 0)
    {
      validMotionDetected = true;
    }
  }

  return validMotionDetected;
}

bool connectToWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }

  Serial.print("Connecting to WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t connectionStartMs = millis();

  while (WiFi.status() != WL_CONNECTED)
  {
    if (
      millis() - connectionStartMs >=
      WIFI_CONNECTION_TIMEOUT_MS
    )
    {
      Serial.println();
      Serial.println("ERROR: WiFi connection timeout.");

      return false;
    }

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected.");

  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  return true;
}

String buildDatasetApiUrl()
{
  String datasetUrl = API_URL;

  const int apiPathPosition =
    datasetUrl.indexOf("/api/");

  if (apiPathPosition >= 0)
  {
    datasetUrl =
      datasetUrl.substring(0, apiPathPosition) +
      "/api/dataset";
  }
  else
  {
    if (datasetUrl.endsWith("/"))
    {
      datasetUrl.remove(datasetUrl.length() - 1);
    }

    datasetUrl += "/api/dataset";
  }

  return datasetUrl;
}

bool buildDatasetPayload(
  const char *label,
  String &jsonBody
)
{
  if (label == nullptr)
  {
    return false;
  }

  jsonBody = "";
  jsonBody.reserve(16000);

  jsonBody += "{\"label\":\"";
  jsonBody += label;
  jsonBody += "\",\"samples\":[";

  for (
    size_t sampleIndex = 0;
    sampleIndex < SAMPLE_COUNT;
    sampleIndex++
  )
  {
    if (sampleIndex > 0)
    {
      jsonBody += ",";
    }

    jsonBody += "{\"ax\":";
    jsonBody += samples[sampleIndex].ax;

    jsonBody += ",\"ay\":";
    jsonBody += samples[sampleIndex].ay;

    jsonBody += ",\"az\":";
    jsonBody += samples[sampleIndex].az;

    jsonBody += ",\"gx\":";
    jsonBody += samples[sampleIndex].gx;

    jsonBody += ",\"gy\":";
    jsonBody += samples[sampleIndex].gy;

    jsonBody += ",\"gz\":";
    jsonBody += samples[sampleIndex].gz;

    jsonBody += "}";
  }

  jsonBody += "]}";

  return true;
}

bool sendDatasetCapture(const char *label)
{
  if (!connectToWiFi())
  {
    return false;
  }

  String jsonBody;

  if (!buildDatasetPayload(label, jsonBody))
  {
    Serial.println(
      "ERROR: Dataset JSON payload could not be built."
    );

    return false;
  }

  const String datasetApiUrl =
    buildDatasetApiUrl();

  Serial.print("Dataset API URL: ");
  Serial.println(datasetApiUrl);

  Serial.print("JSON payload size: ");
  Serial.print(jsonBody.length());
  Serial.println(" bytes");

  Serial.println(
    "Sending dataset capture to Raspberry Pi..."
  );

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, datasetApiUrl))
  {
    Serial.println(
      "ERROR: HTTP client initialization failed."
    );

    return false;
  }

  http.setTimeout(HTTP_REQUEST_TIMEOUT_MS);

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  const int httpResponseCode =
    http.POST(jsonBody);

  Serial.print("HTTP response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode <= 0)
  {
    Serial.print("HTTP request failed: ");
    Serial.println(
      http.errorToString(httpResponseCode)
    );

    http.end();

    return false;
  }

  const String responseBody =
    http.getString();

  http.end();

  Serial.println("Server response:");
  Serial.println(responseBody);

  if (
    httpResponseCode < 200 ||
    httpResponseCode >= 300
  )
  {
    Serial.println(
      "ERROR: Raspberry Pi rejected the dataset capture."
    );

    return false;
  }

  return true;
}

int readReferenceGesture()
{
  const uint32_t waitStartMs = millis();

  while (
    millis() - waitStartMs <
    GESTURE_WAIT_TIMEOUT_MS
  )
  {
    if (apds9960.isGestureAvailable())
    {
      return apds9960.readGesture();
    }

    delay(10);
  }

  return DIR_NONE;
}

void runCapture()
{
  Serial.println();
  Serial.println("Prepare the gesture.");

  blinkLed(3, 150, 150);

  discardPendingGestures();
  delay(100);

  Serial.println("CAPTURE START");

  digitalWrite(LED_PIN, HIGH);

  const uint32_t captureStartUs = micros();
  uint32_t nextSampleTimeUs = captureStartUs;

  for (
    size_t sampleIndex = 0;
    sampleIndex < SAMPLE_COUNT;
    sampleIndex++
  )
  {
    while (
      static_cast<int32_t>(
        micros() - nextSampleTimeUs
      ) < 0
    )
    {
      delayMicroseconds(100);
    }

    if (!readMpuSample(samples[sampleIndex]))
    {
      digitalWrite(LED_PIN, LOW);

      Serial.printf(
        "ERROR: MPU6050 read failed at sample %u.\n",
        static_cast<unsigned int>(sampleIndex)
      );

      return;
    }

    nextSampleTimeUs += SAMPLE_INTERVAL_US;
  }

  const uint32_t captureDurationUs =
    micros() - captureStartUs;

  digitalWrite(LED_PIN, LOW);

  Serial.println("CAPTURE COMPLETED");

  const int detectedGesture =
    readReferenceGesture();

  const char *detectedGestureText =
    gestureToText(detectedGesture);

  const char *datasetLabel =
    gestureToDatasetLabel(detectedGesture);

  Serial.printf(
    "Detected APDS-9960 gesture: %s\n",
    detectedGestureText
  );

  Serial.printf(
    "Captured samples: %u\n",
    static_cast<unsigned int>(SAMPLE_COUNT)
  );

  Serial.printf(
    "Capture duration: %.1f ms\n",
    captureDurationUs / 1000.0
  );

  const bool validMpuMotion =
    printAndValidatePeakToPeakValues();

  if (!validMpuMotion)
  {
    Serial.println();
    Serial.println(
      "CAPTURE REJECTED: MPU6050 returned zero peak-to-peak values."
    );

    Serial.println(
      "Nothing was sent to the Raspberry Pi."
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Check the MPU6050 wiring and press the button to try again."
    );

    return;
  }

  if (datasetLabel == nullptr)
  {
    Serial.println();
    Serial.println(
      "CAPTURE REJECTED: APDS-9960 did not detect LEFT, RIGHT or UP."
    );

    Serial.println(
      "Nothing was sent to the Raspberry Pi."
    );

    blinkLed(4, 80, 80);

    Serial.println();
    Serial.println(
      "Press the button to try another capture."
    );

    return;
  }

  Serial.println();
  Serial.printf(
    "Dataset label accepted: %s\n",
    datasetLabel
  );

  if (!sendDatasetCapture(datasetLabel))
  {
    Serial.println();
    Serial.println(
      "CAPTURE NOT SAVED: Communication with the Raspberry Pi failed."
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Press the button to try another capture."
    );

    return;
  }

  Serial.println();
  Serial.println(
    "DATASET CAPTURE SAVED SUCCESSFULLY."
  );

  blinkLed(2, 300, 150);

  Serial.println();
  Serial.println(
    "Press the button for another capture."
  );
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
  Serial.println(
    "WiFi gesture dataset collector"
  );

  if (!initializeMpu6050())
  {
    Serial.println(
      "ERROR: MPU6050 initialization failed."
    );

    while (true)
    {
      delay(1000);
    }
  }

  if (!apds9960.init())
  {
    Serial.println(
      "ERROR: APDS-9960 initialization failed."
    );

    while (true)
    {
      delay(1000);
    }
  }

  Wire.setClock(I2C_CLOCK_HZ);

  if (!apds9960.enableGestureSensor(false))
  {
    Serial.println(
      "ERROR: APDS-9960 gesture engine failed."
    );

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

  if (!connectToWiFi())
  {
    Serial.println(
      "WiFi is not available yet."
    );

    Serial.println(
      "The ESP32 will retry before sending a capture."
    );
  }

  Serial.println(
    "Press the button to start a simultaneous capture."
  );
}

void loop()
{
  if (!captureRequested)
  {
    delay(5);
    return;
  }

  noInterrupts();
  captureRequested = false;
  interrupts();

  if (
    millis() - lastCaptureMs <
    BUTTON_DEBOUNCE_MS
  )
  {
    return;
  }

  lastCaptureMs = millis();

  runCapture();
}