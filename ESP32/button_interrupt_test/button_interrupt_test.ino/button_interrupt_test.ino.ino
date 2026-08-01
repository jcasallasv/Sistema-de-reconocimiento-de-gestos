constexpr uint8_t BUTTON_PIN = 27;

volatile bool buttonInterruptDetected = false;
volatile unsigned long lastInterruptTime = 0;

void IRAM_ATTR handleButtonInterrupt() {
  const unsigned long currentTime = millis();

  if (currentTime - lastInterruptTime > 200) {
    buttonInterruptDetected = true;
    lastInterruptTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );

  Serial.println();
  Serial.println("Button interrupt test ready");
  Serial.println("Press the button.");
}

void loop() {
  if (buttonInterruptDetected) {
    buttonInterruptDetected = false;

    Serial.println("Button interrupt detected.");
  }
}