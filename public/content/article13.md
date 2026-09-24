# ESP32 网络遥控车开发指南

## 项目简介

本项目展示了如何使用 ESP32 开发板构建一个可以通过 WiFi 网络远程控制的智能小车。通过手机或电脑浏览器，即可实现前进、后退、转向等基本控制功能，是学习物联网和无线控制的绝佳入门项目。

## 功能特性

- ✅ **WiFi 控制**：通过浏览器访问 ESP32 的 IP 地址即可控制
- ✅ **多种控制模式**：支持前进、后退、左转、右转、停止
- ✅ **Web 界面**：内置简单的网页控制面板
- ✅ **实时响应**：低延迟的无线控制体验
- ✅ **扩展性强**：可轻松添加传感器、摄像头等模块

## 所需材料

### 硬件
- **ESP32 开发板** (如 ESP32 DevKit V1)
- **L298N 电机驱动模块** 或 **TB6612FNG**
- **直流减速电机** x2
- **智能小车底盘** 一套
- **18650 锂电池** 或 **7.4V 锂电池**
- **杜邦线** 若干

### 软件
- **Arduino IDE**
- **ESP32 开发板支持包**

## 硬件连接

### 电机驱动连接（以 L298N 为例）

```
ESP32          L298N
-----          -----
GPIO 12  ->    IN1 (左电机方向)
GPIO 13  ->    IN2 (左电机方向)
GPIO 14  ->    IN3 (右电机方向)
GPIO 27  ->    IN4 (右电机方向)
GPIO 25  ->    ENA (左电机PWM调速)
GPIO 26  ->    ENB (右电机PWM调速)

L298N          电源
-----          -----
+12V     ->    电池正极
GND      ->    电池负极、ESP32 GND

L298N          电机
-----          -----
OUT1/OUT2 ->   左电机
OUT3/OUT4 ->   右电机
```

### 完整接线图

```
电池(7.4V)
   |
   +---> L298N +12V
   |
   +---> L298N GND -----> ESP32 GND
   |
ESP32
   |
   +-- GPIO 12 ---> L298N IN1
   +-- GPIO 13 ---> L298N IN2
   +-- GPIO 14 ---> L298N IN3
   +-- GPIO 27 ---> L298N IN4
   +-- GPIO 25 ---> L298N ENA
   +-- GPIO 26 ---> L298N ENB
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

### 3. 选择开发板

**工具 → 开发板 → ESP32 Arduino → ESP32 Dev Module**

## 完整代码

```cpp
#include <WiFi.h>
#include <WebServer.h>

// WiFi 配置
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// 电机引脚定义
#define LEFT_IN1  12
#define LEFT_IN2  13
#define RIGHT_IN3 14
#define RIGHT_IN4 27
#define LEFT_ENA  25
#define RIGHT_ENB 26

// PWM 配置
#define PWM_FREQ     1000
#define PWM_RES      8
#define LEFT_PWM_CH  0
#define RIGHT_PWM_CH 1

WebServer server(80);

// 电机控制函数
void motorInit() {
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);
  
  ledcSetup(LEFT_PWM_CH, PWM_FREQ, PWM_RES);
  ledcSetup(RIGHT_PWM_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(LEFT_ENA, LEFT_PWM_CH);
  ledcAttachPin(RIGHT_ENB, RIGHT_PWM_CH);
  
  stop();
}

void forward(int speed = 200) {
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, HIGH);
  digitalWrite(RIGHT_IN4, LOW);
  ledcWrite(LEFT_PWM_CH, speed);
  ledcWrite(RIGHT_PWM_CH, speed);
}

void backward(int speed = 200) {
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, HIGH);
  ledcWrite(LEFT_PWM_CH, speed);
  ledcWrite(RIGHT_PWM_CH, speed);
}

void turnLeft(int speed = 200) {
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN3, HIGH);
  digitalWrite(RIGHT_IN4, LOW);
  ledcWrite(LEFT_PWM_CH, speed);
  ledcWrite(RIGHT_PWM_CH, speed);
}

void turnRight(int speed = 200) {
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, HIGH);
  ledcWrite(LEFT_PWM_CH, speed);
  ledcWrite(RIGHT_PWM_CH, speed);
}

void stop() {
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, LOW);
  ledcWrite(LEFT_PWM_CH, 0);
  ledcWrite(RIGHT_PWM_CH, 0);
}

// HTML 控制页面
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 遥控车</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            text-align: center; 
            background: #1a1a2e; 
            color: white;
            margin: 0;
            padding: 20px;
        }
        h1 { color: #eee; }
        .controls {
            display: grid;
            grid-template-columns: repeat(3, 80px);
            gap: 10px;
            justify-content: center;
            margin: 30px auto;
        }
        button {
            width: 80px;
            height: 60px;
            font-size: 24px;
            border: none;
            border-radius: 10px;
            background: #16213e;
            color: white;
            cursor: pointer;
            transition: all 0.3s;
        }
        button:hover { background: #0f3460; }
        button:active { background: #e94560; }
        .status {
            margin-top: 20px;
            padding: 10px;
            background: #16213e;
            border-radius: 5px;
        }
    </style>
</head>
<body>
    <h1>ESP32 遥控车</h1>
    <div class="controls">
        <div></div>
        <button ontouchstart="send('forward')" onmousedown="send('forward')" onmouseup="send('stop')">▲</button>
        <div></div>
        <button ontouchstart="send('left')" onmousedown="send('left')" onmouseup="send('stop')">◀</button>
        <button ontouchstart="send('stop')" onmousedown="send('stop')">■</button>
        <button ontouchstart="send('right')" onmousedown="send('right')" onmouseup="send('stop')">▶</button>
        <div></div>
        <button ontouchstart="send('backward')" onmousedown="send('backward')" onmouseup="send('stop')">▼</button>
        <div></div>
    </div>
    <div class="status" id="status">状态: 停止</div>
    
    <script>
        function send(cmd) {
            fetch('/' + cmd)
                .then(r => r.text())
                .then(t => document.getElementById('status').textContent = '状态: ' + t)
                .catch(e => document.getElementById('status').textContent = '错误: ' + e);
        }
    </script>
</body>
</html>
)rawliteral";

// HTTP 请求处理
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleForward() {
  forward();
  server.send(200, "text/plain", "前进");
}

void handleBackward() {
  backward();
  server.send(200, "text/plain", "后退");
}

void handleLeft() {
  turnLeft();
  server.send(200, "text/plain", "左转");
}

void handleRight() {
  turnRight();
  server.send(200, "text/plain", "右转");
}

void handleStop() {
  stop();
  server.send(200, "text/plain", "停止");
}

void setup() {
  Serial.begin(115200);
  
  // 初始化电机
  motorInit();
  
  // 连接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("已连接! IP地址: ");
  Serial.println(WiFi.localIP());
  
  // 设置 HTTP 路由
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);
  
  server.begin();
  Serial.println("HTTP 服务器已启动");
}

void loop() {
  server.handleClient();
}
```

## 代码详解

### 1. 电机控制原理

```cpp
// 前进：左电机正转，右电机正转
void forward(int speed = 200) {
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, HIGH);
  digitalWrite(RIGHT_IN4, LOW);
  ledcWrite(LEFT_PWM_CH, speed);
  ledcWrite(RIGHT_PWM_CH, speed);
}
```

通过控制 IN1-IN4 的电平状态来改变电机转向，通过 PWM 调节速度。

### 2. Web 服务器

```cpp
WebServer server(80);  // 创建 HTTP 服务器，端口 80

server.on("/", handleRoot);        // 首页
server.on("/forward", handleForward);  // 前进
// ...
server.begin();  // 启动服务器
```

ESP32 作为 Web 服务器，提供控制页面和 API 接口。

### 3. HTML 控制界面

内置响应式网页，支持电脑和手机访问，包含方向控制按钮和状态显示。

## 使用方法

### 1. 配置 WiFi

修改代码中的 WiFi 信息：
```cpp
const char* ssid = "你的WiFi名称";
const char* password = "你的WiFi密码";
```

### 2. 上传代码

1. 选择正确的开发板和端口
2. 点击上传
3. 打开串口监视器（115200 波特率）

### 3. 查看 IP 地址

上传成功后，串口会显示：
```
已连接! IP地址: 192.168.1.100
HTTP 服务器已启动
```

### 4. 访问控制页面

在手机或电脑浏览器中输入 ESP32 的 IP 地址，如：
```
http://192.168.1.100
```

### 5. 控制小车

点击页面上的方向按钮即可控制小车移动。

## 扩展功能

### 添加速度调节

```cpp
// 在 HTML 中添加速度滑块
<input type="range" min="0" max="255" value="200" id="speed">

// 在 Arduino 中读取速度
int speed = server.arg("speed").toInt();
forward(speed);
```

### 添加超声波避障

```cpp
#define TRIG_PIN 5
#define ECHO_PIN 18

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;  // 计算距离(cm)
}

// 在 loop 中自动避障
void loop() {
  server.handleClient();
  
  if (getDistance() < 20) {  // 距离小于20cm
    stop();
    delay(500);
    backward();
    delay(500);
    turnRight();
    delay(300);
  }
}
```

### 添加摄像头（ESP32-CAM）

使用 ESP32-CAM 模块可以实现视频流传输，实现"第一人称"遥控体验。

## 常见问题

### 1. 小车不移动
- 检查电池电压是否充足（建议 7.4V 以上）
- 确认电机驱动模块接线正确
- 检查 PWM 引脚是否正确配置

### 2. WiFi 连接失败
- 确认 WiFi 名称和密码正确
- 检查 WiFi 是否为 2.4GHz（ESP32 不支持 5GHz）
- 尝试靠近路由器

### 3. 网页无法访问
- 确认手机和 ESP32 在同一网络
- 检查防火墙设置
- 尝试使用 IP 地址直接访问

### 4. 小车方向相反
- 交换电机两根线的位置
- 或在代码中修改方向控制逻辑

## 安全注意事项

1. **电池安全**：使用合格的锂电池，避免过充过放
2. **电机保护**：避免长时间堵转，防止电机烧毁
3. **速度控制**：初次测试使用较低速度，熟悉后再加速
4. **使用环境**：在平坦、无障碍的地面测试

## 进阶改进

1. **PS4/PS5 手柄控制**：使用蓝牙连接游戏手柄
2. **循迹功能**：添加红外传感器实现自动循迹
3. **APP 控制**：开发专用的手机 APP
4. **GPS 定位**：添加 GPS 模块实现定位功能
5. **4G 远程控制**：使用 4G 模块实现互联网远程控制

## 参考文档

- [ESP32 WiFi 官方文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html)
- [L298N 电机驱动 datasheet](https://www.st.com/resource/en/datasheet/l298.pdf)
- [Arduino WebServer 库](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer)

---

**注意**：使用电机驱动时请注意散热，长时间大电流工作可能需要加装散热片。建议在电池和驱动板之间添加保险丝，防止短路。
