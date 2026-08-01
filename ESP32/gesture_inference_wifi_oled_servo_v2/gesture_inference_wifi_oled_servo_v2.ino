#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include <SparkFun_APDS9960.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

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

constexpr int16_t SCREEN_WIDTH = 128;
constexpr int16_t SCREEN_HEIGHT = 64;

constexpr int8_t OLED_MOSI_PIN = 23;
constexpr int8_t OLED_CLK_PIN = 18;
constexpr int8_t OLED_DC_PIN = 2;
constexpr int8_t OLED_CS_PIN = 5;
constexpr int8_t OLED_RESET_PIN = 4;

constexpr uint8_t SERVO_PIN = 13;
constexpr int SERVO_RIGHT_ANGLE = 0;
constexpr int SERVO_UP_ANGLE = 90;
constexpr int SERVO_LEFT_ANGLE = 180;
constexpr int SERVO_INITIAL_ANGLE = SERVO_UP_ANGLE;
constexpr uint16_t SERVO_STEP_DELAY_MS = 8;

constexpr uint32_t RESULT_SCREEN_TIME_MS = 2200;
constexpr uint32_t PROBABILITY_SCREEN_TIME_MS = 2200;
constexpr uint32_t MESSAGE_SCREEN_TIME_MS = 1800;

constexpr size_t SAMPLE_COUNT = 100;
constexpr uint32_t SAMPLE_INTERVAL_US = 20000;

constexpr uint32_t BUTTON_DEBOUNCE_MS = 350;
constexpr uint32_t BUTTON_RELEASE_STABLE_MS = 80;
constexpr uint8_t MPU_READ_MAX_ATTEMPTS = 4;
constexpr uint32_t MPU_READ_RETRY_DELAY_US = 500;
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

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  OLED_MOSI_PIN,
  OLED_CLK_PIN,
  OLED_DC_PIN,
  OLED_RESET_PIN,
  OLED_CS_PIN
);

Servo gestureServo;

bool displayAvailable = false;
bool servoAvailable = false;
int currentServoAngle = SERVO_INITIAL_ANGLE;

MotionSample samples[SAMPLE_COUNT];

volatile bool captureRequested = false;

uint32_t lastCaptureMs = 0;

void IRAM_ATTR handleButtonInterrupt()
{
  captureRequested = true;
}

void drawCenteredText(
  const String &text,
  int16_t y,
  uint8_t textSize
)
{
  if (!displayAvailable)
  {
    return;
  }

  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t width = 0;
  uint16_t height = 0;

  display.setTextSize(textSize);
  display.getTextBounds(
    text,
    0,
    y,
    &x1,
    &y1,
    &width,
    &height
  );

  int16_t x =
    (
      SCREEN_WIDTH -
      static_cast<int16_t>(width)
    ) / 2;

  if (x < 0)
  {
    x = 0;
  }

  display.setCursor(x, y);
  display.print(text);
}

void showStateScreen(
  const String &title,
  const String &line1 = "",
  const String &line2 = "",
  const String &line3 = ""
)
{
  if (!displayAvailable)
  {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  drawCenteredText(title, 2, 1);
  display.drawFastHLine(
    0,
    13,
    SCREEN_WIDTH,
    SSD1306_WHITE
  );

  drawCenteredText(line1, 20, 1);
  drawCenteredText(line2, 34, 1);
  drawCenteredText(line3, 48, 1);

  display.display();
}

void showReadyScreen()
{
  const String wifiLine =
    WiFi.status() == WL_CONNECTED
      ? "WiFi: connected"
      : "WiFi: retry later";

  showStateScreen(
    "SYSTEM READY",
    "Press the button",
    "to capture gesture",
    wifiLine
  );
}

void showPrepareCountdown()
{
  for (int countdown = 3; countdown >= 1; countdown--)
  {
    showStateScreen(
      "PREPARE GESTURE",
      "Starting in",
      String(countdown),
      "Move during capture"
    );

    digitalWrite(LED_PIN, HIGH);
    delay(150);

    digitalWrite(LED_PIN, LOW);
    delay(150);
  }

  delay(100);
}

void showResultSummary(
  const InferenceResult &result,
  int servoAngle
)
{
  if (!displayAvailable)
  {
    return;
  }

  String gestureText = result.gesture;
  gestureText.toUpperCase();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  drawCenteredText("GESTURE RESULT", 2, 1);
  display.drawFastHLine(
    0,
    13,
    SCREEN_WIDTH,
    SSD1306_WHITE
  );

  drawCenteredText(gestureText, 18, 2);

  drawCenteredText(
    String("Confidence: ") +
    String(result.confidence * 100.0F, 2) +
    "%",
    41,
    1
  );

  const String servoLine =
    servoAngle >= 0
      ? String("Servo: ") +
        String(servoAngle) +
        " deg"
      : String("Servo: no command");

  drawCenteredText(servoLine, 53, 1);

  display.display();
}

void showProbabilityScreen(
  const InferenceResult &result
)
{
  showStateScreen(
    "CNN PROBABILITIES",
    String("Left:  ") +
      String(result.probabilityLeft * 100.0F, 2) +
      "%",
    String("Right: ") +
      String(result.probabilityRight * 100.0F, 2) +
      "%",
    String("Up:    ") +
      String(result.probabilityUp * 100.0F, 2) +
      "%"
  );
}

int gestureToServoAngle(const String &gesture)
{
  if (gesture == "right")
  {
    return SERVO_RIGHT_ANGLE;
  }

  if (gesture == "up")
  {
    return SERVO_UP_ANGLE;
  }

  if (gesture == "left")
  {
    return SERVO_LEFT_ANGLE;
  }

  return -1;
}

int moveServoForGesture(const String &gesture)
{
  const int targetAngle =
    gestureToServoAngle(gesture);

  if (
    !servoAvailable ||
    targetAngle < 0
  )
  {
    return -1;
  }

  const int step =
    targetAngle >= currentServoAngle
      ? 1
      : -1;

  int angle = currentServoAngle;

  while (angle != targetAngle)
  {
    gestureServo.write(angle);
    angle += step;
    delay(SERVO_STEP_DELAY_MS);
  }

  gestureServo.write(targetAngle);
  currentServoAngle = targetAngle;

  return targetAngle;
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

bool readMpuSampleOnce(MotionSample &sample)
{
  while (Wire.available())
  {
    Wire.read();
  }

  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(MPU6050_ACCEL_XOUT_H);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  const size_t receivedBytes =
    Wire.requestFrom(
      MPU6050_ADDRESS,
      static_cast<uint8_t>(14)
    );

  if (receivedBytes != 14 || Wire.available() < 14)
  {
    while (Wire.available())
    {
      Wire.read();
    }

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

void recoverI2cController()
{
  while (Wire.available())
  {
    Wire.read();
  }

  Wire.end();
  delay(2);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);
}

bool readMpuSample(
  MotionSample &sample,
  uint8_t &attemptsUsed
)
{
  attemptsUsed = 0;

  for (
    uint8_t attempt = 1;
    attempt <= MPU_READ_MAX_ATTEMPTS;
    attempt++
  )
  {
    attemptsUsed = attempt;

    if (readMpuSampleOnce(sample))
    {
      return true;
    }

    if (attempt == 2)
    {
      recoverI2cController();
    }
    else
    {
      delayMicroseconds(MPU_READ_RETRY_DELAY_US);
    }
  }

  return false;
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

  showStateScreen(
    "CONNECTING WIFI",
    "Please wait...",
    "",
    ""
  );

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

      showStateScreen(
        "WIFI ERROR",
        "Connection timeout",
        "Check hotspot",
        "Retry on next capture"
      );

      return false;
    }

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected.");

  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  showStateScreen(
    "WIFI CONNECTED",
    "ESP32 IP:",
    WiFi.localIP().toString(),
    "Server: 172.20.10.7"
  );

  delay(700);

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

  showStateScreen(
    "SENDING DATA",
    "Raspberry Pi",
    "100 MPU samples",
    "HTTP request"
  );

  delay(250);

  showStateScreen(
    "CNN PROCESSING",
    "NumPy 1D CNN",
    "Waiting result",
    "Please wait..."
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

  showPrepareCountdown();

  // Do not call readGesture() before capture. The SparkFun APDS-9960
  // routine can block while waiting for a gesture sequence to finish.
  Serial.println("CAPTURE START");

  showStateScreen(
    "CAPTURING",
    "MPU6050 motion",
    "100 samples / 2 s",
    "Keep moving"
  );

  digitalWrite(LED_PIN, HIGH);

  const uint32_t captureStartUs = micros();
  uint32_t nextSampleTimeUs = captureStartUs;
  uint16_t recoveredMpuReadCount = 0;
  uint8_t maximumMpuReadAttempts = 1;

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

    uint8_t attemptsUsed = 0;

    if (!readMpuSample(samples[sampleIndex], attemptsUsed))
    {
      digitalWrite(LED_PIN, LOW);

      Serial.printf(
        "ERROR: MPU6050 read failed at sample %u after %u attempts.\n",
        static_cast<unsigned int>(sampleIndex),
        static_cast<unsigned int>(attemptsUsed)
      );

      showStateScreen(
        "SENSOR ERROR",
        "MPU6050 read",
        "Capture stopped",
        "Press to retry"
      );

      blinkLed(6, 80, 80);
      delay(MESSAGE_SCREEN_TIME_MS);
      showReadyScreen();

      return;
    }

    if (attemptsUsed > 1)
    {
      recoveredMpuReadCount++;

      if (attemptsUsed > maximumMpuReadAttempts)
      {
        maximumMpuReadAttempts = attemptsUsed;
      }
    }

    nextSampleTimeUs += SAMPLE_INTERVAL_US;
  }

  const uint32_t captureDurationUs =
    micros() - captureStartUs;

  digitalWrite(LED_PIN, LOW);

  Serial.println("CAPTURE COMPLETED");

  showStateScreen(
    "CAPTURE COMPLETE",
    "Validating motion",
    "Reading reference",
    "Please wait..."
  );

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

  Serial.printf(
    "Recovered MPU6050 reads: %u (maximum attempts used: %u)\n",
    static_cast<unsigned int>(recoveredMpuReadCount),
    static_cast<unsigned int>(maximumMpuReadAttempts)
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

    showStateScreen(
      "MOTION REJECTED",
      "Movement too low",
      "Move the sensor",
      "Press to retry"
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Move the MPU6050 clearly and press the button to try again."
    );

    delay(MESSAGE_SCREEN_TIME_MS);
    showReadyScreen();

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

    showStateScreen(
      "SERVER ERROR",
      "Inference failed",
      "Check WiFi / Pi",
      "Press to retry"
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Press the button to try another capture."
    );

    delay(MESSAGE_SCREEN_TIME_MS);
    showReadyScreen();

    return;
  }

  printInferenceResult(
    inferenceResult,
    referenceLabel
  );

  showStateScreen(
    "ACTUATING",
    String("Gesture: ") +
      inferenceResult.gesture,
    "Moving servo",
    "Updating outputs"
  );

  const int servoAngle =
    moveServoForGesture(
      inferenceResult.gesture
    );

  showResultSummary(
    inferenceResult,
    servoAngle
  );

  showPredictionWithLed(
    inferenceResult.gesture
  );

  delay(RESULT_SCREEN_TIME_MS);

  showProbabilityScreen(
    inferenceResult
  );

  delay(PROBABILITY_SCREEN_TIME_MS);

  Serial.println();
  Serial.println(
    "INFERENCE COMPLETED SUCCESSFULLY."
  );

  Serial.println();
  Serial.println(
    "Press the button for another capture."
  );

  showReadyScreen();
}

void setup()
{
  Serial.begin(115200);
  delay(1500);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  displayAvailable =
    display.begin(
      SSD1306_SWITCHCAPVCC
    );

  if (!displayAvailable)
  {
    Serial.println(
      "WARNING: OLED initialization failed. Core system will continue."
    );
  }
  else
  {
    display.clearDisplay();
    display.display();

    showStateScreen(
      "EMBEDDED GESTURE",
      "Booting system",
      "OLED interface: OK",
      "Please wait..."
    );
  }

  ESP32PWM::allocateTimer(0);
  gestureServo.setPeriodHertz(50);
  gestureServo.attach(
    SERVO_PIN,
    544,
    2400
  );

  servoAvailable =
    gestureServo.attached();

  if (!servoAvailable)
  {
    Serial.println(
      "WARNING: Servo attachment failed. Core system will continue."
    );
  }
  else
  {
    gestureServo.write(
      SERVO_INITIAL_ANGLE
    );

    currentServoAngle =
      SERVO_INITIAL_ANGLE;

    delay(350);
  }

  showStateScreen(
    "INITIALIZING",
    "MPU6050 + APDS",
    servoAvailable
      ? "Servo: OK"
      : "Servo: unavailable",
    "Please wait..."
  );

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

    showStateScreen(
      "SENSOR ERROR",
      "MPU6050 failed",
      "Check wiring",
      "Restart system"
    );

    while (true)
    {
      delay(1000);
    }
  }

  showStateScreen(
    "INITIALIZING",
    "MPU6050: OK",
    "APDS9960...",
    "Please wait..."
  );

  if (!apds9960.init())
  {
    Serial.println(
      "ERROR: APDS-9960 initialization failed."
    );

    showStateScreen(
      "SENSOR ERROR",
      "APDS9960 failed",
      "Check wiring",
      "Restart system"
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

    showStateScreen(
      "SENSOR ERROR",
      "APDS gesture failed",
      "Check library",
      "Restart system"
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

  showStateScreen(
    "SENSORS READY",
    "MPU6050: OK",
    "APDS9960: OK",
    "Connecting WiFi"
  );

  delay(500);

  if (!connectToWiFi())
  {
    Serial.println(
      "WiFi is not available yet."
    );

    Serial.println(
      "The ESP32 will retry before sending a capture."
    );

    delay(MESSAGE_SCREEN_TIME_MS);
  }

  Serial.println(
    "Press the button to start an inference capture."
  );

  showReadyScreen();
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

  // Ignore button bounce and electrical noise during the complete capture,
  // HTTP request, OLED update, and servo movement.
  detachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN)
  );

  runCapture();

  // Rearm only after the button is physically released and stable.
  while (digitalRead(BUTTON_PIN) == LOW)
  {
    delay(5);
  }

  delay(BUTTON_RELEASE_STABLE_MS);

  noInterrupts();
  captureRequested = false;
  interrupts();

  lastCaptureMs = millis();

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );
}
