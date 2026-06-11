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
int speed = 200; // 0-255

// 处理根目录请求
void handleRoot() {
  String message = "<!DOCTYPE html><html lang='zh-CN'>\n\
  <head>\n\
    <meta charset='UTF-8'>\n\
    <title>ESP32 小车控制</title>\n\
    <style>\n\
      body {\n\
        font-family: Arial, sans-serif;\n\
        background-color: #f0f0f0;\n\
        text-align: center;\n\
        padding: 50px;\n\
      }\n\
      h1 {\n\
        color: #333;\n\
      }\n\
      .controls {\n\
        margin-top: 20px;\n\
      }\n\
      button {\n\
        padding: 15px 30px;\n\
        font-size: 16px;\n\
        margin: 10px;\n\
        border: none;\n\
        border-radius: 5px;\n\
        cursor: pointer;\n\
        color: white;\n\
      }\n\
      #forward {\n\
        background-color: #4CAF50;\n\
      }\n\
      #backward {\n\
        background-color: #f44336;\n\
      }\n\
      #left {\n\
        background-color: #008CBA;\n\
      }\n\
      #right {\n\
        background-color: #FFA500;\n\
      }\n\
      #stop {\n\
        background-color: #555;\n\
      }\n\
    </style>\n\
  </head>\n\
  <body>\n\
    <h1>ESP32 小车控制</h1>\n\
    <div class='controls'>\n\
      <button id='forward' onclick=\"location.href='/forward'\">前进</button>\n\
      <button id='backward' onclick=\"location.href='/backward'\">后退</button>\n\
      <button id='left' onclick=\"location.href='/left'\">左转</button>\n\
      <button id='right' onclick=\"location.href='/right'\">右转</button>\n\
      <button id='stop' onclick=\"location.href='/stop'\">停止</button>\n\
    </div>\n\
  </body>\n\
  </html>";
  server.send(200, "text/html", message);
}

// 处理前进请求
void handleForward() {
  // 设置电机1正转
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(EN1, speed);
  
  // 设置电机2正转
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(EN2, speed);
  
  server.sendHeader("Location", "/");
  server.send(303);
}

// 处理后退请求
void handleBackward() {
  // 设置电机1反转
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(EN1, speed);
  
  // 设置电机2反转
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(EN2, speed);
  
  server.sendHeader("Location", "/");
  server.send(303);
}

// 处理左转请求
void handleLeft() {
  // 设置电机1正转
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(EN1, speed);
  
  // 设置电机2停止
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(EN2, 0);
  
  server.sendHeader("Location", "/");
  server.send(303);
}

// 处理右转请求
void handleRight() {
  // 设置电机1停止
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(EN1, 0);
  
  // 设置电机2正转
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(EN2, speed);
  
  server.sendHeader("Location", "/");
  server.send(303);
}

// 处理停止请求
void handleStop() {
  // 停止电机1
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(EN1, 0);
  
  // 停止电机2
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(EN2, 0);
  
  server.sendHeader("Location", "/");
  server.send(303);
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
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);
  
  // 启动服务器
  server.begin();
  Serial.println("HTTP服务器已启动");
}

void loop() {
  server.handleClient();
}