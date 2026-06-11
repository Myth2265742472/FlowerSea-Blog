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

    // 根据摇杆位置控制电机
    if (y < -30) {
      // 前进
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      analogWrite(EN1, speed1);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(EN2, speed2);
    } else if (y > 30) {
      // 后退
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      analogWrite(EN1, speed1);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(EN2, speed2);
    } else if (x > 30) {
      // 右转
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      analogWrite(EN1, 0);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(EN2, speed2);
    } else if (x < -30) {
      // 左转
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      analogWrite(EN1, speed1);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      analogWrite(EN2, 0);
    } else {
      // 停止
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      analogWrite(EN1, 0);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      analogWrite(EN2, 0);
    }
  }
  server.send(200, "text/plain", "OK");
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
}