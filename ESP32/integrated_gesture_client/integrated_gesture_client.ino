#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>

#include "secrets.h"

// Optional hardware features.
// Change to 1 only after the corresponding hardware and libraries are ready.
#define ENABLE_OLED 0
#define ENABLE_SERVO 0

#if ENABLE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

#if ENABLE_SERVO
#include <ESP32Servo.h>
#endif

// Hardware pins
constexpr uint8_t MPU_ADDRESS = 0x68;
constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;
constexpr uint8_t BUTTON_PIN = 27;
constexpr uint8_t LED_PIN = 26;
constexpr uint8_t BUZZER_CONTROL_PIN = 25;
constexpr uint8_t SERVO_SIGNAL_PIN = 33;

// MPU6050-compatible register addresses
constexpr uint8_t SIGNAL_PATH_RESET = 0x68;
constexpr uint8_t POWER_MANAGEMENT_1 = 0x6B;
constexpr uint8_t POWER_MANAGEMENT_2 = 0x6C;
constexpr uint8_t WHO_AM_I = 0x75;
constexpr uint8_t ACCEL_DATA_START = 0x3B;

// Gesture capture configuration
constexpr size_t SAMPLE_COUNT = 100;
constexpr uint32_t SAMPLE_INTERVAL_US = 20000;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 250;

// Communication configuration
constexpr uint32_t WIFI_CONNECTION_TIMEOUT_MS = 20000;
constexpr uint32_t HTTP_REQUEST_TIMEOUT_MS = 15000;

// OLED configuration
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr int OLED_WIDTH = 128;
constexpr int OLED_HEIGHT = 64;

// Servo positions
constexpr int SERVO_LEFT_ANGLE = 30;
constexpr int SERVO_UP_ANGLE = 90;
constexpr int SERVO_RIGHT_ANGLE = 150;

struct MotionSample {
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;
};

MotionSample samples[SAMPLE_COUNT];

volatile bool captureRequested = false;
volatile bool captureActive = false;
volatile uint32_t lastInterruptTime = 0;

bool mpuReady = false;
bool displayReady = false;
bool servoReady = false;

#if ENABLE_OLED
Adafruit_SSD1306 display(
  OLED_WIDTH,
  OLED_HEIGHT,
  &Wire,
  -1
);
#endif

#if ENABLE_SERVO
Servo gestureServo;
#endif


void IRAM_ATTR handleButtonInterrupt() {
  if (captureActive) {
    return;
  }

  const uint32_t currentTime = millis();

  if (currentTime - lastInterruptTime >= BUTTON_DEBOUNCE_MS) {
    captureRequested = true;
    lastInterruptTime = currentTime;
  }
}


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

  if (
    Wire.requestFrom(
      MPU_ADDRESS,
      static_cast<uint8_t>(1),
      true
    ) != 1
  ) {
    return 0xFF;
  }

  return Wire.read();
}


bool readMotionData(MotionSample &sample) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(ACCEL_DATA_START);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (
    Wire.requestFrom(
      MPU_ADDRESS,
      static_cast<uint8_t>(14),
      true
    ) != 14
  ) {
    return false;
  }

  sample.ax = (Wire.read() << 8) | Wire.read();
  sample.ay = (Wire.read() << 8) | Wire.read();
  sample.az = (Wire.read() << 8) | Wire.read();

  // Skip the two temperature bytes.
  Wire.read();
  Wire.read();

  sample.gx = (Wire.read() << 8) | Wire.read();
  sample.gy = (Wire.read() << 8) | Wire.read();
  sample.gz = (Wire.read() << 8) | Wire.read();

  return true;
}


bool initializeMpu() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  Serial.println("Initializing MPU6050-compatible sensor...");

  const uint8_t whoAmIValue = readRegister(WHO_AM_I);

  Serial.print("WHO_AM_I: 0x");

  if (whoAmIValue < 0x10) {
    Serial.print("0");
  }

  Serial.println(whoAmIValue, HEX);

  if (!writeRegister(POWER_MANAGEMENT_1, 0x80)) {
    Serial.println("MPU reset command failed.");
    return false;
  }

  delay(150);

  if (!writeRegister(SIGNAL_PATH_RESET, 0x07)) {
    Serial.println("MPU signal path reset failed.");
    return false;
  }

  delay(150);

  if (!writeRegister(POWER_MANAGEMENT_1, 0x01)) {
    Serial.println("MPU clock configuration failed.");
    return false;
  }

  if (!writeRegister(POWER_MANAGEMENT_2, 0x00)) {
    Serial.println("MPU power configuration failed.");
    return false;
  }

  delay(150);

  Serial.println("MPU initialization completed.");

  return true;
}


bool initializeDisplay() {
#if ENABLE_OLED
  Serial.println("Initializing OLED display...");

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDRESS
    )
  ) {
    Serial.println("OLED initialization failed.");
    return false;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("GESTURE SYSTEM");
  display.println();
  display.println("INITIALIZING...");
  display.display();

  Serial.println("OLED initialization completed.");

  return true;
#else
  Serial.println("OLED support reserved but disabled.");
  return false;
#endif
}


bool initializeServo() {
#if ENABLE_SERVO
  Serial.println("Initializing servo...");

  gestureServo.setPeriodHertz(50);

  const int attachedChannel = gestureServo.attach(
    SERVO_SIGNAL_PIN,
    500,
    2400
  );

  if (attachedChannel < 0) {
    Serial.println("Servo attachment failed.");
    return false;
  }

  gestureServo.write(SERVO_UP_ANGLE);
  delay(500);

  Serial.println("Servo initialization completed.");

  return true;
#else
  Serial.println("Servo support reserved but disabled.");
  return false;
#endif
}


void showSystemStatus(
  const char *line1,
  const char *line2 = nullptr,
  const char *line3 = nullptr
) {
#if ENABLE_OLED
  if (!displayReady) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.println(line1);

  if (line2 != nullptr) {
    display.println();
    display.println(line2);
  }

  if (line3 != nullptr) {
    display.println(line3);
  }

  display.display();
#endif
}


void showInferenceResult(
  const String &gesture,
  float confidence
) {
#if ENABLE_OLED
  if (!displayReady) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.setTextSize(1);
  display.println("GESTURE:");

  display.setTextSize(2);
  display.println(gesture);

  display.setTextSize(1);
  display.println();

  display.print("CONFIDENCE: ");
  display.print(confidence * 100.0f, 1);
  display.println("%");

  display.display();
#endif
}


bool connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  showSystemStatus(
    "GESTURE SYSTEM",
    "CONNECTING WIFI"
  );

  Serial.print("Connecting to WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t connectionStartTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (
      millis() - connectionStartTime >=
      WIFI_CONNECTION_TIMEOUT_MS
    ) {
      Serial.println();
      Serial.println("WiFi connection timeout.");

      showSystemStatus(
        "ERROR",
        "WIFI TIMEOUT"
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

  return true;
}


bool captureGestureSamples() {
  showSystemStatus(
    "CAPTURING...",
    "PERFORM GESTURE"
  );

  Serial.println("Capture started.");
  Serial.println("Perform the gesture now.");

  uint32_t nextSampleTime = micros();

  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    while (
      static_cast<int32_t>(
        micros() - nextSampleTime
      ) < 0
    ) {
      delay(1);
    }

    if (!readMotionData(samples[index])) {
      Serial.print("Sensor read failed at sample ");
      Serial.println(index + 1);

      showSystemStatus(
        "ERROR",
        "SENSOR READ"
      );

      return false;
    }

    nextSampleTime += SAMPLE_INTERVAL_US;
  }

  Serial.print("Capture completed. Samples: ");
  Serial.println(SAMPLE_COUNT);

  return true;
}


bool buildGesturePayload(String &jsonBody) {
  if (!jsonBody.reserve(12000)) {
    Serial.println("JSON memory reservation failed.");
    return false;
  }

  jsonBody = "{\"samples\":[";

  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    if (index > 0) {
      jsonBody += ",";
    }

    jsonBody += "{\"ax\":";
    jsonBody += samples[index].ax;

    jsonBody += ",\"ay\":";
    jsonBody += samples[index].ay;

    jsonBody += ",\"az\":";
    jsonBody += samples[index].az;

    jsonBody += ",\"gx\":";
    jsonBody += samples[index].gx;

    jsonBody += ",\"gy\":";
    jsonBody += samples[index].gy;

    jsonBody += ",\"gz\":";
    jsonBody += samples[index].gz;

    jsonBody += "}";
  }

  jsonBody += "]}";

  return true;
}


bool extractJsonBoolean(
  const String &json,
  const char *key,
  bool &value
) {
  String token = "\"";
  token += key;
  token += "\":";

  int position = json.indexOf(token);

  if (position < 0) {
    return false;
  }

  position += token.length();

  while (
    position < static_cast<int>(json.length()) &&
    json[position] == ' '
  ) {
    position++;
  }

  if (json.substring(position, position + 4) == "true") {
    value = true;
    return true;
  }

  if (json.substring(position, position + 5) == "false") {
    value = false;
    return true;
  }

  return false;
}


bool extractJsonString(
  const String &json,
  const char *key,
  String &value
) {
  String token = "\"";
  token += key;
  token += "\":";

  int position = json.indexOf(token);

  if (position < 0) {
    return false;
  }

  position += token.length();

  while (
    position < static_cast<int>(json.length()) &&
    json[position] == ' '
  ) {
    position++;
  }

  if (
    position >= static_cast<int>(json.length()) ||
    json[position] != '"'
  ) {
    return false;
  }

  const int valueStart = position + 1;
  const int valueEnd = json.indexOf('"', valueStart);

  if (valueEnd < 0) {
    return false;
  }

  value = json.substring(valueStart, valueEnd);

  return true;
}


bool extractJsonNumber(
  const String &json,
  const char *key,
  float &value
) {
  String token = "\"";
  token += key;
  token += "\":";

  int position = json.indexOf(token);

  if (position < 0) {
    return false;
  }

  position += token.length();

  while (
    position < static_cast<int>(json.length()) &&
    json[position] == ' '
  ) {
    position++;
  }

  const int valueStart = position;

  while (position < static_cast<int>(json.length())) {
    const char character = json[position];

    const bool isNumberCharacter =
      (character >= '0' && character <= '9') ||
      character == '-' ||
      character == '+' ||
      character == '.' ||
      character == 'e' ||
      character == 'E';

    if (!isNumberCharacter) {
      break;
    }

    position++;
  }

  if (position == valueStart) {
    return false;
  }

  value = json.substring(
    valueStart,
    position
  ).toFloat();

  return true;
}


void turnOutputsOff() {
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_CONTROL_PIN, LOW);
}


void moveServoForGesture(const String &gesture) {
#if ENABLE_SERVO
  if (!servoReady) {
    return;
  }

  if (gesture == "left") {
    gestureServo.write(SERVO_LEFT_ANGLE);
    return;
  }

  if (gesture == "up") {
    gestureServo.write(SERVO_UP_ANGLE);
    return;
  }

  if (gesture == "right") {
    gestureServo.write(SERVO_RIGHT_ANGLE);
  }
#endif
}


void applyGestureFeedback(const String &gesture) {
  turnOutputsOff();

  moveServoForGesture(gesture);

  if (gesture == "left") {
    Serial.println("Feedback: LED and servo left.");

    digitalWrite(LED_PIN, HIGH);
    delay(800);
    digitalWrite(LED_PIN, LOW);

    return;
  }

  if (gesture == "right") {
    Serial.println("Feedback: buzzer and servo right.");

    digitalWrite(BUZZER_CONTROL_PIN, HIGH);
    delay(400);
    digitalWrite(BUZZER_CONTROL_PIN, LOW);

    return;
  }

  if (gesture == "up") {
    Serial.println("Feedback: LED, buzzer and servo center.");

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_CONTROL_PIN, HIGH);

    delay(500);

    turnOutputsOff();

    return;
  }

  Serial.println(
    "No feedback assigned to the received gesture."
  );
}


bool processServerResponse(const String &response) {
  bool success = false;

  String gesture;

  float confidence = 0.0f;
  float leftProbability = 0.0f;
  float rightProbability = 0.0f;
  float upProbability = 0.0f;

  if (
    !extractJsonBoolean(
      response,
      "success",
      success
    )
  ) {
    Serial.println(
      "Response does not contain a valid success field."
    );

    return false;
  }

  if (!success) {
    Serial.println(
      "Server reported an unsuccessful inference."
    );

    Serial.println(response);

    return false;
  }

  if (
    !extractJsonString(
      response,
      "gesture",
      gesture
    )
  ) {
    Serial.println(
      "Response does not contain a valid gesture."
    );

    return false;
  }

  if (
    !extractJsonNumber(
      response,
      "confidence",
      confidence
    )
  ) {
    Serial.println(
      "Response does not contain valid confidence."
    );

    return false;
  }

  extractJsonNumber(
    response,
    "left",
    leftProbability
  );

  extractJsonNumber(
    response,
    "right",
    rightProbability
  );

  extractJsonNumber(
    response,
    "up",
    upProbability
  );

  Serial.println();
  Serial.println("Inference result");

  Serial.print("Gesture: ");
  Serial.println(gesture);

  Serial.print("Confidence: ");
  Serial.println(confidence, 4);

  Serial.print("Left probability: ");
  Serial.println(leftProbability, 4);

  Serial.print("Right probability: ");
  Serial.println(rightProbability, 4);

  Serial.print("Up probability: ");
  Serial.println(upProbability, 4);

  showInferenceResult(
    gesture,
    confidence
  );

  applyGestureFeedback(gesture);

  return true;
}


bool sendGestureSamples() {
  if (!connectToWiFi()) {
    return false;
  }

  String jsonBody;

  if (!buildGesturePayload(jsonBody)) {
    return false;
  }

  Serial.print("JSON payload size: ");
  Serial.print(jsonBody.length());
  Serial.println(" bytes");

  Serial.println(
    "Sending gesture samples to Raspberry Pi..."
  );

  showSystemStatus(
    "SENDING...",
    "WAITING FOR AI"
  );

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, API_URL)) {
    Serial.println(
      "HTTP client initialization failed."
    );

    showSystemStatus(
      "ERROR",
      "HTTP CLIENT"
    );

    return false;
  }

  http.setTimeout(HTTP_REQUEST_TIMEOUT_MS);
  http.addHeader(
    "Content-Type",
    "application/json"
  );

  const int httpResponseCode = http.POST(jsonBody);

  Serial.print("HTTP response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode <= 0) {
    Serial.print("HTTP request failed: ");
    Serial.println(
      http.errorToString(httpResponseCode)
    );

    http.end();

    showSystemStatus(
      "ERROR",
      "HTTP REQUEST"
    );

    return false;
  }

  const String response = http.getString();

  http.end();

  if (
    httpResponseCode < 200 ||
    httpResponseCode >= 300
  ) {
    Serial.println(
      "Server returned an HTTP error."
    );

    Serial.println(response);

    showSystemStatus(
      "ERROR",
      "SERVER RESPONSE"
    );

    return false;
  }

  Serial.println("Server response received.");

  return processServerResponse(response);
}


void runGestureCycle() {
  if (!mpuReady) {
    Serial.println(
      "Capture cancelled because the MPU is not ready."
    );

    showSystemStatus(
      "ERROR",
      "MPU NOT READY"
    );

    return;
  }

  if (!captureGestureSamples()) {
    Serial.println("Gesture capture failed.");
    return;
  }

  if (!sendGestureSamples()) {
    Serial.println(
      "Gesture transmission or inference failed."
    );

    return;
  }

  Serial.println(
    "Gesture cycle completed successfully."
  );
}


void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_CONTROL_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  turnOutputsOff();

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Integrated Gesture Client");
  Serial.println("-------------------------");

  mpuReady = initializeMpu();
  displayReady = initializeDisplay();
  servoReady = initializeServo();

  if (!mpuReady) {
    Serial.println("MPU initialization failed.");
  }

  connectToWiFi();

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );

  showSystemStatus(
    "SYSTEM READY",
    "PRESS BUTTON"
  );

  Serial.println();
  Serial.println("System ready.");
  Serial.println(
    "Press the button to capture a gesture."
  );
}


void loop() {
  if (!captureRequested) {
    delay(10);
    return;
  }

  noInterrupts();

  captureRequested = false;
  captureActive = true;

  interrupts();

  Serial.println();
  Serial.println("Button interrupt detected.");

  runGestureCycle();

  captureActive = false;

  Serial.println();
  Serial.println("System ready.");
  Serial.println(
    "Press the button to capture another gesture."
  );
}
