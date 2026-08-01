constexpr uint8_t BUZZER_CONTROL_PIN = 25;

void setup() {
  pinMode(BUZZER_CONTROL_PIN, OUTPUT);

  // The transistor is disabled with a LOW output.
  digitalWrite(BUZZER_CONTROL_PIN, LOW);

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("Transistor buzzer test started.");
}

void loop() {
  digitalWrite(BUZZER_CONTROL_PIN, HIGH);
  Serial.println("Buzzer ON");
  delay(500);

  digitalWrite(BUZZER_CONTROL_PIN, LOW);
  Serial.println("Buzzer OFF");
  delay(1500);
}