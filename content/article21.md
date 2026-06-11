# ESP32 舵机控制全攻略：从基础到高级应用

## 项目简介

本项目合集展示了 ESP32 与舵机（Servo）的多种控制方式，从最简单的单舵机测试，到基于 MPU6050 姿态传感器的无线同步控制，再到结合 Python 计算机视觉的头部追踪控制。涵盖了有线/无线通信、传感器融合、卡尔曼滤波、PCA9685 扩展驱动等核心技术，是学习嵌入式舵机控制的完整教程。

## 功能特性

- **单舵机基础控制**：通过串口命令控制舵机角度
- **MPU6050 有线同步**：倾斜传感器直接控制舵机角度
- **ESP-NOW 无线同步**：双 ESP32 无线传输姿态数据控制舵机
- **二维舵机云台**：双轴卡尔曼滤波姿态同步云台
- **Python 头部追踪**：MediaPipe 人脸检测 + 串口控制多路舵机
- **PCA9685 扩展驱动**：支持最多 16 路舵机同时控制

## 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                     舵机控制系统架构                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐    I2C     ┌─────────────┐                │
│  │   MPU6050   │◄──────────►│    ESP32    │───► 舵机        │
│  │  姿态传感器  │            │  (有线同步)  │                │
│  └─────────────┘            └─────────────┘                │
│                                                             │
│  ┌─────────────┐   ESP-NOW   ┌─────────────┐               │
│  │ESP32+MPU6050│◄───────────►│    ESP32    │───► 舵机       │
│  │  (发送端)   │  无线传输    │  (接收端)   │               │
│  └─────────────┘             └─────────────┘               │
│                                                             │
│  ┌─────────────┐   串口/USB  ┌─────────────┐               │
│  │  Python     │◄───────────►│    ESP32    │───► 多路舵机   │
│  │ MediaPipe   │  头部追踪    │ + PCA9685   │   (16通道)    │
│  └─────────────┘             └─────────────┘               │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 所需材料

### 硬件

| 组件 | 数量 | 说明 |
|------|------|------|
| ESP32 开发板 | 2-3 块 | ESP32 或 ESP32-S3 |
| SG90 舵机 | 2-4 个 | 小型 9g 舵机 |
| MPU6050 传感器 | 1-2 个 | 六轴姿态传感器 |
| PCA9685 驱动板 | 1 个 | 16 通道 PWM 驱动（可选）|
| 杜邦线 | 若干 | 公对母/母对母 |
| 5V 电源适配器 | 1 个 | 2A 以上（舵机供电）|
| USB 摄像头 | 1 个 | 用于头部追踪（可选）|

### 软件

- **Arduino IDE**
- **ESP32 开发板支持包**
- **Python 3.8+**（头部追踪项目）
- **OpenCV、MediaPipe、pyserial**（Python 依赖）

### 所需库

```cpp
// Arduino 库
#include <ESP32Servo.h>      // ESP32 舵机控制
#include <Wire.h>            // I2C 通信
#include <esp_now.h>         // ESP-NOW 无线通信
#include <WiFi.h>            // WiFi 功能
#include <Adafruit_PWMServoDriver.h>  // PCA9685 驱动
#include <MPU6050.h>         // MPU6050 库（部分项目）
```

```bash
# Python 依赖
pip install opencv-python mediapipe pyserial numpy
```

---

## 项目一：单舵机基础控制（PCA9685 驱动）

### 项目简介

最基础的舵机控制项目，使用 PCA9685 16 通道 PWM 驱动板控制单个舵机，支持串口命令交互。适合初学者了解舵机的基本工作原理和控制方式。

### 硬件连接

```
PCA9685 驱动板          ESP32
┌─────────────┐        ┌─────────┐
│  VCC  → 3.3V│        │         │
│  GND  → GND │        │  GPIO21 ─── SDA
│  SCL  → GPIO22│      │  GPIO22 ─── SCL
│  SDA  → GPIO21│      │         │
└─────────────┘        └─────────┘

舵机 SG90
┌───────────┐
│ 红线 → 5V  │  (外部电源)
│ 棕线 → GND │  (共地)
│ 橙线 → PCA9685 通道0
└───────────┘
```

### 核心代码

```cpp
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// 舵机配置
static const int SERVO_MIN_PULSE = 150;   // 0.5ms
static const int SERVO_MAX_PULSE = 600;   // 2.5ms

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22, 400000);  // SDA, SCL, 400kHz
  
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);  // 50Hz 标准舵机频率
  
  // 初始化到 90 度
  setServoAngle(0, 90);
}

void setServoAngle(int channel, float angle) {
  angle = constrain(angle, 0, 180);
  int pulseWidth = map(angle, 0, 180, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  pwm.setPWM(channel, 0, pulseWidth);
}
```

### 串口命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `set <角度>` | 平滑移动到目标角度 | `set 90` |
| `move <角度>` | 立即移动到目标角度 | `move 45` |
| `center` | 回到中心位置 | `center` |
| `test` | 运行测试序列 | `test` |
| `status` | 显示当前角度 | `status` |
| `help` | 显示帮助 | `help` |

---

## 项目二：MPU6050 有线同步控制舵机

### 项目简介

将 MPU6050 姿态传感器与舵机直接连接在同一 ESP32 上，实现"倾斜即转动"的同步效果。通过读取加速度计数据计算倾斜角度，直接映射到舵机角度。

### 硬件连接

```
同一 ESP32-S3 开发板
┌─────────────────────────────────────┐
│                                     │
│  GPIO5  ───── SDA ────► MPU6050    │
│  GPIO6  ───── SCL ────► MPU6050    │
│  3.3V   ───── VCC ────► MPU6050    │
│  GND    ───── GND ────► MPU6050    │
│                                     │
│  GPIO7  ───── 信号 ────► 舵机       │
│  5V     ───── VCC ────► 舵机(外部)  │
│  GND    ───── GND ────► 舵机(共地)  │
│                                     │
└─────────────────────────────────────┘
```

### 核心代码

```cpp
#include <Wire.h>
#include <ESP32Servo.h>

#define MPU6050_ADDR 0x68
#define MPU6050_SDA_PIN 5
#define MPU6050_SCL_PIN 6
#define SERVO_PIN 7

Servo myservo;
float angleX, angleY;
float offsetX = 0, offsetY = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(MPU6050_SDA_PIN, MPU6050_SCL_PIN);
  
  // 初始化 MPU6050
  initMPU6050();
  
  // 初始化舵机
  ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);
  
  // 归零校准
  calibrateOffsets();
  myservo.write(90);
}

void loop() {
  readMPU6050Data();
  calculateAngles();
  
  // 映射角度到舵机 (0-180度)
  int servoAngle = (int)((angleX + 90) * 180.0 / 180.0);
  servoAngle = constrain(servoAngle, 0, 180);
  
  myservo.write(servoAngle);
  
  Serial.printf("X=%.1f° | 舵机: %d°\n", angleX, servoAngle);
  delay(50);
}

// 计算倾斜角度
void calculateAngles() {
  // 从加速度计计算 Roll 和 Pitch
  angleX = atan2(accelY, accelZ) * 180.0 / PI - offsetX;
  angleY = atan2(-accelX, sqrt(accelY*accelY + accelZ*accelZ)) * 180.0 / PI - offsetY;
  
  // 限制范围
  angleX = constrain(angleX, -90, 90);
  angleY = constrain(angleY, -90, 90);
}
```

### 角度映射原理

```
MPU6050 倾斜角度          舵机角度
    -90°  ──────────────────►  0°
      0°  ──────────────────►  90°  (中心)
    +90°  ──────────────────►  180°
```

---

## 项目三：MPU6050 无线同步控制舵机（ESP-NOW）

### 项目简介

使用 ESP-NOW 协议实现双 ESP32 无线通信：发送端读取 MPU6050 姿态数据，通过 ESP-NOW 无线传输到接收端，接收端控制舵机跟随转动。无需 WiFi 路由器，点对点直连，延迟低、配置简单。

### 系统架构

```
发送端 (ESP32-S3 + MPU6050)          接收端 (ESP32 + 舵机)
┌─────────────────────┐             ┌─────────────────────┐
│  读取 MPU6050 数据   │             │  接收 ESP-NOW 数据   │
│  ↓                  │  ESP-NOW    │  ↓                  │
│  计算倾斜角度        │◄───────────►│  映射到舵机角度      │
│  ↓                  │  无线传输    │  ↓                  │
│  封装数据结构        │             │  平滑滤波 + 死区控制 │
│  ↓                  │             │  ↓                  │
│  esp_now_send()     │             │  servo.write()      │
└─────────────────────┘             └─────────────────────┘
```

### 发送端代码

```cpp
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

uint8_t receiverMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// 数据结构
typedef struct struct_message {
  float angleX;
  float angleY;
  float angleZ;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  // 初始化 ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW 初始化失败");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
  
  // 注册接收端
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
  
  // 初始化 MPU6050
  Wire.begin(5, 6);
  initMPU6050();
  calibrateOffsets();
}

void loop() {
  readMPU6050Data();
  
  myData.angleX = angleX;
  myData.angleY = angleY;
  myData.angleZ = angleZ;
  
  // 发送数据
  esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&myData, sizeof(myData));
  
  Serial.printf("发送: X=%.1f°, Y=%.1f°\n", angleX, angleY);
  delay(30);  // 约 33Hz 更新率
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("发送失败");
  }
}
```

### 接收端代码

```cpp
#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

#define SERVO_PIN 5
#define FILTER_SAMPLES 15
#define DEAD_ZONE 2.0

Servo myservo;
struct_message myData;
float servoAngle_history[FILTER_SAMPLES];
int filter_index = 0;
int lastServoAngle = 90;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
  
  // 初始化舵机
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);
  
  // 初始化滤波历史
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    servoAngle_history[i] = 90;
  }
  
  myservo.write(90);
  Serial.print("本设备 MAC: ");
  Serial.println(WiFi.macAddress());
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  // 映射角度到舵机
  int servoAngle = (int)((myData.angleX + 90) * 180.0 / 180.0);
  servoAngle = constrain(servoAngle, 0, 180);
  
  // 平滑滤波
  servoAngle_history[filter_index] = servoAngle;
  filter_index = (filter_index + 1) % FILTER_SAMPLES;
  
  float sum = 0;
  for (int i = 0; i < FILTER_SAMPLES; i++) sum += servoAngle_history[i];
  servoAngle = (int)(sum / FILTER_SAMPLES);
  
  // 死区控制：角度变化小于阈值不更新
  if (abs(servoAngle - lastServoAngle) >= DEAD_ZONE) {
    myservo.write(servoAngle);
    lastServoAngle = servoAngle;
  }
}
```

### ESP-NOW 配置步骤

1. **上传接收端程序**：先上传接收端代码，打开串口监视器记录 MAC 地址
2. **修改发送端 MAC**：将接收端 MAC 地址填入发送端代码
3. **上传发送端程序**：上传修改后的发送端代码
4. **测试**：倾斜 MPU6050，观察舵机是否同步转动

### 平滑滤波与死区控制

```cpp
// 平滑滤波：移动平均，减少抖动
#define FILTER_SAMPLES 15
float history[FILTER_SAMPLES];

int smoothServoAngle(int angle) {
  history[index] = angle;
  index = (index + 1) % FILTER_SAMPLES;
  
  float sum = 0;
  for (int i = 0; i < FILTER_SAMPLES; i++) sum += history[i];
  return (int)(sum / FILTER_SAMPLES);
}

// 死区控制：小角度变化不响应
#define DEAD_ZONE 2.0
if (abs(newAngle - lastAngle) >= DEAD_ZONE) {
  servo.write(newAngle);
  lastAngle = newAngle;
}
```

---

## 项目四：二维舵机云台（卡尔曼滤波 + 双轴控制）

### 项目简介

进阶项目，使用卡尔曼滤波器融合加速度计和陀螺仪数据，实现更精确的姿态估计。控制两个舵机构成二维云台，分别对应俯仰（Pitch）和平面转角（Yaw），实现 1:1 姿态同步。

### 卡尔曼滤波器原理

卡尔曼滤波器通过预测-更新循环，融合传感器数据：

```
预测步骤：
  angle += dt * (gyro - bias)
  更新协方差矩阵 P

更新步骤：
  计算卡尔曼增益 K
  angle += K * (accel_angle - angle)
  bias += K * (accel_angle - angle)
```

### 发送端（卡尔曼滤波 + ESP-NOW）

```cpp
// 卡尔曼滤波器参数
const float Q_angle = 0.0005;   // 角度过程噪声
const float Q_gyro = 0.002;     // 陀螺仪过程噪声
const float R_angle = 0.015;    // 测量噪声

// 卡尔曼滤波器变量
float angle_roll = 0, bias_roll = 0;
float P_roll[2][2] = {{0, 0}, {0, 0}};

float angle_pitch = 0, bias_pitch = 0;
float P_pitch[2][2] = {{0, 0}, {0, 0}};

void kalmanUpdate(float &angle, float &bias, float P[2][2],
                  float newAngle, float newRate, float dt) {
  // 预测
  angle += dt * (newRate - bias);
  P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
  P[0][1] -= dt * P[1][1];
  P[1][0] -= dt * P[1][1];
  P[1][1] += Q_gyro * dt;
  
  // 更新
  float S = P[0][0] + R_angle;
  float K0 = P[0][0] / S;
  float K1 = P[1][0] / S;
  float y = newAngle - angle;
  
  angle += K0 * y;
  bias += K1 * y;
  
  // 更新协方差
  float P00_temp = P[0][0];
  float P01_temp = P[0][1];
  P[0][0] -= K0 * P00_temp;
  P[0][1] -= K0 * P01_temp;
  P[1][0] -= K1 * P00_temp;
  P[1][1] -= K1 * P01_temp;
}

void readMPU6050Data() {
  // 读取原始数据
  int16_t rawAccelX = ...;
  int16_t rawGyroX = ...;
  
  // 转换为物理量
  float accelX = rawAccelX / 4096.0;  // ±8g
  float gyroX = rawGyroX / 65.5;       // ±500°/s
  
  // 计算时间间隔
  float dt = ...;
  
  // 从加速度计计算角度
  float roll_acc = atan2(accelY, accelZ) * 180.0 / PI;
  float pitch_acc = atan2(-accelX, sqrt(accelY*accelY + accelZ*accelZ)) * 180.0 / PI;
  
  // 卡尔曼滤波更新
  kalmanUpdate(angle_roll, bias_roll, P_roll, roll_acc, gyroX, dt);
  kalmanUpdate(angle_pitch, bias_pitch, P_pitch, pitch_acc, gyroY, dt);
  
  // Yaw 角积分
  angle_yaw += gyroZ * dt;
}
```

### 接收端（双舵机 1:1 同步）

```cpp
#define SERVO_PITCH_PIN  12
#define SERVO_YAW_PIN    13

Servo servoPitch;
Servo servoYaw;

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  // 1:1 同步映射
  int pitchAngle = mapAngleToServo(myData.angleY);
  int yawAngle = mapAngleToServo(myData.angleZ);
  
  // 直接控制（无滤波，追求最低延迟）
  servoPitch.write(pitchAngle);
  servoYaw.write(yawAngle);
}

int mapAngleToServo(float angle) {
  int servoAngle = (int)((angle + 90) * 180.0 / 180.0);
  return constrain(servoAngle, 0, 180);
}
```

### 硬件连接

```
发送端：
  ESP32-S3 + MPU6050 (GPIO17=SDA, GPIO18=SCL)

接收端：
  ESP32 + 舵机1(俯仰, GPIO12) + 舵机2(平面转角, GPIO13)
  舵机使用独立 5V 电源，与 ESP32 共地
```

---

## 项目五：Python 头部追踪 + 舵机同步

### 项目简介

使用 Python + MediaPipe 进行实时人脸检测，通过计算头部偏航角（Yaw）控制舵机转动。支持直接控制单个舵机，或通过 PCA9685 控制最多 16 路舵机。适用于头部追踪摄像头、交互式装置等场景。

### 系统架构

```
摄像头 ──► Python 程序 ──► 串口 ──► ESP32 ──► 舵机/PCA9685
            │
            ├── MediaPipe FaceMesh (468 个面部特征点)
            ├── 计算头部偏航角
            ├── 平滑处理
            └── 串口发送角度命令
```

### Python 代码

```python
import cv2
import mediapipe as mp
import serial
import math
import time

class YawEstimator:
    def __init__(self, camera_index=0):
        self.cap = cv2.VideoCapture(camera_index)
        self.mp_face = mp.solutions.face_mesh
        self.face = self.mp_face.FaceMesh(
            static_image_mode=False,
            max_num_faces=1,
            min_detection_confidence=0.5,
            min_tracking_confidence=0.5
        )
    
    def compute_yaw_norm(self, frame):
        """计算头部偏航角，范围 [-1, 1]"""
        h, w = frame.shape[:2]
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        res = self.face.process(rgb)
        
        if not res.multi_face_landmarks:
            return None
        
        lm = res.multi_face_landmarks[0].landmark
        
        # 关键特征点
        # 33: 左眼外角, 263: 右眼外角, 1: 鼻尖
        lx, ly = lm[33].x * w, lm[33].y * h
        rx, ry = lm[263].x * w, lm[263].y * h
        nx, ny = lm[1].x * w, lm[1].y * h
        
        # 计算眼睛中心
        cx = (lx + rx) * 0.5
        
        # 眼间距
        eye_dist = math.hypot(rx - lx, ry - ly)
        if eye_dist <= 1e-6:
            return None
        
        # 偏航角：鼻尖相对于眼睛中心的水平偏移
        yaw_norm = (nx - cx) / eye_dist * 2.0
        return max(-1.0, min(1.0, yaw_norm))

def yaw_to_angle(yaw_norm, center=90, max_delta=60):
    """将偏航角映射到舵机角度"""
    angle = int(center + yaw_norm * max_delta)
    return max(0, min(180, angle))

def send_angle(ser, angle, channel=None):
    """发送角度命令到 ESP32"""
    angle = max(0, min(180, int(angle)))
    if channel is None:
        cmd = f"ANGLE {angle}\n"
    else:
        cmd = f"CHANGLE {channel} {angle}\n"
    ser.write(cmd.encode("utf-8"))
    print(f"[TX] {cmd.strip()}")

# 主程序
ser = serial.Serial('COM5', 115200, timeout=0.1)
time.sleep(2)

tracker = YawEstimator()
last_angle = None

while True:
    frame = tracker.read_frame()
    yaw_norm = tracker.compute_yaw_norm(frame)
    
    if yaw_norm is not None:
        angle = yaw_to_angle(yaw_norm)
        
        # 平滑步进限制
        if last_angle is not None:
            if abs(angle - last_angle) > 4:
                angle = last_angle + (4 if angle > last_angle else -4)
        
        send_angle(ser, angle)
        last_angle = angle
    
    # 显示调试信息
    cv2.imshow('Head Tracking', frame)
    if cv2.waitKey(1) & 0xFF == 27:  # ESC 退出
        break

ser.close()
cv2.destroyAllWindows()
```

### ESP32 接收端（支持 PCA9685）

```cpp
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVO_PIN 14
static const uint8_t PCA9685_ADDR = 0x40;

Servo servo;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PCA9685_ADDR);
int chAngle[16];
int currentAngle = 90;

// 命令解析
void processLine(String line) {
    line.trim();
    int s1 = line.indexOf(' ');
    String cmd = (s1 < 0) ? line : line.substring(0, s1);
    String rest = (s1 < 0) ? "" : line.substring(s1 + 1);
    cmd.toUpperCase();
    
    if (cmd == "ANGLE" && rest.length() > 0) {
        int angle = rest.toInt();
        setAngle(angle);
    } else if (cmd == "CHANGLE") {
        int s = rest.indexOf(' ');
        if (s > 0) {
            int ch = rest.substring(0, s).toInt();
            int angle = rest.substring(s + 1).toInt();
            setChannelAngle(ch, angle);
        }
    } else if (cmd == "INC" || cmd == "DEC") {
        int d = rest.toInt();
        if (cmd == "DEC") d = -d;
        setAngle(currentAngle + d);
    }
}

void setAngle(int angle) {
    currentAngle = constrain(angle, 0, 180);
    int pulseUs = 500 + (int)(2000 * (currentAngle / 180.0));
    servo.writeMicroseconds(pulseUs);
}

void setChannelAngle(int ch, int angle) {
    if (ch < 0 || ch > 15) return;
    chAngle[ch] = constrain(angle, 0, 180);
    int pulseUs = 500 + (int)(2000 * (chAngle[ch] / 180.0));
    int offCount = map(pulseUs, 0, 20000, 0, 4095);
    pwm.setPWM(ch, 0, offCount);
}

void setup() {
    Serial.begin(115200);
    
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    servo.setPeriodHertz(50);
    servo.attach(SERVO_PIN, 500, 2500);
    setAngle(90);
    
    // PCA9685 初始化
    Wire.begin();
    pwm.begin();
    pwm.setPWMFreq(50);
    for (int i = 0; i < 16; ++i) {
        chAngle[i] = 90;
        setChannelAngle(i, 90);
    }
}

void loop() {
    static String line;
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (line.length() > 0) {
                processLine(line);
                line = "";
            }
        } else {
            line += c;
        }
    }
}
```

### 命令格式

| 命令 | 说明 | 示例 |
|------|------|------|
| `ANGLE <角度>` | 设置单舵机角度 | `ANGLE 90` |
| `INC <增量>` | 增加角度 | `INC 5` |
| `DEC <减量>` | 减少角度 | `DEC 5` |
| `CHANGLE <通道> <角度>` | 设置 PCA9685 通道角度 | `CHANGLE 0 90` |
| `CHINC <通道> <增量>` | 增加指定通道角度 | `CHINC 0 5` |
| `CHDEC <通道> <减量>` | 减少指定通道角度 | `CHDEC 0 5` |

---

## 常见问题与解决方案

### 1. 舵机抖动严重

**原因**：
- 电源供电不足
- 传感器数据噪声大
- 更新频率过高

**解决方案**：
- 使用独立 5V 电源给舵机供电（至少 2A）
- 增加平滑滤波（增大 `FILTER_SAMPLES`）
- 增大死区值（`DEAD_ZONE`）
- 降低数据更新频率

### 2. 舵机不转动

**原因**：
- 电源电流不足
- 信号线连接错误
- 引脚配置错误

**解决方案**：
- 检查电源是否能提供足够电流（SG90 约 100-250mA，MG996R 约 500-900mA）
- 确认信号线连接到正确的 GPIO
- 检查 ESP32Servo 库的定时器分配

### 3. ESP-NOW 连接失败

**原因**：
- MAC 地址配置错误
- 设备距离过远
- 信道不匹配

**解决方案**：
- 确认接收端 MAC 地址正确填入发送端
- 两个设备距离保持在 10 米以内
- 确保两个设备使用相同信道

### 4. MPU6050 检测不到

**原因**：
- I2C 接线错误
- 地址冲突
- 上拉电阻缺失

**解决方案**：
- 检查 SDA/SCL 是否接反
- 确认 MPU6050 地址（默认 0x68，AD0 接高为 0x69）
- 添加 4.7kΩ 上拉电阻（部分模块已内置）

### 5. Python 程序无法识别摄像头

**原因**：
- 摄像头被占用
- 驱动问题
- 索引错误

**解决方案**：
- 关闭其他使用摄像头的程序
- 尝试不同的摄像头索引（0, 1, 2）
- 检查摄像头驱动是否安装

---

## 进阶优化建议

### 1. 电源优化

```
推荐供电方案：

5V 电源适配器 (3A+)
    ├──► 舵机1 VCC (红线)
    ├──► 舵机2 VCC (红线)
    └──► 公共地线
            ├──► 舵机1 GND (棕线)
            ├──► 舵机2 GND (棕线)
            └──► ESP32 GND (共地！)

ESP32 单独 USB 供电
```

### 2. 卡尔曼滤波参数调优

| 参数 | 作用 | 调大效果 | 调小效果 |
|------|------|---------|---------|
| `Q_angle` | 角度过程噪声 | 更信任模型预测 | 更信任测量值 |
| `Q_gyro` | 陀螺仪噪声 | 更信任陀螺仪 | 更信任加速度计 |
| `R_angle` | 测量噪声 | 更平滑但延迟大 | 更灵敏但可能抖动 |

### 3. 多路舵机扩展

使用 PCA9685 可以扩展至 16 路舵机：

```cpp
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// 控制第 0-15 通道的舵机
for (int ch = 0; ch < 16; ch++) {
    setChannelAngle(ch, 90);  // 全部初始化到 90 度
}
```

---

## 参考文档

- [ESP32Servo 库文档](https://github.com/madhephaestus/ESP32Servo)
- [Adafruit PCA9685 教程](https://learn.adafruit.com/16-channel-pwm-servo-driver)
- [ESP-NOW 官方文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [MediaPipe FaceMesh 文档](https://developers.google.com/mediapipe/solutions/vision/face_landmarker)
- [MPU6050 数据手册](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [卡尔曼滤波器详解](https://www.bzarg.com/p/how-a-kalman-filter-works-in-pictures/)

---

**注意**：使用舵机时请注意安全，确保固定牢固。高速旋转的舵机可能造成危险，测试时请注意防护。舵机供电必须使用独立电源，不可直接使用 ESP32 的 5V 引脚驱动多个舵机。
