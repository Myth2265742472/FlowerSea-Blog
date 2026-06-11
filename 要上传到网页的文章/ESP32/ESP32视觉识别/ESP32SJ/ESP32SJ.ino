// 定义电机控制引脚
const int IN1 = 18; // 电机1控制
const int IN2 = 19;
const int EN1 = 5;
const int IN3 = 21; // 电机2控制
const int IN4 = 22;
const int EN2 = 23;

// 设置默认速度
int speed1 = 200; // 0-255
int speed2 = 200; // 0-255

void setup() {
  // 初始化串口
  Serial.begin(115200);
  
  // 初始化电机控制引脚
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(EN1, OUTPUT);
  pinMode(EN2, OUTPUT);
  
  // 停止电机
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(EN1, 0);
  analogWrite(EN2, 0);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'F') {
      // 前进
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(EN1, speed1);
      analogWrite(EN2, speed2);
      Serial.println("Forward");
    } else if (command == 'S') {
      // 停止
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      analogWrite(EN1, 0);
      analogWrite(EN2, 0);
      Serial.println("Stop");
    }
  }
  delay(10); // 控制循环频率
}