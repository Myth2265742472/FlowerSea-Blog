#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED屏幕配置
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// 创建OLED显示对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Web服务器配置
WebServer server(80);

// AP模式配置
const char* ssid = "ESP32_OLED_Control";
const char* password = "12345678";

// 当前显示的数字
int currentNumber = 0;

// HTML页面内容
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 OLED 数字控制</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            background-color: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 30px;
        }
        .number-display {
            font-size: 48px;
            font-weight: bold;
            text-align: center;
            color: #2196F3;
            margin: 20px 0;
            padding: 20px;
            background-color: #f5f5f5;
            border-radius: 5px;
        }
        .controls {
            display: flex;
            justify-content: center;
            gap: 10px;
            margin: 20px 0;
        }
        button {
            padding: 15px 25px;
            font-size: 18px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        .btn-primary {
            background-color: #2196F3;
            color: white;
        }
        .btn-primary:hover {
            background-color: #1976D2;
        }
        .btn-secondary {
            background-color: #757575;
            color: white;
        }
        .btn-secondary:hover {
            background-color: #616161;
        }
        .btn-danger {
            background-color: #f44336;
            color: white;
        }
        .btn-danger:hover {
            background-color: #d32f2f;
        }
        .input-group {
            margin: 20px 0;
            text-align: center;
        }
        input[type="number"] {
            padding: 10px;
            font-size: 16px;
            border: 1px solid #ddd;
            border-radius: 5px;
            width: 100px;
            text-align: center;
        }
        .status {
            text-align: center;
            margin-top: 20px;
            padding: 10px;
            border-radius: 5px;
        }
        .status.success {
            background-color: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .status.error {
            background-color: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 OLED 数字控制</h1>
        
        <div class="number-display" id="currentNumber">0</div>
        
        <div class="controls">
            <button class="btn-primary" onclick="changeNumber(-10)">-10</button>
            <button class="btn-primary" onclick="changeNumber(-1)">-1</button>
            <button class="btn-secondary" onclick="setNumber(0)">重置</button>
            <button class="btn-primary" onclick="changeNumber(1)">+1</button>
            <button class="btn-primary" onclick="changeNumber(10)">+10</button>
        </div>
        
        <div class="input-group">
            <input type="number" id="numberInput" placeholder="输入数字" min="-999" max="999">
            <button class="btn-primary" onclick="setCustomNumber()">设置</button>
        </div>
        
        <div class="controls">
            <button class="btn-danger" onclick="clearDisplay()">清空显示</button>
        </div>
        
        <div id="status"></div>
    </div>

    <script>
        let currentNumber = 0;
        
        function updateDisplay() {
            document.getElementById('currentNumber').textContent = currentNumber;
        }
        
        function showStatus(message, isError = false) {
            const statusDiv = document.getElementById('status');
            statusDiv.textContent = message;
            statusDiv.className = 'status ' + (isError ? 'error' : 'success');
            setTimeout(() => {
                statusDiv.textContent = '';
                statusDiv.className = 'status';
            }, 3000);
        }
        
        function changeNumber(delta) {
            currentNumber += delta;
            updateDisplay();
            sendNumber(currentNumber);
        }
        
        function setNumber(number) {
            currentNumber = number;
            updateDisplay();
            sendNumber(currentNumber);
        }
        
        function setCustomNumber() {
            const input = document.getElementById('numberInput');
            const number = parseInt(input.value);
            if (!isNaN(number)) {
                currentNumber = number;
                updateDisplay();
                sendNumber(currentNumber);
                input.value = '';
            } else {
                showStatus('请输入有效数字', true);
            }
        }
        
        function clearDisplay() {
            currentNumber = 0;
            updateDisplay();
            sendNumber(-999); // 特殊值表示清空
        }
        
        function sendNumber(number) {
            fetch('/setNumber?value=' + number)
                .then(response => response.text())
                .then(data => {
                    if (data === 'OK') {
                        showStatus('数字已更新: ' + number);
                    } else {
                        showStatus('更新失败', true);
                    }
                })
                .catch(error => {
                    showStatus('网络错误', true);
                });
        }
        
        // 页面加载时获取当前数字
        window.onload = function() {
            fetch('/getNumber')
                .then(response => response.text())
                .then(data => {
                    currentNumber = parseInt(data);
                    updateDisplay();
                })
                .catch(error => {
                    console.log('获取当前数字失败');
                });
        };
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    
    // 初始化OLED屏幕
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306分配失败"));
        for(;;);
    }
    
    // 清空显示缓冲区
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32 OLED");
    display.println("Web Control");
    display.display();
    
    // 设置AP模式
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP地址: ");
    Serial.println(IP);
    
    // 在OLED上显示IP地址
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("AP模式已启动");
    display.println("SSID: " + String(ssid));
    display.println("IP: " + IP.toString());
    display.println("密码: " + String(password));
    display.display();
    
    // 设置Web服务器路由
    server.on("/", handleRoot);
    server.on("/setNumber", handleSetNumber);
    server.on("/getNumber", handleGetNumber);
    
    // 启动Web服务器
    server.begin();
    Serial.println("Web服务器已启动");
}

void loop() {
    server.handleClient();
}

void handleRoot() {
    server.send(200, "text/html", htmlPage);
}

void handleSetNumber() {
    if (server.hasArg("value")) {
        int value = server.arg("value").toInt();
        
        if (value == -999) {
            // 清空显示
            display.clearDisplay();
            display.display();
            currentNumber = 0;
        } else {
            currentNumber = value;
            updateOLEDDisplay(currentNumber);
        }
        
        server.send(200, "text/plain", "OK");
        Serial.println("数字已更新: " + String(currentNumber));
    } else {
        server.send(400, "text/plain", "缺少参数");
    }
}

void handleGetNumber() {
    server.send(200, "text/plain", String(currentNumber));
}

void updateOLEDDisplay(int number) {
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    
    // 计算文本居中位置
    String numStr = String(number);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(numStr, 0, 0, &x1, &y1, &w, &h);
    
    int x = (SCREEN_WIDTH - w) / 2;
    int y = (SCREEN_HEIGHT - h) / 2;
    
    display.setCursor(x, y);
    display.println(numStr);
    display.display();
}