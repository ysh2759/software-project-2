#include <Servo.h>

// Arduino pin assignment
#define PIN_LED   9   // LED active-low
#define PIN_TRIG  12  // sonar sensor TRIGGER
#define PIN_ECHO  13  // sonar sensor ECHO
#define PIN_SERVO 10  // servo motor

// configurable parameters for sonar
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25      // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 180.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 360.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL) // coefficent to convert duration to distance

#define _EMA_ALPHA 0.1    // EMA weight of new sample (range: 0 to 1)

// duty duration for myservo.writeMicroseconds()
// 서보 보정값을 반영한 펄스 폭 설정
#define _DUTY_MIN 500  // servo full clockwise position (0 degree)
#define _DUTY_MAX 2400 // servo full counterclockwise position (180 degree)
#define _DUTY_NEU ((_DUTY_MIN + _DUTY_MAX) / 2) // servo neutral position (90 degree)

// global variables
float  dist_ema, dist_prev = _DIST_MAX; // unit: mm
unsigned long last_sampling_time;       // unit: ms

Servo myservo;

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);    // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);     // sonar ECHO
  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar 

  // Active-Low LED 초기화 (꺼짐)
  digitalWrite(PIN_LED, HIGH);

  myservo.attach(PIN_SERVO); 
  myservo.writeMicroseconds(_DUTY_NEU);

  // initialize USS related variables
  dist_prev = _DIST_MIN; // raw distance output from USS (unit: mm)
  dist_ema = _DIST_MIN;  // EMA 초기화

  // initialize serial port
  Serial.begin(57600);
}

void loop() {
  float  dist_raw, dist_filtered;
  bool   in_range = true;
  
  // wait until next sampling time.
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // the range filter (180~360mm 제한)
  if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX)) {
      dist_filtered = dist_prev;
      in_range = false;
  } else if (dist_raw < _DIST_MIN) {
      dist_filtered = dist_prev;
      in_range = false;
  } else {    // In desired Range
      dist_filtered = dist_raw;
      dist_prev = dist_raw;
      in_range = true;
  }

  // EMA 수식 구현
  dist_ema = _EMA_ALPHA * dist_filtered + (1.0 - _EMA_ALPHA) * dist_ema;

  // 측정 범위 안으로 들어올 경우 PIN_LED 점등 (Active-Low)
  if (in_range) {
      digitalWrite(PIN_LED, LOW);  // LED ON
  } else {
      digitalWrite(PIN_LED, HIGH); // LED OFF
  }

  // 거리(18cm ~ 36cm)에 비례하여 서보 각도 연속 제어
  float duty_target;

  if (dist_ema <= _DIST_MIN) {
      duty_target = _DUTY_MIN;
  } else if (dist_ema >= _DIST_MAX) {
      duty_target = _DUTY_MAX;
  } else {
      duty_target = _DUTY_MIN + (dist_ema - _DIST_MIN) * (_DUTY_MAX - _DUTY_MIN) / (_DIST_MAX - _DIST_MIN);
  }

  // 서보 모터에 펄스 폭 전달
  myservo.writeMicroseconds((int)duty_target);

  // 과제 슬라이드 지정 시리얼 플롯 형식 출력
  Serial.print("Min:");      Serial.print(_DIST_MIN);
  Serial.print(",dist:");    Serial.print(dist_raw);
  Serial.print(",ema:");     Serial.print(dist_ema);
  Serial.print(",Servo:");   Serial.print(myservo.read());
  Serial.print(",Max:");     Serial.print(_DIST_MAX);
  Serial.println("");
 
  // update last sampling time
  last_sampling_time += INTERVAL;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}
