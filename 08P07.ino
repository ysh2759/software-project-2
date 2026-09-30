// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec) - 25ms 적용[cite: 1, 2]
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficient to convert duration to distance

unsigned long last_sampling_time;   // unit: msec

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);   // sonar ECHO
  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar 
  
  // initialize serial port
  Serial.begin(57600);
}

void loop() { 
  float distance;

  // wait until next sampling time (polling)
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  distance = USS_measure(PIN_TRIG, PIN_ECHO); // read distance

  int brightness = 255; // 기본값: 꺼짐 (Active Low)[cite: 1]

  // 장애물 거리에 비례하여 LED 밝기 제어
  if (distance < _DIST_MIN || distance > _DIST_MAX) {
      brightness = 255; // 범위 밖: 꺼짐[cite: 1]
  } else if (distance <= 200.0) {
      // 100mm(꺼짐: 255) ~ 200mm(최대 밝기: 0)[cite: 1]
      brightness = (int)(255.0 - (distance - 100.0) * (255.0 / 100.0));
  } else {
      // 200mm(최대 밝기: 0) ~ 300mm(꺼짐: 255)[cite: 1]
      brightness = (int)((distance - 200.0) * (255.0 / 100.0));
  }

  // Active Low PWM 출력 적용 (0: 가장 밝음, 255: 꺼짐)[cite: 1]
  analogWrite(PIN_LED, brightness);

  // output the distance to the serial port
  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(",distance:");  Serial.print(distance);
  Serial.print(",brightness:"); Serial.print(brightness);
  Serial.print(",Max:");       Serial.print(_DIST_MAX);
  Serial.println("");
  
  // delay(50); 삭제 완료[cite: 1, 2]
  
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
