/*
 * ESP32-S3 舵机ESP-NOW接收端
 * 功能: 接收MPU6050数据并控制舵机
 *
 * 硬件连接:
 * - ESP32-S3-N16R8
 * - SG90 舵机:
 *   红线(VCC) -> 5V
 *   棕线(GND) -> GND
 *   橙线(信号) -> GPIO7
 */

#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

// 舵机引脚
#define SERVO_PIN 5

// 舵机对象
Servo myservo;

// 数据结构
typedef struct struct_message {
  float angleX;
  float angleY;
  float angleZ;
} struct_message;

struct_message myData;

// 平滑滤波参数
#define FILTER_SAMPLES 15
float servoAngle_history[FILTER_SAMPLES];
int filter_index = 0;

// 死区控制
#define DEAD_ZONE 2.0
int lastServoAngle = 90;

// 舵机控制间隔
unsigned long lastServoUpdate = 0;
const int servoUpdateInterval = 20; // 50Hz控制间隔

// 数据接收时间
unsigned long lastReceiveTime = 0;
const int timeoutMs = 2000; // 超时时间

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n=== 舵机ESP-NOW接收端 ===");

  // 初始化WiFi
  WiFi.mode(WIFI_STA);

  // 初始化ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW初始化失败");
    return;
  }
  Serial.println("ESP-NOW初始化成功");

  // 注册接收回调
  esp_now_register_recv_cb(OnDataRecv);

  // 打印本设备MAC地址（用于发送端配置）
  Serial.print("本设备MAC地址: ");
  Serial.println(WiFi.macAddress());

  // 初始化舵机
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);
  Serial.println("舵机初始化完成");

  // 初始化历史数据
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    servoAngle_history[i] = 90;
  }

  // 舵机回到初始位置
  myservo.write(90);
  delay(1000);

  Serial.println("系统初始化完成!");
  Serial.println("等待接收数据...");
}

void loop() {
  // 检查接收超时
  if (lastReceiveTime > 0 && millis() - lastReceiveTime > timeoutMs) {
    Serial.println("接收超时，等待数据...");
    lastReceiveTime = 0;
  }

  // 定时控制舵机（避免过于频繁）
  if (millis() - lastServoUpdate >= servoUpdateInterval && lastReceiveTime > 0) {
    lastServoUpdate = millis();

    // 映射角度到舵机
    int servoAngle = mapAngleToServo(myData.angleX);

    // 平滑滤波
    servoAngle = smoothServoAngle(servoAngle);

    // 死区控制
    int angleDiff = abs(servoAngle - lastServoAngle);
    if (angleDiff >= DEAD_ZONE) {
      myservo.write(servoAngle);
      lastServoAngle = servoAngle;
    }
  }
}

// 接收回调函数
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(myData)) {
    memcpy(&myData, incomingData, sizeof(myData));
    lastReceiveTime = millis();

    // 映射角度到舵机
    int servoAngle = mapAngleToServo(myData.angleX);

    // 平滑滤波
    servoAngle = smoothServoAngle(servoAngle);

    // 死区控制
    int angleDiff = abs(servoAngle - lastServoAngle);
    if (angleDiff >= DEAD_ZONE) {
      myservo.write(servoAngle);
      lastServoAngle = servoAngle;

      // 打印调试信息
      Serial.printf("接收: X=%.1f°, Y=%.1f°, Z=%.1f° | 舵机: %d°\n",
                    myData.angleX, myData.angleY, myData.angleZ, servoAngle);
    }
  } else {
    Serial.println("数据长度错误");
  }
}

// 映射角度到舵机角度 (0-180度)
int mapAngleToServo(float angle) {
  int servoAngle = (int)((angle + 90) * 180.0 / 180.0);

  if (servoAngle < 0) servoAngle = 0;
  if (servoAngle > 180) servoAngle = 180;

  return servoAngle;
}

// 平滑滤波
int smoothServoAngle(int servoAngle) {
  servoAngle_history[filter_index] = servoAngle;
  filter_index = (filter_index + 1) % FILTER_SAMPLES;

  float sum = 0;
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    sum += servoAngle_history[i];
  }

  return (int)(sum / FILTER_SAMPLES);
}
