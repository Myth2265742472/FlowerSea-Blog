#include <WiFi.h>
#include <WebServer.h>

// Motor control pins
#define MOTOR1_PWM 12
#define MOTOR1_DIR 13
#define MOTOR2_PWM 14
#define MOTOR2_DIR 15

// PWM settings
const int freq = 1000;
const int resolution = 8;

// WiFi settings
const char* ssid = "ESP32_Car";
const char* password = "12345678";

WebServer server(80);

void setup() {
  // Configure motor control pins
  pinMode(MOTOR1_PWM, OUTPUT);
  pinMode(MOTOR1_DIR, OUTPUT);
  pinMode(MOTOR2_PWM, OUTPUT);
  pinMode(MOTOR2_DIR, OUTPUT);
  
  // Configure PWM
  ledcSetup(0, freq, resolution);
  ledcSetup(1, freq, resolution);
  ledcAttachPin(MOTOR1_PWM, 0);
  ledcAttachPin(MOTOR2_PWM, 1);

  // Start WiFi AP
  WiFi.softAP(ssid, password);
  
  // Start web server
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = R"=====(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP32 Car Control</title>
    <style>
      .control {
        width: 100px;
        height: 100px;
        margin: 10px;
        font-size: 24px;
      }
    </style>
  </head>
  <body>
    <h1>ESP32 Car Control</h1>
    <button class="control" onclick="sendCommand('forward')">Forward</button><br>
    <button class="control" onclick="sendCommand('left')">Left</button>
    <button class="control" onclick="sendCommand('stop')">Stop</button>
    <button class="control" onclick="sendCommand('right')">Right</button><br>
    <button class="control" onclick="sendCommand('backward')">Backward</button>
    
    <script>
      function sendCommand(cmd) {
        fetch('/control?cmd=' + cmd)
          .then(response => response.text())
          .then(data => console.log(data));
      }
    </script>
  </body>
  </html>
  )=====";
  
  server.send(200, "text/html", html);
}

void handleControl() {
  String command = server.arg("cmd");
  
  if (command == "forward") {
    moveForward();
  } else if (command == "backward") {
    moveBackward();
  } else if (command == "left") {
    turnLeft();
  } else if (command == "right") {
    turnRight();
  } else if (command == "stop") {
    stopMotors();
  }
  
  server.send(200, "text/plain", "Command received: " + command);
}

void moveForward() {
  digitalWrite(MOTOR1_DIR, HIGH);
  digitalWrite(MOTOR2_DIR, HIGH);
  ledcWrite(0, 255);
  ledcWrite(1, 255);
}

void moveBackward() {
  digitalWrite(MOTOR1_DIR, LOW);
  digitalWrite(MOTOR2_DIR, LOW);
  ledcWrite(0, 255);
  ledcWrite(1, 255);
}

void turnLeft() {
  digitalWrite(MOTOR1_DIR, HIGH);
  digitalWrite(MOTOR2_DIR, LOW);
  ledcWrite(0, 255);
  ledcWrite(1, 255);
}

void turnRight() {
  digitalWrite(MOTOR1_DIR, LOW);
  digitalWrite(MOTOR2_DIR, HIGH);
  ledcWrite(0, 255);
  ledcWrite(1, 255);
}

void stopMotors() {
  ledcWrite(0, 0);
  ledcWrite(1, 0);
}
