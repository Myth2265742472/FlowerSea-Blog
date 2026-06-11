#include <WiFi.h>
#include <WebServer.h>

// 设置 ESP32 作为 AP 的 SSID 和密码
const char* ssid = "666666";
const char* password = "12345678";

// 定义电机控制引脚
const int IN1 = 18;
const int IN2 = 19;
const int IN3 = 21;
const int IN4 = 22;
const int ENA = 5;
const int ENB = 17;

// 创建 Web 服务器对象，监听 80 端口
WebServer server(80);

// HTML 页面内容
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <title>ESP32 遥控车控制</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f0f0f0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }
    .container {
      background-color: #fff;
      padding: 20px 30px;
      border-radius: 8px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      text-align: center;
    }
    h1 {
      color: #333;
    }
    .controls {
      display: flex;
      flex-direction: column;
    }
    .buttons {
      display: flex;
      justify-content: center;
      margin: 10px 0;
    }
    button {
      background-color: #4CAF50;
      color: white;
      padding: 15px 25px;
      margin: 5px;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      font-size: 16px;
    }
    button.stop {
      background-color: #f44336;
    }
    button:hover {
      opacity: 0.9;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 遥控车控制</h1>
    <div class="controls">
      <div class="buttons">
        <button onclick="move('forward')">前进</button>
      </div>
      <div class="buttons">
        <button onclick="move('left')">左转</button>
        <button onclick="move('stop')" class="stop">停止</button>
        <button onclick="move('right')">右转</button>
      </div>
      <div class="buttons">
        <button onclick="move('backward')">后退</button>
      </div>
    </div>
  </div>

  <script>
    function move(direction) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/move?direction=" + direction, true);
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";

// 处理根目录请求
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// 处理移动请求
void handleMove() {
  if(server.hasArg("direction")) {
    String direction = server.arg("direction");
    Serial.println("移动方向: " + direction);
    if(direction == "forward") {
      moveForward();
    }
    else if(direction == "backward") {
      moveBackward();
    }
    else if(direction == "left") {
      turnLeft();
    }
    else if(direction == "right") {
      turnRight();
    }
    else if(direction == "stop") {
      stopMoving();
    }
  }
  handleRoot();
}

// 前进
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 200); // 设置速度
  analogWrite(ENB, 200);
}

// 后退
void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

// 左转
void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

// 右转
void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

// 停止
void stopMoving() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void setup() {
  // 初始化串口通信，波特率设置为115200
  Serial.begin(115200);
  delay(1000);
  
  // 初始化电机控制引脚为输出模式
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  // 停止所有电机
  stopMoving();

  // 打印设置信息
  Serial.println();
  Serial.print("设置 ESP32 为 AP 模式，SSID: ");
  Serial.println(ssid);

  // 设置 ESP32 为 AP 模式
  WiFi.mode(WIFI_AP);
  
  // 配置 AP 参数
  bool result = WiFi.softAP(ssid, password);
  
  if(result){
    Serial.println("AP 启动成功");
    Serial.print("AP IP 地址: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("AP 启动失败");
  }

  // 定义路由
  server.on("/", handleRoot);
  server.on("/move", handleMove);

  // 启动服务器
  server.begin();
  Serial.println("HTTP 服务器已启动");
}

void loop() {
  // 处理客户端请求
  server.handleClient();
}