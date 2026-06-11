#include <WiFi.h>
#include <WebServer.h>

// 设置AP模式的Wi-Fi名称和密码
const char* ssid = "ESP32_Car";       // 热点名称
const char* password = "12345678";    // 热点密码

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

// 摇杆死区范围
const int deadZone = 10;

// 摇杆最大移动距离
const int maxDistance = 100;

// 处理根目录请求
void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 小车控制</title>
    <style>
        body, html {
            margin: 0;
            padding: 0;
            height: 100%;
            display: flex;
            flex-direction: column;
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
            margin-bottom: 20px;
        }
        .joystick {
            width: 80px;
            height: 80px;
            border-radius: 50%;
            background-color: #4CAF50;
            position: absolute;
            cursor: pointer;
            transition: transform 0.1s ease-out;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
        }
        .controls {
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .button {
            width: 100px;
            height: 100px;
            margin: 10px;
            border: none;
            border-radius: 10px;
            background-color: #4CAF50;
            color: white;
            font-size: 24px;
            cursor: pointer;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        }
        .button:active {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <div class="joystick-container">
        <div class="joystick"></div>
    </div>
    <div class="controls">
        <button class="button" id="forward">前进</button>
        <div>
            <button class="button" id="left">左转</button>
            <button class="button" id="right">右转</button>
        </div>
        <button class="button" id="backward">后退</button>
        <button class="button" id="stop">停止</button>
    </div>
    <script>
        // 摇杆逻辑
        const joystick = document.querySelector('.joystick');
        const container = document.querySelector('.joystick-container');
        let isDragging = false;
        const maxDistance = 100;
        let currentX = 0;
        let currentY = 0;

        function startDrag(e) {
            isDragging = true;
            moveJoystick(e);
        }

        function moveDrag(e) {
            if (isDragging) {
                moveJoystick(e);
            }
        }

        function endDrag() {
            isDragging = false;
            resetJoystick();
        }

        function moveJoystick(e) {
            const rect = container.getBoundingClientRect();
            const centerX = rect.width / 2;
            const centerY = rect.height / 2;

            let clientX, clientY;
            if (e.touches) {
                clientX = e.touches[0].clientX;
                clientY = e.touches[0].clientY;
            } else {
                clientX = e.clientX;
                clientY = e.clientY;
            }

            const touchX = clientX - rect.left - centerX;
            const touchY = clientY - rect.top - centerY;

            const distance = Math.sqrt(touchX * touchX + touchY * touchY);
            if (distance > maxDistance) {
                const angle = Math.atan2(touchY, touchX);
                currentX = Math.cos(angle) * maxDistance;
                currentY = Math.sin(angle) * maxDistance;
            } else {
                currentX = touchX;
                currentY = touchY;
            }

            updateJoystickPosition();
            sendJoystickData();
        }

        function updateJoystickPosition() {
            joystick.style.transform = `translate(${currentX}px, ${currentY}px)`;
        }

        function resetJoystick() {
            currentX = 0;
            currentY = 0;
            updateJoystickPosition();
            sendJoystickData();
        }

        function sendJoystickData() {
            const xhr = new XMLHttpRequest();
            xhr.open('GET', `/joystick?x=${currentX}&y=${currentY}`, true);
            xhr.send();
        }

        container.addEventListener('mousedown', startDrag);
        document.addEventListener('mousemove', moveDrag);
        document.addEventListener('mouseup', endDrag);
        container.addEventListener('touchstart', startDrag);
        container.addEventListener('touchmove', moveDrag);
        container.addEventListener('touchend', endDrag);

        // 按键逻辑
        const forwardBtn = document.getElementById('forward');
        const leftBtn = document.getElementById('left');
        const rightBtn = document.getElementById('right');
        const backwardBtn = document.getElementById('backward');
        const stopBtn = document.getElementById('stop');

        function sendCommand(command) {
            const xhr = new XMLHttpRequest();
            xhr.open('GET', `/control?cmd=${command}`, true);
            xhr.send();
        }

        forwardBtn.addEventListener('mousedown', () => sendCommand('forward'));
        forwardBtn.addEventListener('mouseup', () => sendCommand('stop'));
        forwardBtn.addEventListener('touchstart', () => sendCommand('forward'));
        forwardBtn.addEventListener('touchend', () => sendCommand('stop'));

        leftBtn.addEventListener('mousedown', () => sendCommand('left'));
        leftBtn.addEventListener('mouseup', () => sendCommand('stop'));
        leftBtn.addEventListener('touchstart', () => sendCommand('left'));
        leftBtn.addEventListener('touchend', () => sendCommand('stop'));

        rightBtn.addEventListener('mousedown', () => sendCommand('right'));
        rightBtn.addEventListener('mouseup', () => sendCommand('stop'));
        rightBtn.addEventListener('touchstart', () => sendCommand('right'));
        rightBtn.addEventListener('touchend', () => sendCommand('stop'));

        backwardBtn.addEventListener('mousedown', () => sendCommand('backward'));
        backwardBtn.addEventListener('mouseup', () => sendCommand('stop'));
        backwardBtn.addEventListener('touchstart', () => sendCommand('backward'));
        backwardBtn.addEventListener('touchend', () => sendCommand('stop'));

        stopBtn.addEventListener('click', () => sendCommand('stop'));
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
    Serial.print("Received joystick data: x=");
    Serial.print(x);
    Serial.print(", y=");
    Serial.println(y);

    // 摇杆死区处理
    if (abs(x) < deadZone) x = 0;
    if (abs(y) < deadZone) y = 0;

    // 根据摇杆位置设置目标速度
    if (y < -deadZone) {
      // 前进
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(EN1, speed1);
      analogWrite(EN2, speed2);
    } else if (y > deadZone) {
      // 后退
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(EN1, speed1);
      analogWrite(EN2, speed2);
    } else if (x > deadZone) {
      // 左转
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(EN1, speed1);
      analogWrite(EN2, speed2);
    } else if (x < -deadZone) {
      // 右转
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(EN1, speed1);
      analogWrite(EN2, speed2);
    } else {
      // 停止
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      analogWrite(EN1, 0);
      analogWrite(EN2, 0);
    }
  }
  server.send(200, "text/plain", "OK");
}

// 处理控制请求
void handleControl() {
  if (server.method() == HTTP_GET) {
    String command = server.arg("cmd");
    Serial.print("Received control command: ");
    Serial.println(command);

    if (command == "forward") {
      // 前进
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(EN1, speed1);
      analogWrite(EN2, speed2);
    } else if (command == "backward") {
      // 后退
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(EN1, speed1);
      analogWrite(EN2, speed2);
    } else if (command == "left") {
      // 左转
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(EN1, speed1);
      analogWrite(EN2, speed2);
    } else if (command == "right") {
      // 右转
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(EN1, speed1);
      analogWrite(EN2, speed2);
    } else if (command == "stop") {
      // 停止
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      analogWrite(EN1, 0);
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
  
  // 设置AP模式
  WiFi.softAP(ssid, password);
  Serial.println("AP模式已启动");
  Serial.print("热点名称: ");
  Serial.println(ssid);
  Serial.print("热点密码: ");
  Serial.println(password);
  Serial.print("IP地址: ");
  Serial.println(WiFi.softAPIP());
  
  // 配置路由
  server.on("/", handleRoot);
  server.on("/joystick", handleJoystick);
  server.on("/control", handleControl);
  
  // 启动服务器
  server.begin();
  Serial.println("HTTP服务器已启动");
}

void loop() {
  server.handleClient();
  delay(10); // 控制循环频率
}