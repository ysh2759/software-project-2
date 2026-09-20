const int ledPin = 7; // GPIO 7번 핀 지정

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // 1. 처음 1초 동안 LED 켜기 (ACTIVE LOW 방식이므로 '0' 출력)
  digitalWrite(ledPin, LOW);
  delay(1000);

  // 2. 다음 1초 동안 5회 깜빡이기
  // Off(1/HIGH) 0.1초 -> On(0/LOW) 0.1초 (총 0.2초 * 5회 = 1초)
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPin, HIGH); // 꺼짐 (OFF)
    delay(100);
    digitalWrite(ledPin, LOW);  // 켜짐 (ON)
    delay(100);
  }

  // 3. 2초 시점에 LED를 완전히 끄고(1/HIGH 출력) 종료
  digitalWrite(ledPin, HIGH);
  while (1) {
    // infinite loop
  }
}
