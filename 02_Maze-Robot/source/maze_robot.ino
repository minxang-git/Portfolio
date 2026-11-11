 // 모터 방향 제어 핀
int motor1Pin1 = 22;
int motor1Pin2 = 23;
int motor2Pin1 = 24;
int motor2Pin2 = 25;

// 모터 속도 제어용 PWM 핀
int motor1Enable = 5;
int motor2Enable = 6;

// 초음파 센서 5개 핀 설정
const int trigFront = 30;
const int echoFront = 31;
const int trigLeft = 32;
const int echoLeft = 33;
const int trigRight = 34;
const int echoRight = 35;
const int trigFLeft = 36;
const int echoFLeft = 37;
const int trigFRight = 38;
const int echoFRight = 39;

// 자율주행 / RC 모드 전환 버튼
const int buttonPin = 2;
bool autoMode = true;
bool lastButtonState = HIGH;
bool currentButtonState;

// RC 수신기 입력 핀
const int throttlePin = 3;
const int steeringPin = 4;

// 현재 모터 속도 및 가감속 설정
int currentLeftSpeed = 0;
int currentRightSpeed = 0;
const int accelStep = 40;

// 핀 입출력 및 시리얼 통신 초기화
void setup() {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(motor1Enable, OUTPUT);
  pinMode(motor2Enable, OUTPUT);

  pinMode(trigFront, OUTPUT);
  pinMode(echoFront, INPUT);
  pinMode(trigLeft, OUTPUT);
  pinMode(echoLeft, INPUT);
  pinMode(trigRight, OUTPUT);
  pinMode(echoRight, INPUT);
  pinMode(trigFLeft, OUTPUT);
  pinMode(echoFLeft, INPUT);
  pinMode(trigFRight, OUTPUT);
  pinMode(echoFRight, INPUT);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(throttlePin, INPUT);
  pinMode(steeringPin, INPUT);

  Serial.begin(9600);
  Serial.println("시작 준비 완료!");
}

// 선택된 모드에 따라 주행 실행
void loop() {
  checkModeButton();

  if (autoMode) {
    runAutonomousMode();
  } else {
    runRCMode();
  }
}

// 버튼 입력으로 주행 모드 전환
void checkModeButton() {
  currentButtonState = digitalRead(buttonPin);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    autoMode = !autoMode;
    Serial.print("모드 전환됨: ");
    Serial.println(autoMode ? "자율 주행 모드" : "RC 수동 모드");
    delay(500);
  }

  lastButtonState = currentButtonState;
}

// 초음파 센서를 이용한 자율주행
void runAutonomousMode() {
  long front = measureDistance(trigFront, echoFront);
  long left = measureDistance(trigLeft, echoLeft);
  long right = measureDistance(trigRight, echoRight);
  long fLeft = measureDistance(trigFLeft, echoFLeft);
  long fRight = measureDistance(trigFRight, echoFRight);

  // 센서 거리값 출력
  Serial.print("Front: "); Serial.print(front);
  Serial.print(" | Left: "); Serial.print(left);
  Serial.print(" | Right: "); Serial.print(right);
  Serial.print(" | FLeft: "); Serial.print(fLeft);
  Serial.print(" | FRight: "); Serial.println(fRight);

  // 전방에 장애물이 없으면 전진
  if (front > 30) {
    moveForwardWithAlign();
  }
  // 전방 장애물 감지 시 회전 방향 결정
  else {
    if (left > 30 && right <= 30) {
      turnRight();
    } else if (right > 30 && left <= 30) {
      turnLeft();
    }
    // 양쪽 공간이 확보된 경우 대각선 센서 확인
    else if (left > 30 && right > 30) {
      if (fLeft < 30 && fRight < 30) {
        if (front > 30) {
          moveForwardWithAlign();
        } else {
          stopMotors();
        }
      } else if (fLeft < 30 && right > 30 && front > 30) {
        turnRight();
      } else if (fRight < 30 && left > 30 && front > 30) {
        turnLeft();
      } else {
        if (left > right) {
          turnLeft();
        } else {
          turnRight();
        }
      }
    }
    // 장애물을 벗어날 때까지 회전
    else if (front <= 30) {
      if (left > right) {
        while (measureDistance(trigFront, echoFront) <= 30) {
          turnRight();
          delay(100);
        }
      } else {
        while (measureDistance(trigFront, echoFront) <= 30) {
          turnLeft();
          delay(100);
        }
      }
      moveForwardWithAlign();
    }
    // 주행 불가능 시 정지
    else {
      stopMotors();
    }
  }
}

// RC 조종기 입력을 이용한 수동주행
void runRCMode() {
  static int lastThrottle = 1400;
  static int lastSteering = 1400;

  // RC 수신기 PWM 신호 읽기
  int throttleRaw = pulseIn(throttlePin, HIGH, 50000);
  int steeringRaw = pulseIn(steeringPin, HIGH, 50000);

  if (throttleRaw > 900 && throttleRaw < 1800) lastThrottle = throttleRaw;
  if (steeringRaw > 1000 && steeringRaw < 1800) lastSteering = steeringRaw;

  // 입력 신호를 모터 속도 범위로 변환
  int throttleVal = map(lastThrottle, 1000, 1800, -255, 255);
  int steeringVal = map(lastSteering, 1000, 1800, -255, 255);

  // 작은 입력값은 무시
  if (abs(throttleVal) < 30) throttleVal = 0;
  if (abs(steeringVal) < 30) steeringVal = 0;

  // 좌우 모터 목표 속도 계산
  int targetLeftSpeed = throttleVal + steeringVal;
  int targetRightSpeed = throttleVal - steeringVal;

  int minTurnSpeed = 60;

  // 회전 시 최소 속도 유지
  if (steeringVal < 0 && abs(targetRightSpeed) < minTurnSpeed) {
    targetRightSpeed = (targetRightSpeed > 0) ? minTurnSpeed : -minTurnSpeed;
  }
  if (steeringVal > 0 && abs(targetLeftSpeed) < minTurnSpeed) {
    targetLeftSpeed = (targetLeftSpeed > 0) ? minTurnSpeed : -minTurnSpeed;
  }

  // 급격한 속도 변화를 방지하는 가감속 처리
  currentLeftSpeed = gradualChange(currentLeftSpeed, targetLeftSpeed, accelStep);
  currentRightSpeed = gradualChange(currentRightSpeed, targetRightSpeed, accelStep);

  Serial.print("Throttle: "); Serial.print(throttleVal);
  Serial.print(" | Steering: "); Serial.print(steeringVal);
  Serial.print(" | LeftSpeed: "); Serial.print(currentLeftSpeed);
  Serial.print(" | RightSpeed: "); Serial.println(currentRightSpeed);

  setMotorSpeed(currentLeftSpeed, currentRightSpeed);
}

// 초음파 센서로 장애물까지의 거리 측정
long measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return 999;

  // 초음파 왕복 시간을 cm 단위로 변환
  return duration * 0.0344 / 2;
}

// 좌우 모터의 회전 방향 및 PWM 속도 제어
void setMotorSpeed(int left, int right) {
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);

  if (left == 0) {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
  } else if (left > 0) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
  } else {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    left = -left;
  }

  if (right == 0) {
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);
  } else if (right > 0) {
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
  } else {
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
    right = -right;
  }

  analogWrite(motor1Enable, left);
  analogWrite(motor2Enable, right);
}

// 제자리 좌회전
void turnLeft() {
  stopMotors();
  delay(500);
  setMotorSpeed(-160, 150);
  delay(500);
  stopMotors();
}

// 제자리 우회전
void turnRight() {
  stopMotors();
  delay(500);
  setMotorSpeed(160, -150);
  delay(500);
  stopMotors();
}

// 양쪽 모터 정지
void stopMotors() {
  analogWrite(motor1Enable, 0);
  analogWrite(motor2Enable, 0);
}

// 좌우 벽과의 거리 차이를 이용한 중앙 정렬
void alignToCenter() {
  long left = measureDistance(trigLeft, echoLeft);
  long right = measureDistance(trigRight, echoRight);
  long diff = left - right;

  if (abs(diff) > 5) {
    if (diff > 0) {
      setMotorSpeed(80, -80);
      delay(120);
    } else {
      setMotorSpeed(-80, 80);
      delay(120);
    }
    stopMotors();
  }
}

// 중앙 정렬 후 짧게 전진
void moveForwardWithAlign() {
  alignToCenter();
  setMotorSpeed(150, 140);
  delay(100);
  stopMotors();
}

// 현재 속도를 목표 속도까지 단계적으로 변경
int gradualChange(int current, int target, int step) {
  if (current < target) {
    current += step;
    if (current > target) current = target;
  } else if (current > target) {
    current -= step;
    if (current < target) current = target;
  }
  return current;
}

