// 08_example_2.ino
// 장애물 거리에 따른 LED 밝기 제어

// Arduino pin assignment
#define PIN_LED   9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0       // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25         // sampling interval (unit: msec)
#define PULSE_DURATION 10   // ultra-sound Pulse Duration (unit: usec)

#define _DIST_MIN  100.0    // minimum distance (unit: mm)
#define _DIST_PEAK 200.0    // maximum LED brightness distance (unit: mm)
#define _DIST_MAX  300.0    // maximum distance (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

unsigned long last_sampling_time;   // unit: msec

void setup()
{
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);   // sonar ECHO

  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar

  // Active Low LED:
  // analogWrite(PIN_LED, 255) = LED OFF
  analogWrite(PIN_LED, 255);

  // initialize serial port
  Serial.begin(57600);
}

void loop()
{
  float distance;
  int led_pwm;

  // wait until next sampling time
  // sampling interval: 25 msec
  if (millis() - last_sampling_time < INTERVAL)
    return;

  // read distance
  distance = USS_measure(PIN_TRIG, PIN_ECHO);

  if ((distance == 0.0) || (distance > _DIST_MAX))
  {
    // 측정 실패 또는 300mm보다 먼 경우
    distance = _DIST_MAX + 10.0;  // 시리얼 플로터에는 310mm 출력
    led_pwm = 255;                // LED OFF
  }
  else if (distance < _DIST_MIN)
  {
    // 100mm보다 가까운 경우
    distance = _DIST_MIN - 10.0;  // 시리얼 플로터에는 90mm 출력
    led_pwm = 255;                // LED OFF
  }
  else if (distance <= _DIST_PEAK)
  {
    // 100~200mm 구간
    // 100mm: PWM 255, LED OFF
    // 150mm: PWM 약 128, Duty 50%
    // 200mm: PWM 0, 최대 밝기

    float ratio =
      (distance - _DIST_MIN) /
      (_DIST_PEAK - _DIST_MIN);

    led_pwm = (int)(255.0 * (1.0 - ratio) + 0.5);
  }
  else
  {
    // 200~300mm 구간
    // 200mm: PWM 0, 최대 밝기
    // 250mm: PWM 약 128, Duty 50%
    // 300mm: PWM 255, LED OFF

    float ratio =
      (distance - _DIST_PEAK) /
      (_DIST_MAX - _DIST_PEAK);

    led_pwm = (int)(255.0 * ratio + 0.5);
  }

  // Active Low LED brightness control
  analogWrite(PIN_LED, led_pwm);
  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",distance:");
  Serial.print(distance);

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.println("");

  // update last sampling time
  last_sampling_time += INTERVAL;
}

// get a distance reading from USS
// return value is in millimeter
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
