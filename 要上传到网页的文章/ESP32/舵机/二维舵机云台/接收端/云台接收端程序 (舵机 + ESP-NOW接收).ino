/*
 * 云台接收端程序 (舵机 + ESP-NOW接收)
 * 功能：接收ESP-NOW数据，控制舵机实现1:1姿态同步
 * 优化：无滤波、无死区、实时控制，实现精确的1:1同步
 */

#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

// ============= 舵机引脚 =============
#define SERVO_PITCH_PIN  12  // 俯仰角舵机引脚
#define SERVO_YAW_PIN    13  // 平面转角舵机引脚

// ============= 舵机角度配置 =============
#define PITCH_MIN_ANGLE   0   // 俯仰角最小角度
#define PITCH_MAX_ANGLE   180 // 俯仰角最大角度
#define PITCH_CENTER      90  // 俯仰角中心位置

#define YAW_MIN_ANGLE     0   // 平面转角最小角度
#define YAW_MAX_ANGLE     180 // 平面转角最大角度
#define YAW_CENTER        90  // 平面转角中心位置

// ============= 角度同步比例 =============
// 设置为 1.0 表示 1:1 同步（发送端转1度，舵机也转1度）
const float PITCH_SCALE = 1.0;  // 俯仰角比例（建议范围：0.1 ~ 2.0）
const float YAW_SCALE = 1.0;    // 平面转角比例（建议范围：0.1 ~ 2.0）

// ============= 舵机控制间隔 =============
// 直接控制，无延迟，实现1:1同步
unsigned long lastServoUpdate = 0;

// ============= 数据结构 =============
typedef struct struct_message {
  float angleX;  // Roll
  float angleY;  // Pitch
  float angleZ;  // Yaw
} struct_message;

struct_message myData;

// ============= 数据接收时间 =============
unsigned long lastReceiveTime = 0;
const int timeoutMs = 2000; // 超时时间

// ============= 舵机对象 =============
Servo servoPitch;
Servo servoYaw;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n=== 云台接收端（ESP-NOW） ===");

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
  Serial.println("初始化舵机...");
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  servoPitch.setPeriodHertz(50);
  servoPitch.attach(SERVO_PITCH_PIN, 500, 2400);

  servoYaw.setPeriodHertz(50);
  servoYaw.attach(SERVO_YAW_PIN, 500, 2400);

  // 舵机回到初始位置
  servoPitch.write(PITCH_CENTER);
  servoYaw.write(YAW_CENTER);
  delay(500);
  Serial.println("舵机初始化完成");

  Serial.println("系统初始化完成!");
  Serial.println("等待接收数据...");
}

void loop() {
  // 检查接收超时
  if (lastReceiveTime > 0 && millis() - lastReceiveTime > timeoutMs) {
    Serial.println("接收超时，等待数据...");
    lastReceiveTime = 0;
  }

  // 主循环无需控制舵机，由接收回调直接控制
  delay(10);
}

// 接收回调函数
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(myData)) {
    memcpy(&myData, incomingData, sizeof(myData));
    lastReceiveTime = millis();

    // 映射角度到舵机（1:1同步，无滤波，无死区）
    int servoPitchAngle = mapAngleToServo(myData.angleY);  // Pitch
    int servoYawAngle = mapAngleToServo(myData.angleZ);    // Yaw

    // 直接控制舵机
    servoPitch.write(servoPitchAngle);
    servoYaw.write(servoYawAngle);

    // 打印调试信息
    Serial.printf("接收: X=%.1f°, Y=%.1f°, Z=%.1f° | 舵机: Pitch=%d°, Yaw=%d°\n",
                  myData.angleX, myData.angleY, myData.angleZ, servoPitchAngle, servoYawAngle);
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
