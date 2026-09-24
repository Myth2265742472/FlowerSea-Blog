# ESP32 + MPU6050 姿态检测与可视化系统

## 项目简介

本项目是一个完整的姿态检测与可视化系统，基于 ESP32 开发板和 MPU6050 六轴传感器。系统实现了传感器数据采集、姿态解算、互补滤波、四元数转换，并通过串口将数据发送到 MATLAB 进行实时 3D 可视化。同时包含一个四电机控制版本，可用于无人机或机器人姿态控制。

## 功能特性

- ✅ **六轴数据采集**：同时读取加速度计和陀螺仪数据
- ✅ **姿态解算**：计算 Pitch、Roll、Yaw 三个姿态角
- ✅ **互补滤波**：融合加速度计和陀螺仪数据，提高精度
- ✅ **传感器校准**：自动校准零偏，消除累积误差
- ✅ **四元数输出**：支持四元数格式，便于 3D 可视化
- ✅ **MATLAB 可视化**：实时显示 3D 立方体姿态
- ✅ **WiFi 控制**：四电机控制版本支持 Web 远程控制

## 系统架构

```
┌─────────────────┐     串口/USB      ┌─────────────────┐
│   ESP32         │ ◄──────────────► │   MATLAB        │
│ + MPU6050       │    115200波特率   │ + 3D可视化       │
│                 │                  │                 │
└─────────────────┘                  └─────────────────┘
        │
        └────── WiFi AP (四电机版本)
               └── Web控制界面
```

## 所需材料

### 硬件
- **ESP32 开发板** (如 ESP32 DevKit V1)
- **MPU6050 传感器模块**
- **USB 数据线** 一条
- **杜邦线** 若干

### 四电机版本额外硬件
- **直流电机** x4
- **电机驱动模块** (如 L298N 或 DRV8833)
- **电源** (根据电机需求)

### 软件
- **Arduino IDE**
- **ESP32 开发板支持包**
- **MPU6050 库** (Arduino)
- **MATLAB** (用于可视化)

## 硬件连接

### ESP32 + MPU6050 基本连接

```
MPU6050       ESP32
-------       -----
VCC     ->    3.3V
GND     ->    GND
SCL     ->    GPIO 22 (I2C SCL)
SDA     ->    GPIO 21 (I2C SDA)
```

### 四电机版本额外连接

```
电机驱动       ESP32
--------       -----
电机1 AIN1 ->  GPIO 17
电机1 AIN2 ->  GPIO 18
电机1 PWMA ->  GPIO 16

电机2 BIN1 ->  GPIO 21
电机2 BIN2 ->  GPIO 22
电机2 PWMB ->  GPIO 19

电机3 AIN1 ->  GPIO 25
电机3 AIN2 ->  GPIO 26
电机3 PWMA ->  GPIO 23

电机4 BIN1 ->  GPIO 32
电机4 BIN2 ->  GPIO 33
电机4 PWMB ->  GPIO 27
```

## 软件准备

### 1. 安装 Arduino IDE

从 [Arduino 官网](https://www.arduino.cc/en/software) 下载并安装。

### 2. 添加 ESP32 支持

1. **文件 → 首选项 → 附加开发板管理器网址**：
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
2. **工具 → 开发板 → 开发板管理器 → 搜索 "ESP32" → 安装**

### 3. 安装 MPU6050 库

**工具 → 管理库 → 搜索 "MPU6050" → 安装 "MPU6050 by Electronic Cats"**

## 完整代码示例

### 示例 1：基础姿态检测（互补滤波）

```cpp
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

float accPitch, accRoll;
float gyroPitch = 0, gyroRoll = 0, gyroYaw = 0;
float pitch = 0, roll = 0, yaw = 0;
float lastTime = 0;
float offsetPitch = 0, offsetRoll = 0;
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;
const int calibCount = 200;
const float alpha = 0.98; // 互补滤波系数

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  // 校准
  long sumPitch = 0, sumRoll = 0;
  long sumGX = 0, sumGY = 0, sumGZ = 0;
  Serial.println("Calibrating... Keep MPU6050 still.");
  for (int i = 0; i < calibCount; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    float aPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / PI;
    float aRoll = atan2(ay, az) * 180 / PI;
    sumPitch += aPitch;
    sumRoll += aRoll;
    sumGX += gx;
    sumGY += gy;
    sumGZ += gz;
    delay(10);
  }
  offsetPitch = sumPitch / (float)calibCount;
  offsetRoll = sumRoll / (float)calibCount;
  gyroOffsetX = sumGX / (float)calibCount;
  gyroOffsetY = sumGY / (float)calibCount;
  gyroOffsetZ = sumGZ / (float)calibCount;
  Serial.println("Calibration done.");

  lastTime = millis() / 1000.0;
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 加速度计角度
  accPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / PI - offsetPitch;
  accRoll = atan2(ay, az) * 180 / PI - offsetRoll;

  // 陀螺仪角速度（deg/s），减去零偏
  float gPitch = (gy - gyroOffsetY) / 131.0;
  float gRoll  = (gx - gyroOffsetX) / 131.0;
  float gYaw   = (gz - gyroOffsetZ) / 131.0;

  // 时间间隔
  float now = millis() / 1000.0;
  float dt = now - lastTime;
  lastTime = now;

  // 互补滤波
  pitch = alpha * (pitch + gPitch * dt) + (1 - alpha) * accPitch;
  roll  = alpha * (roll  + gRoll  * dt) + (1 - alpha) * accRoll;
  yaw   += gYaw * dt; // Yaw只能用陀螺仪积分，易漂移

  // 限制Yaw在-180~180
  if (yaw > 180) yaw -= 360;
  if (yaw < -180) yaw += 360;

  // 输出整数
  Serial.print("P:"); Serial.print((int)pitch);
  Serial.print(" R:"); Serial.print((int)roll);
  Serial.print(" Y:"); Serial.println((int)yaw);

  delay(20); // 50Hz
}
```

### 示例 2：四元数输出（用于 MATLAB 可视化）

```cpp
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// 姿态角
float pitch = 0, roll = 0, yaw = 0;
float lastTime = 0;

// 校准参数
float offsetPitch = 0, offsetRoll = 0;
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;
const int calibCount = 200;
const float alpha = 0.98;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  // 校准（同示例1，省略）
  // ...

  lastTime = millis() / 1000.0;
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 计算姿态角（同示例1，省略）
  // ...

  // 转换为四元数
  float cy = cos(yaw * 0.5);
  float sy = sin(yaw * 0.5);
  float cp = cos(pitch * 0.5);
  float sp = sin(pitch * 0.5);
  float cr = cos(roll * 0.5);
  float sr = sin(roll * 0.5);

  float qw = cr * cp * cy + sr * sp * sy;
  float qx = sr * cp * cy - cr * sp * sy;
  float qy = cr * sp * cy + sr * cp * sy;
  float qz = cr * cp * sy - sr * sp * cy;

  // 输出四元数格式：Q,w,x,y,z
  Serial.print("Q,");
  Serial.print(qw, 4); Serial.print(",");
  Serial.print(qx, 4); Serial.print(",");
  Serial.print(qy, 4); Serial.print(",");
  Serial.println(qz, 4);

  delay(20);
}
```

### 示例 3：四电机控制 + WiFi

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <MPU6050.h>

// 创建MPU6050对象
MPU6050 mpu;

// 设置Wi-Fi接入点参数
const char* ssid = "123456789";
const char* password = "123456";

// 创建Web服务器对象，监听80端口
WebServer server(80);

// 定义电机控制引脚
const int motor1AIN1 = 17, motor1AIN2 = 18, motor1PWMA = 16;
const int motor2BIN1 = 21, motor2BIN2 = 22, motor2PWMB = 19;
const int motor3AIN1 = 25, motor3AIN2 = 26, motor3PWMA = 23;
const int motor4BIN1 = 32, motor4BIN2 = 33, motor4PWMB = 27;

// PID参数
float Kp = 1.0, Ki = 0.5, Kd = 0.1;
float setPoint = 0.0;
float error, previous_error = 0, integral = 0, derivative, output;
int motorSpeed1, motorSpeed2, motorSpeed3, motorSpeed4;
long dt = 10, lastTime;
int ax, ay, az, gx, gy, gz;

// HTML页面内容
const char* htmlPage = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>无人机操作界面</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>无人机操作</h1>
  <button onclick="sendCommand('start')">启动</button>
  <button onclick="sendCommand('stop')">停止</button>
  <h2>电机控制</h2>
  <button onclick="sendCommand('forward')">前进</button>
  <button onclick="sendCommand('backward')">后退</button>
  <button onclick="sendCommand('left')">左转</button>
  <button onclick="sendCommand('right')">右转</button>
  <script>
    function sendCommand(cmd) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/command?cmd=" + cmd, true);
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleCommand() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    Serial.println("Received command: " + cmd);
    if (cmd == "forward") {
      motorSpeed1 = motorSpeed2 = motorSpeed3 = motorSpeed4 = 200;
    } else if (cmd == "backward") {
      motorSpeed1 = motorSpeed2 = motorSpeed3 = motorSpeed4 = -200;
    } else if (cmd == "left") {
      motorSpeed1 = motorSpeed2 = 200;
      motorSpeed3 = motorSpeed4 = -200;
    } else if (cmd == "right") {
      motorSpeed1 = motorSpeed2 = -200;
      motorSpeed3 = motorSpeed4 = 200;
    }
    setMotorSpeed(motorSpeed1, motorSpeed2, motorSpeed3, motorSpeed4);
  }
  server.send(200, "text/plain", "OK");
}

void setMotorSpeed(int m1, int m2, int m3, int m4) {
  // 电机1
  digitalWrite(motor1AIN1, m1 > 0 ? HIGH : LOW);
  digitalWrite(motor1AIN2, m1 > 0 ? LOW : HIGH);
  analogWrite(motor1PWMA, constrain(abs(m1), 0, 255));

  // 电机2
  digitalWrite(motor2BIN1, m2 > 0 ? HIGH : LOW);
  digitalWrite(motor2BIN2, m2 > 0 ? LOW : HIGH);
  analogWrite(motor2PWMB, constrain(abs(m2), 0, 255));

  // 电机3
  digitalWrite(motor3AIN1, m3 > 0 ? HIGH : LOW);
  digitalWrite(motor3AIN2, m3 > 0 ? LOW : HIGH);
  analogWrite(motor3PWMA, constrain(abs(m3), 0, 255));

  // 电机4
  digitalWrite(motor4BIN1, m4 > 0 ? HIGH : LOW);
  digitalWrite(motor4BIN2, m4 > 0 ? LOW : HIGH);
  analogWrite(motor4PWMB, constrain(abs(m4), 0, 255));
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050连接失败");
    while (1);
  }

  // 初始化电机控制引脚
  pinMode(motor1AIN1, OUTPUT); pinMode(motor1AIN2, OUTPUT); pinMode(motor1PWMA, OUTPUT);
  pinMode(motor2BIN1, OUTPUT); pinMode(motor2BIN2, OUTPUT); pinMode(motor2PWMB, OUTPUT);
  pinMode(motor3AIN1, OUTPUT); pinMode(motor3AIN2, OUTPUT); pinMode(motor3PWMA, OUTPUT);
  pinMode(motor4BIN1, OUTPUT); pinMode(motor4BIN2, OUTPUT); pinMode(motor4PWMB, OUTPUT);
  
  setMotorSpeed(0, 0, 0, 0);
  delay(2000);

  // 启动Wi-Fi接入点
  WiFi.softAP(ssid, password);
  Serial.print("AP IP地址: ");
  Serial.println(WiFi.softAPIP());

  // 设置Web服务器的路由
  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.begin();
}

void loop() {
  server.handleClient();
  
  // 这里可以添加PID控制逻辑
  // mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  // 计算姿态...
  // 计算PID输出...
  // 设置电机速度...
}
```

### 示例 4：MATLAB 3D 可视化

```matlab
% Live cube visualization from ESP32+MPU6050 quaternion stream
% Protocol: text lines "Q,w,x,y,z" at 115200 baud

clear; clc;

% ---- CONFIG ----
portName = "COM4";   % change to your port
baud = 115200;

% ---- SERIAL ----
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

sp = serialport(portName, baud, "Timeout", 1);
configureTerminator(sp, "LF");
flush(sp);

% ---- FIGURE ----
figure('Name','MPU6050 Cube','Color','w');
axis equal; grid on; view(35, 20);
xlabel('X'); ylabel('Y'); zlabel('Z');
xlim([-1 1]); ylim([-1 1]); zlim([-1 1]);
hold on;

% Create a unit cube centered at origin
v = 0.2; % half-size
verts = [
    -v -v -v;
     v -v -v;
     v  v -v;
    -v  v -v;
    -v -v  v;
     v -v  v;
     v  v  v;
    -v  v  v];
faces = [
    1 2 3 4;  % bottom
    5 6 7 8;  % top
    1 2 6 5;  % sides
    2 3 7 6;
    3 4 8 7;
    4 1 5 8];
col = [0.8 0.8 1.0];
ph = patch('Vertices', verts, 'Faces', faces, 'FaceColor', col, 'FaceAlpha', 0.8);

% Add axis triad
quiver3(0,0,0, 0.4,0,0, 'r','LineWidth',2); % X
quiver3(0,0,0, 0,0.4,0, 'g','LineWidth',2); % Y
quiver3(0,0,0, 0,0,0.4, 'b','LineWidth',2); % Z

% Quaternion to rotation matrix helper
quat2rotm_local = @(q) [
    1-2*(q(3)^2+q(4)^2), 2*(q(2)*q(3)-q(1)*q(4)), 2*(q(2)*q(4)+q(1)*q(3));
    2*(q(2)*q(3)+q(1)*q(4)), 1-2*(q(2)^2+q(4)^2), 2*(q(3)*q(4)-q(1)*q(2));
    2*(q(2)*q(4)-q(1)*q(3)), 2*(q(3)*q(4)+q(1)*q(2)), 1-2*(q(2)^2+q(3)^2)];

disp('Waiting for quaternion data...');

% ---- LOOP ----
while isvalid(sp) && ishghandle(ph)
    try
        line = readline(sp);
    catch
        pause(0.001);
        continue;
    end
    if strlength(line) == 0
        continue;
    end
    parts = split(strtrim(line), ',');
    if numel(parts) ~= 5
        continue;
    end
    if parts{1} ~= "Q"
        continue;
    end
    q = zeros(4,1);
    for i=1:4
        v = str2double(parts{i+1});
        if isnan(v), v = 0; end
        q(i) = v;
    end
    % Normalize to be safe
    q = q / norm(q + 1e-12);
    R = quat2rotm_local(q');

    % Rotate cube vertices
    vertsR = (R * verts')';
    set(ph, 'Vertices', vertsR);

    drawnow limitrate;
end

% Cleanup
if isvalid(sp)
    clear sp;
end
```

## 代码详解

### 1. 互补滤波算法

```cpp
pitch = alpha * (pitch + gPitch * dt) + (1 - alpha) * accPitch;
```

- **alpha = 0.98**：信任陀螺仪 98%，加速度计 2%
- **优点**：结合两者优势，消除漂移和高频噪声
- **缺点**：Yaw 角无法通过加速度计校正，会随时间漂移

### 2. 四元数转换

```cpp
float qw = cr * cp * cy + sr * sp * sy;
float qx = sr * cp * cy - cr * sp * sy;
float qy = cr * sp * cy + sr * cp * sy;
float qz = cr * cp * sy - sr * sp * cy;
```

四元数避免了万向节锁问题，适合 3D 旋转和插值。

### 3. MATLAB 可视化

- **串口通信**：通过 `serialport` 读取 ESP32 发送的数据
- **四元数转旋转矩阵**：`quat2rotm_local` 函数
- **3D 图形**：使用 `patch` 创建立方体，实时更新顶点位置

## 使用方法

### 基础姿态检测

1. 上传示例 1 代码到 ESP32
2. 打开串口监视器（115200 波特率）
3. 保持 MPU6050 静止，等待校准完成
4. 移动传感器，观察姿态角变化

### MATLAB 可视化

1. 上传示例 2 代码到 ESP32（四元数输出）
2. 在 MATLAB 中运行示例 4 代码
3. 修改 `portName` 为正确的串口号
4. 观察 3D 立方体随传感器姿态实时旋转

### WiFi 控制

1. 上传示例 3 代码到 ESP32
2. 手机连接 WiFi "123456789"，密码 "123456"
3. 浏览器访问 `192.168.4.1`
4. 点击按钮控制电机

## 扩展功能

### 1. 添加 PID 控制

```cpp
// PID 计算
error = setPoint - currentPitch;
integral += error * dt;
derivative = (error - previous_error) / dt;
output = Kp * error + Ki * integral + Kd * derivative;
previous_error = error;

// 应用输出到电机
setMotorSpeed(output, output, output, output);
```

### 2. 数据记录到 SD 卡

```cpp
#include <SD.h>
File dataFile = SD.open("/姿态数据.txt", FILE_APPEND);
dataFile.printf("%.2f,%.2f,%.2f\n", pitch, roll, yaw);
dataFile.close();
```

### 3. 蓝牙传输

使用 ESP32 的蓝牙功能，将数据发送到手机 APP 显示。

## 常见问题

### 1. MPU6050 连接失败
- 检查 I2C 接线（SCL→GPIO22，SDA→GPIO21）
- 确认供电正常（3.3V）
- 检查是否安装了 MPU6050 库

### 2. 姿态角漂移
- Yaw 角漂移是正常的，需要磁力计校正
- 增加校准时间，确保传感器静止
- 检查互补滤波系数是否合适

### 3. MATLAB 无法读取数据
- 确认串口号正确
- 检查波特率是否为 115200
- 确认数据格式为 "Q,w,x,y,z"

### 4. 电机不转
- 检查电机驱动模块供电
- 确认 PWM 引脚配置正确
- 检查电机是否堵转

## 参考文档

- [MPU6050 数据手册](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [互补滤波算法详解 - The Balance Filter](https://www.cnblogs.com/portb/p/12037338.html)
- [四元数与三维旋转 - 博客园详解](https://www.cnblogs.com/leixinyue/p/13469155.html)
- [MATLAB Serial 文档](https://www.mathworks.com/help/matlab/serial-port-devices.html)

---

**注意**：使用电机时请注意安全，确保固定牢固。高速旋转的电机可能造成危险，测试时请注意防护。
