#include <Servo.h>
#include <math.h>

#define PIN_SERVO 10
#define PIN_TRIG  12
#define PIN_ECHO  13

Servo myServo;

unsigned long MOVING_TIME = 1500;
unsigned long moveStartTime = 0;
unsigned long lastCheckTime = 0;

int startAngle = 10;   // 현재 움직임의 시작 각도
int stopAngle  = 170;  // 현재 움직임의 목표 각도
int servoAngle = 10;

const int CLOSE_ANGLE = 10;
const int OPEN_ANGLE  = 170;

const float CAR_IN_DISTANCE  = 200.0; // 200mm 이내이면 차량 접근
const float CAR_OUT_DISTANCE = 260.0; // 260mm 이상이면 차량 통과

bool isMoving = false;
bool isOpened = false;
bool carDetected = false;

// 1: sigmoid 함수
// 2: cos 함수
#define MOVE_FUNCTION 1


// 초음파 센서로 거리 측정, 단위는 mm
float getDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long echoTime = pulseIn(PIN_ECHO, HIGH, 30000);

  if (echoTime == 0) {
    return 1000.0;
  }

  return echoTime * 0.173;
}


// sigmoid 함수를 0~1 범위로 보정
float sigmoid(float progress) {
  float strength = 10.0;

  float result =
    1.0 / (1.0 + exp(-strength * (progress - 0.5)));

  float first =
    1.0 / (1.0 + exp(strength * 0.5));

  float last =
    1.0 / (1.0 + exp(-strength * 0.5));

  return (result - first) / (last - first);
}


// 두 번째 부드러운 제어 함수
float cosineMoving(float progress) {
  return (1.0 - cos(PI * progress)) / 2.0;
}


// 새로운 각도로 이동 시작
void startMoving(int destination) {
  startAngle = servoAngle;
  stopAngle = destination;

  moveStartTime = millis();
  isMoving = true;
}


// 시간에 따라 서보 위치 변경
void moveServo() {
  if (!isMoving) {
    return;
  }

  unsigned long progressTime = millis() - moveStartTime;

  if (progressTime >= MOVING_TIME) {
    servoAngle = stopAngle;
    myServo.write(servoAngle);
    isMoving = false;

    isOpened = (stopAngle == OPEN_ANGLE);
    return;
  }

  float progress =
    (float)progressTime / (float)MOVING_TIME;

  float smoothProgress;

#if MOVE_FUNCTION == 1
  smoothProgress = sigmoid(progress);
#else
  smoothProgress = cosineMoving(progress);
#endif

  servoAngle =
    startAngle +
    (stopAngle - startAngle) * smoothProgress;

  myServo.write(servoAngle);
}


void setup() {
  Serial.begin(9600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_TRIG, LOW);

  myServo.attach(PIN_SERVO);
  myServo.write(CLOSE_ANGLE);

  servoAngle = CLOSE_ANGLE;
  startAngle = CLOSE_ANGLE;
  stopAngle = CLOSE_ANGLE;
}


void loop() {
  // 서보 이동은 loop가 반복될 때마다 갱신
  moveServo();

  // 초음파 센서는 100ms마다 측정
  if (millis() - lastCheckTime < 100) {
    return;
  }

  lastCheckTime = millis();

  float distance = getDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" mm");

  // 가까운 곳에서 차량이 감지됨
  if (!carDetected && distance <= CAR_IN_DISTANCE) {
    carDetected = true;

    Serial.println("Car detected - barrier opens");
    startMoving(OPEN_ANGLE);
  }

  // 차량이 센서에서 멀어지면 통과한 것으로 판단
  else if (carDetected &&
           distance >= CAR_OUT_DISTANCE &&
           isOpened &&
           !isMoving) {
    carDetected = false;

    Serial.println("Car passed - barrier closes");
    startMoving(CLOSE_ANGLE);
  }
}
