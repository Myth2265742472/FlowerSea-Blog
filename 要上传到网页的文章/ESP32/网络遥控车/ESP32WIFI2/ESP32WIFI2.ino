#include <WiFi.h>
#include <WebServer.h>

// 替换为你的Wi-Fi网络名称和密码
const char* ssid = "12345678";
const char* password = "";

// 创建Web服务器对象，监听80端口
WebServer server(80);

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

// 电机目标速度
int targetSpeed1 = 0;
int targetSpeed2 = 0;

// 电机当前速度
int currentSpeed1 = 0;
int currentSpeed2 = 0;

// 摇杆死区范围
const int deadZone = 10;

// 摇杆最大移动距离（与HTML部分一致）
const int maxDistance = 60;

// 处理根目录请求
void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>ESP32 小车控制</title>
    <style>
        body, html {
            margin: 0;
            padding: 0;
            height: 100%;
            display: flex;
            justify-content: center;
            align-items: center;
            background-color: #f0f0f0;
            font-family: Arial, sans-serif;
        }
        .joystick-container {
            width: 200px;
            height: 200px;
            border-radius: 50%;
            background-color: #ccc;
            position: relative;
            display: flex;
            justify-content: center;
            align-items: center;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        }
        .joystick {
            width: 80px;
            height: 80px;
            border-radius: 50%;
            background-color: #333;
            position: absolute;
            cursor: pointer;
            transition: transform 0.1s ease-out;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
        }
        .position-display {
            margin-top: 20px;
            text-align: center;
            font-size: 18px;
            color: #333;
        }
        .instructions {
            margin-top: 20px;
            text-align: center;
            font-size: 16px;
            color: #666;
        }
    </style>
</head>
<body>
    <div>
        <!-- 摇杆容器 -->
        <div class="joystick-container">
            <div class="joystick"></div>
        </div>
        <!-- 显示摇杆位置 -->
        <div class="position-display">
            摇杆位置: <span id="position">(0, 0)</span>
        </div>
        <!-- 方向键提示 -->
        <div class="instructions">
            使用方向键（↑ ↓ ← →）控制摇杆
        </div>
    </div>

    <script>
        const joystick = document.querySelector('.joystick');
        const container = document.querySelector('.joystick-container');
        const positionDisplay = document.getElementById('position');

        let isDragging = false;
        const maxDistance = 60; // 摇杆最大移动距离
        let currentX = 0;
        let currentY = 0;

        // 鼠标按下事件
        container.addEventListener('mousedown', (e) => {
            isDragging = true;
            moveJoystick(e);
        });

        // 鼠标移动事件
        document.addEventListener('mousemove', (e) => {
            if (isDragging) {
                moveJoystick(e);
            }
        });

        // 鼠标释放事件
        document.addEventListener('mouseup', () => {
            isDragging = false;
            resetJoystick();
        });

        // 移动摇杆的逻辑
        function moveJoystick(e) {
            const rect = container.getBoundingClientRect();
            const centerX = rect.width / 2;
            const centerY = rect.height / 2;

            // 计算鼠标相对于容器中心的位置
            const mouseX = e.clientX - rect.left - centerX;
            const mouseY = e.clientY - rect.top - centerY;

            // 计算距离，确保摇杆不会超出容器
            const distance = Math.sqrt(mouseX * mouseX + mouseY * mouseY);
            if (distance > maxDistance) {
                const angle = Math.atan2(mouseY, mouseX);
                currentX = Math.cos(angle) * maxDistance;
                currentY = Math.sin(angle) * maxDistance;
            } else {
                currentX = mouseX;
                currentY = mouseY;
            }

            updateJoystickPosition();
            sendJoystickData();
        }

        // 更新摇杆位置
        function updateJoystickPosition() {
            joystick.style.transform = `translate(${currentX}px, ${currentY}px)`;
            positionDisplay.textContent = `(${currentX.toFixed(1)}, ${currentY.toFixed(1)})`;
        }

        // 重置摇杆位置
        function resetJoystick() {
            currentX = 0;
            currentY = 0;
            updateJoystickPosition();
            sendJoystickData();
        }

        // 方向键控制
        const keyState = {
            ArrowUp: false,
            ArrowDown: false,
            ArrowLeft: false,
            ArrowRight: false,
        };

        // 键盘按下事件
        document.addEventListener('keydown', (e) => {
            if (keyState.hasOwnProperty(e.key)) {
                keyState[e.key] = true;
                updateJoystickByKeys();
            }
        });

        // 键盘释放事件
        document.addEventListener('keyup', (e) => {
            if (keyState.hasOwnProperty(e.key)) {
                keyState[e.key] = false;
                updateJoystickByKeys();
            }
        });

        // 根据方向键更新摇杆位置
        function updateJoystickByKeys() {
            let x = 0;
            let y = 0;

            if (keyState.ArrowUp) y -= maxDistance;
            if (keyState.ArrowDown) y += maxDistance;
            if (keyState.ArrowLeft) x -= maxDistance;
            if (keyState.ArrowRight) x += maxDistance;

            // 限制对角线移动的距离
            const distance = Math.sqrt(x * x + y * y);
            if (distance > maxDistance) {
                const angle = Math.atan2(y, x);
                x = Math.cos(angle) * maxDistance;
                y = Math.sin(angle) * maxDistance;
            }

            currentX = x;
            currentY = y;
            updateJoystickPosition();
            sendJoystickData();
        }

        // 发送摇杆数据到ESP32
        function sendJoystickData() {
            const xhr = new XMLHttpRequest();
            xhr.open('GET', `/joystick?x=${currentX}&y=${currentY}`, true);
            xhr.send();
        }
    </script>
</body>
</html>
)=====";
  server.send(200, "text/html", html);
}

// 处理摇杆请求
void handleJoystick() {
  if (server.method() == HTTP_GET) {
    float x = server.arg("x").toFloat();
    float y = server.arg("y").toFloat();

    // 摇杆死区处理
    if (abs(x) < deadZone) x = 0;
    if (abs(y) < deadZone) y = 0;

    // 根据摇杆位置设置目标速度
    if (y < -deadZone) {
      // 前进
      targetSpeed1 = map(y, -maxDistance, -deadZone, speed1, 0);
      targetSpeed2 = map(y, -maxDistance, -deadZone, speed2, 0);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else if (y > deadZone) {
      // 后退
      targetSpeed1 = map(y, deadZone, maxDistance, 0, speed1);
      targetSpeed2 = map(y, deadZone, maxDistance, 0, speed2);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
    } else if (x > deadZone) {
      // 右转
      targetSpeed1 = 0;
      targetSpeed2 = map(x, deadZone, maxDistance, 0, speed2);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else if (x < -deadZone) {
      // 左转
      targetSpeed1 = map(x, -maxDistance, -deadZone, speed1, 0);
      targetSpeed2 = 0;
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
    } else {
      // 停止
      targetSpeed1 = 0;
      targetSpeed2 = 0;
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
    }

    // 调试信息
    Serial.print("Target Speed 1: ");
    Serial.println(targetSpeed1);
    Serial.print("Target Speed 2: ");
    Serial.println(targetSpeed2);
  }
  server.send(200, "text/plain", "OK");
}

// 电机速度渐变函数
void smoothMotorControl() {
  // 电机1速度渐变
  if (currentSpeed1 < targetSpeed1) {
    currentSpeed1 += 5;
  } else if (currentSpeed1 > targetSpeed1) {
    currentSpeed1 -= 5;
  }

  // 电机2速度渐变
  if (currentSpeed2 < targetSpeed2) {
    currentSpeed2 += 5;
  } else if (currentSpeed2 > targetSpeed2) {
    currentSpeed2 -= 5;
  }

  // 限制速度范围
  currentSpeed1 = constrain(currentSpeed1, 0, speed1);
  currentSpeed2 = constrain(currentSpeed2, 0, speed2);

  // 设置电机速度
  analogWrite(EN1, currentSpeed1);
  analogWrite(EN2, currentSpeed2);

  // 调试信息
  Serial.print("Current Speed 1: ");
  Serial.println(currentSpeed1);
  Serial.print("Current Speed 2: ");
  Serial.println(currentSpeed2);
}

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
  
  // 连接Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("连接到Wi-Fi中...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wi-Fi连接成功！");
  Serial.print("IP地址: ");
  Serial.println(WiFi.localIP());
  
  // 配置路由
  server.on("/", handleRoot);
  server.on("/joystick", handleJoystick); // 添加摇杆路由
  
  // 启动服务器
  server.begin();
  Serial.println("HTTP服务器已启动");
}

void loop() {
  server.handleClient();
  smoothMotorControl(); // 电机速度渐变控制
  delay(10); // 控制循环频率
}