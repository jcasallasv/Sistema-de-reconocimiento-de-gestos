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

constexpr int32_t MIN_ACCEL_PEAK_TO_PEAK = 1500;
constexpr int32_t MIN_GYRO_PEAK_TO_PEAK = 500;

struct MotionSample
{
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;
};

struct InferenceResult
{
  bool success;
  String gesture;
  float confidence;
  float probabilityLeft;
  float probabilityRight;
  float probabilityUp;
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

const char *gestureToReferenceLabel(int gesture)
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

void showPredictionWithLed(const String &gesture)
{
  if (gesture == "left")
  {
    blinkLed(1, 400, 150);
    return;
  }

  if (gesture == "up")
  {
    blinkLed(2, 250, 150);
    return;
  }

  if (gesture == "right")
  {
    blinkLed(3, 180, 150);
    return;
  }

  blinkLed(6, 80, 80);
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

  int32_t maximumAccelPeakToPeak = 0;
  int32_t maximumGyroPeakToPeak = 0;

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

    if (
      channel < 3 &&
      peakToPeak > maximumAccelPeakToPeak
    )
    {
      maximumAccelPeakToPeak = peakToPeak;
    }

    if (
      channel >= 3 &&
      peakToPeak > maximumGyroPeakToPeak
    )
    {
      maximumGyroPeakToPeak = peakToPeak;
    }
  }

  const bool validMotionDetected =
    maximumAccelPeakToPeak >= MIN_ACCEL_PEAK_TO_PEAK
    || maximumGyroPeakToPeak >= MIN_GYRO_PEAK_TO_PEAK;

  Serial.printf(
    "Maximum accelerometer peak-to-peak: %ld\n",
    static_cast<long>(maximumAccelPeakToPeak)
  );

  Serial.printf(
    "Maximum gyroscope peak-to-peak: %ld\n",
    static_cast<long>(maximumGyroPeakToPeak)
  );

  Serial.printf(
    "Motion thresholds: accel >= %ld OR gyro >= %ld\n",
    static_cast<long>(MIN_ACCEL_PEAK_TO_PEAK),
    static_cast<long>(MIN_GYRO_PEAK_TO_PEAK)
  );

  Serial.print("Motion validation: ");
  Serial.println(
    validMotionDetected
      ? "ACCEPTED"
      : "REJECTED"
  );

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

String buildGestureApiUrl()
{
  String gestureUrl = API_URL;

  const int apiPathPosition =
    gestureUrl.indexOf("/api/");

  if (apiPathPosition >= 0)
  {
    gestureUrl =
      gestureUrl.substring(0, apiPathPosition) +
      "/api/gesture";
  }
  else
  {
    if (gestureUrl.endsWith("/"))
    {
      gestureUrl.remove(gestureUrl.length() - 1);
    }

    gestureUrl += "/api/gesture";
  }

  return gestureUrl;
}

bool buildInferencePayload(String &jsonBody)
{
  jsonBody = "";
  jsonBody.reserve(16000);

  jsonBody += "{\"samples\":[";

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

bool extractJsonBoolean(
  const String &jsonText,
  const char *key,
  bool &value
)
{
  String keyToken = "\"";
  keyToken += key;
  keyToken += "\":";

  int valuePosition =
    jsonText.indexOf(keyToken);

  if (valuePosition < 0)
  {
    return false;
  }

  valuePosition += keyToken.length();

  while (
    valuePosition < jsonText.length() &&
    isspace(
      static_cast<unsigned char>(
        jsonText[valuePosition]
      )
    )
  )
  {
    valuePosition++;
  }

  if (
    jsonText.substring(
      valuePosition,
      valuePosition + 4
    ) == "true"
  )
  {
    value = true;
    return true;
  }

  if (
    jsonText.substring(
      valuePosition,
      valuePosition + 5
    ) == "false"
  )
  {
    value = false;
    return true;
  }

  return false;
}

bool extractJsonString(
  const String &jsonText,
  const char *key,
  String &value
)
{
  String keyToken = "\"";
  keyToken += key;
  keyToken += "\":";

  const int keyPosition =
    jsonText.indexOf(keyToken);

  if (keyPosition < 0)
  {
    return false;
  }

  const int openingQuotePosition =
    jsonText.indexOf(
      '"',
      keyPosition + keyToken.length()
    );

  if (openingQuotePosition < 0)
  {
    return false;
  }

  const int closingQuotePosition =
    jsonText.indexOf(
      '"',
      openingQuotePosition + 1
    );

  if (closingQuotePosition < 0)
  {
    return false;
  }

  value = jsonText.substring(
    openingQuotePosition + 1,
    closingQuotePosition
  );

  return true;
}

bool extractJsonFloat(
  const String &jsonText,
  const char *key,
  float &value
)
{
  String keyToken = "\"";
  keyToken += key;
  keyToken += "\":";

  int valueStartPosition =
    jsonText.indexOf(keyToken);

  if (valueStartPosition < 0)
  {
    return false;
  }

  valueStartPosition += keyToken.length();

  while (
    valueStartPosition < jsonText.length() &&
    isspace(
      static_cast<unsigned char>(
        jsonText[valueStartPosition]
      )
    )
  )
  {
    valueStartPosition++;
  }

  int valueEndPosition = valueStartPosition;

  while (valueEndPosition < jsonText.length())
  {
    const char currentCharacter =
      jsonText[valueEndPosition];

    const bool numericCharacter =
      (
        currentCharacter >= '0' &&
        currentCharacter <= '9'
      )
      || currentCharacter == '-'
      || currentCharacter == '+'
      || currentCharacter == '.'
      || currentCharacter == 'e'
      || currentCharacter == 'E';

    if (!numericCharacter)
    {
      break;
    }

    valueEndPosition++;
  }

  if (valueEndPosition <= valueStartPosition)
  {
    return false;
  }

  value = jsonText.substring(
    valueStartPosition,
    valueEndPosition
  ).toFloat();

  return true;
}

bool parseInferenceResponse(
  const String &responseBody,
  InferenceResult &result
)
{
  result.success = false;
  result.gesture = "";
  result.confidence = 0.0F;
  result.probabilityLeft = 0.0F;
  result.probabilityRight = 0.0F;
  result.probabilityUp = 0.0F;

  bool responseSuccess = false;

  if (
    !extractJsonBoolean(
      responseBody,
      "success",
      responseSuccess
    )
  )
  {
    return false;
  }

  if (!responseSuccess)
  {
    return false;
  }

  if (
    !extractJsonString(
      responseBody,
      "gesture",
      result.gesture
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "confidence",
      result.confidence
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "left",
      result.probabilityLeft
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "right",
      result.probabilityRight
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "up",
      result.probabilityUp
    )
  )
  {
    return false;
  }

  result.success = true;

  return true;
}

bool sendInferenceCapture(
  InferenceResult &result
)
{
  if (!connectToWiFi())
  {
    return false;
  }

  String jsonBody;

  if (!buildInferencePayload(jsonBody))
  {
    Serial.println(
      "ERROR: Inference JSON payload could not be built."
    );

    return false;
  }

  const String gestureApiUrl =
    buildGestureApiUrl();

  Serial.print("Gesture API URL: ");
  Serial.println(gestureApiUrl);

  Serial.print("JSON payload size: ");
  Serial.print(jsonBody.length());
  Serial.println(" bytes");

  Serial.println(
    "Sending capture to Raspberry Pi for inference..."
  );

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, gestureApiUrl))
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
      "ERROR: Raspberry Pi rejected the inference request."
    );

    return false;
  }

  if (!parseInferenceResponse(responseBody, result))
  {
    Serial.println(
      "ERROR: Raspberry Pi response could not be parsed."
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

void printInferenceResult(
  const InferenceResult &result,
  const char *referenceLabel
)
{
  Serial.println();
  Serial.println("CNN INFERENCE RESULT");

  Serial.print("Predicted gesture: ");
  Serial.println(result.gesture);

  Serial.printf(
    "Confidence: %.4f\n",
    result.confidence
  );

  Serial.println("Probabilities:");

  Serial.printf(
    "  left: %.4f\n",
    result.probabilityLeft
  );

  Serial.printf(
    "  right: %.4f\n",
    result.probabilityRight
  );

  Serial.printf(
    "  up: %.4f\n",
    result.probabilityUp
  );

  Serial.print("APDS reference comparison: ");

  if (referenceLabel == nullptr)
  {
    Serial.println("unavailable");
    return;
  }

  if (result.gesture == referenceLabel)
  {
    Serial.println("MATCH");
  }
  else
  {
    Serial.println("MISMATCH");
  }
}

void runCapture()
{
  Serial.println();
  Serial.println("Prepare the gesture.");

  blinkLed(3, 150, 150);

  // Do not call readGesture() before capture. The SparkFun APDS-9960
  // routine can block while waiting for a gesture sequence to finish.
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

  const char *referenceLabel =
    gestureToReferenceLabel(detectedGesture);

  Serial.printf(
    "Detected APDS-9960 reference gesture: %s\n",
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
      "CAPTURE REJECTED: MPU6050 motion was below the minimum thresholds."
    );

    Serial.println(
      "Nothing was sent to the Raspberry Pi."
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Move the MPU6050 clearly and press the button to try again."
    );

    return;
  }

  if (referenceLabel == nullptr)
  {
    Serial.println();
    Serial.println(
      "APDS reference is unavailable, but CNN inference will continue."
    );
  }
  else
  {
    Serial.println();
    Serial.printf(
      "APDS reference label: %s\n",
      referenceLabel
    );
  }

  InferenceResult inferenceResult;

  if (!sendInferenceCapture(inferenceResult))
  {
    Serial.println();
    Serial.println(
      "INFERENCE FAILED: Communication with the Raspberry Pi failed."
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Press the button to try another capture."
    );

    return;
  }

  printInferenceResult(
    inferenceResult,
    referenceLabel
  );

  showPredictionWithLed(
    inferenceResult.gesture
  );

  Serial.println();
  Serial.println(
    "INFERENCE COMPLETED SUCCESSFULLY."
  );

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
    "WiFi gesture inference system"
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
    "Press the button to start an inference capture."
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