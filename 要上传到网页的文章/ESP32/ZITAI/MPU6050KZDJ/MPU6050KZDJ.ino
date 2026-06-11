#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <MPU6050.h>

// 创建MPU6050对象
MPU6050 mpu;

// 设置Wi-Fi接入点参数
const char* ssid = "123456789"; // 更改为新的SSID
const char* password = "123456"; // 保持密码不变

// 创建Web服务器对象，监听80端口
WebServer server(80);

// 定义电机控制引脚
// 电机1
const int motor1AIN1 = 17;
const int motor1AIN2 = 18;
const int motor1PWMA = 16;

// 电机2
const int motor2BIN1 = 21;
const int motor2BIN2 = 22;
const int motor2PWMB = 19;

// 电机3
const int motor3AIN1 = 25;
const int motor3AIN2 = 26;
const int motor3PWMA = 23;

// 电机4
const int motor4BIN1 = 32;
const int motor4BIN2 = 33;
const int motor4PWMB = 27;

// PID参数
float Kp = 1.0;
float Ki = 0.5;
float Kd = 0.1;

// 设定点（目标姿态）
float setPoint = 0.0;

// 误差变量
float error;
float previous_error = 0;
float integral = 0;
float derivative;
float output;

// 电机速度变量
int motorSpeed1;
int motorSpeed2;
int motorSpeed3;
int motorSpeed4;

// 辅助变量
long dt = 10; // 时间间隔，单位毫秒
long lastTime;
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

// 处理根路径请求
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// 处理命令请求
void handleCommand() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    Serial.println("Received command: " + cmd);
    if (cmd == "start") {
      // 启动无人机
      // 这里可以添加启动逻辑
    } else if (cmd == "stop") {
      // 停止无人机
      // 这里可以添加停止逻辑
    } else if (cmd == "forward") {
      // 前进
      motorSpeed1 = 200;
      motorSpeed2 = 200;
      motorSpeed3 = 200;
      motorSpeed4 = 200;
      setMotorSpeed(motorSpeed1, motorSpeed2, motorSpeed3, motorSpeed4);
    } else if (cmd == "backward") {
      // 后退
      motorSpeed1 = -200;
      motorSpeed2 = -200;
      motorSpeed3 = -200;
      motorSpeed4 = -200;
      setMotorSpeed(motorSpeed1, motorSpeed2, motorSpeed3, motorSpeed4);
    } else if (cmd == "left") {
      // 左转
      motorSpeed1 = 200;
      motorSpeed2 = 200;
      motorSpeed3 = -200;
      motorSpeed4 = -200;
      setMotorSpeed(motorSpeed1, motorSpeed2, motorSpeed3, motorSpeed4);
    } else if (cmd == "right") {
      // 右转
      motorSpeed1 = -200;
      motorSpeed2 = -200;
      motorSpeed3 = 200;
      motorSpeed4 = 200;
      setMotorSpeed(motorSpeed1, motorSpeed2, motorSpeed3, motorSpeed4);
    }
  }
  server.send(200, "text/plain", "OK");
}

// 设置电机速度的函数
void setMotorSpeed(int m1, int m2, int m3, int m4) {
  // 电机1
  if (m1 > 0) {
    digitalWrite(motor1AIN1, HIGH);
    digitalWrite(motor1AIN2, LOW);
  } else {
    digitalWrite(motor1AIN1, LOW);
    digitalWrite(motor1AIN2, HIGH);
    m1 = -m1;
  }
  analogWrite(motor1PWMA, constrain(m1, 0, 255));

  // 电机2
  if (m2 > 0) {
    digitalWrite(motor2BIN1, HIGH);
    digitalWrite(motor2BIN2, LOW);
  } else {
    digitalWrite(motor2BIN1, LOW);
    digitalWrite(motor2BIN2, HIGH);
    m2 = -m2;
  }
  analogWrite(motor2PWMB, constrain(m2, 0, 255));

  // 电机3
  if (m3 > 0) {
    digitalWrite(motor3AIN1, HIGH);
    digitalWrite(motor3AIN2, LOW);
  } else {
    digitalWrite(motor3AIN1, LOW);
    digitalWrite(motor3AIN2, HIGH);
    m3 = -m3;
  }
  analogWrite(motor3PWMA, constrain(m3, 0, 255));

  // 电机4
  if (m4 > 0) {
    digitalWrite(motor4BIN1, HIGH);
    digitalWrite(motor4BIN2, LOW);
  } else {
    digitalWrite(motor4BIN1, LOW);
    digitalWrite(motor4BIN2, HIGH);
    m4 = -m4;
  }
  analogWrite(motor4PWMB, constrain(m4, 0, 255));
}

// AP状态检测函数
void checkAPStatus() {
  if (WiFi.softAPgetStationNum() > 0) {
    Serial.println("有设备连接到AP");
  } else {
    Serial.println("没有设备连接到AP");
  }
  Serial.print("AP IP地址: ");
  Serial.println(WiFi.softAPIP());
}

// 初始化函数
void setup() {
  // 初始化串口
  Serial.begin(115200);
  // 初始化I2C
  Wire.begin();
  // 初始化MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050连接失败");
    while (1);
  }
  // 初始化电机控制引脚
  pinMode(motor1AIN1, OUTPUT);
  pinMode(motor1AIN2, OUTPUT);
  pinMode(motor1PWMA, OUTPUT);
  pinMode(motor2BIN1, OUTPUT);
  pinMode(motor2BIN2, OUTPUT);
  pinMode(motor2PWMB, OUTPUT);
  pinMode(motor3AIN1, OUTPUT);
  pinMode(motor3AIN2, OUTPUT);
  pinMode(motor3PWMA, OUTPUT);
  pinMode(motor4BIN1, OUTPUT);
  pinMode(motor4BIN2, OUTPUT);
  pinMode(motor4PWMB, OUTPUT);
  
  // 设置初始电机速度为最低
  setMotorSpeed(0, 0, 0, 0);
  delay(2000); // 等待电机初始化

  // 启动Wi-Fi接入点
  bool result = WiFi.softAP(ssid, password);
  if (result) {
    Serial.println("WiFi接入点已启动");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("密码: ");
    Serial.println(password);
    Serial.print("AP IP地址: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("WiFi接入点启动失败");
  }

  // 设置Web服务器的路由
  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.begin();
  Serial.println("Web服务器已启动");
}

// 主循环
void loop() {
  server.handleClient();
  // AP状态检测
  checkAPStatus();
  // 这里可以添加PID控制逻辑
  // 例如：
  // mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  // 计算姿态...
  // 计算PID输出...
  // 设置电机速度...
}