int led = 7;

void setup() {
  pinMode(led, OUTPUT);
}

void loop() {
  // 1초 동안 LED 키기
  digitalWrite(led, LOW);
  delay(1000);

  // 1초 동안 LED 5회 깜빡이기
  for (int i = 0; i < 5; i++) {
    digitalWrite(led, HIGH);
    delay(100);
    
    digitalWrite(led, LOW);
    delay(100);
  }

  // LED 끄기
  digitalWrite(led, HIGH);

  while (1) {
  }
}
