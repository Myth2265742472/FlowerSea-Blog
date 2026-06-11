#include <WiFi.h>

// 引脚定义
const int potPin = 32;      // 电位器连接到GPIO32
const int servoPin = 2;     // 舵机信号线连接到GPIO2

// PWM设置
const int PWMChannel = 0;
const int PWMFrequency = 50;  // 舵机常用的50Hz
const int PWMResolution = 16; // 16位分辨率 (0-65535)

// 舵机角度映射
const int SERVO_MIN_PULSE = 500;   // 最小角度对应脉冲宽度(微秒)
const int SERVO_MAX_PULSE = 2500;  // 最大角度对应脉冲宽度(微秒)

void setup() {
  Serial.begin(115200);
  
  // 配置PWM通道
  ledcSetup(PWMChannel, PWMFrequency, PWMResolution);
  
  // 将PWM信号附加到GPIO引脚
  ledcAttachPin(servoPin, PWMChannel);
  
  Serial.println("ESP32电位器控制舵机系统启动");
}

void loop() {
  // 读取电位器值 (0-4095)
  int potValue = analogRead(potPin);
  
  // 转换为角度 (0-180度)
  int angle = map(potValue, 0, 4095, 0, 180);
  
  // 计算PWM值 (对应脉冲宽度)
  int pwmValue = map(angle, 0, 180, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  pwmValue = map(pwmValue, 0, 20000, 0, 65535); // 转换到PWM范围
  
  // 输出PWM信号控制舵机
  ledcWrite(PWMChannel, pwmValue);
  
  // 串口输出调试信息
  Serial.print("电位器值: ");
  Serial.print(potValue);
  Serial.print(" → 角度: ");
  Serial.print(angle);
  Serial.print("° → PWM: ");
  Serial.println(pwmValue);
  
  delay(100); // 稍微延迟避免过于频繁更新
}