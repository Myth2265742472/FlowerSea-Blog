#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPIFFS.h>
#include <FS.h>

// OLED屏幕配置
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// 创建OLED显示对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Web服务器配置
WebServer server(80);

// WiFi配置
const char* ssid = "66666";
const char* password = "12345678";

// 当前显示的数字
int currentNumber = 0;

// 图片显示模式
bool imageMode = false;

// 函数声明
void handleRoot();
void handleSetNumber();
void handleGetNumber();
void handleUploadImage();
void handleSwitchToNumberMode();
void updateOLEDDisplay(int number);
void displayImage(String binaryData);

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
        
        <div class="input-group">
            <h3>图片上传</h3>
            <input type="file" id="imageInput" accept="image/*" onchange="uploadImage()">
            <button class="btn-primary" onclick="switchToNumberMode()">切换到数字模式</button>
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
        
        function uploadImage() {
            const fileInput = document.getElementById('imageInput');
            const file = fileInput.files[0];
            
            if (file) {
                const reader = new FileReader();
                reader.onload = function(e) {
                    const img = new Image();
                    img.onload = function() {
                        // 将图片转换为128x64像素
                        const canvas = document.createElement('canvas');
                        const ctx = canvas.getContext('2d');
                        canvas.width = 128;
                        canvas.height = 64;
                        
                        // 绘制缩放后的图片
                        ctx.drawImage(img, 0, 0, 128, 64);
                        
                        // 获取像素数据
                        const imageData = ctx.getImageData(0, 0, 128, 64);
                        const pixels = imageData.data;
                        
                        // 转换为黑白数据
                        let binaryData = '';
                        for (let i = 0; i < pixels.length; i += 4) {
                            const r = pixels[i];
                            const g = pixels[i + 1];
                            const b = pixels[i + 2];
                            const gray = (r + g + b) / 3;
                            binaryData += gray > 128 ? '1' : '0';
                        }
                        
                        // 发送图片数据
                        sendImageData(binaryData);
                    };
                    img.src = e.target.result;
                };
                reader.readAsDataURL(file);
            }
        }
        
        function sendImageData(data) {
            fetch('/uploadImage', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({imageData: data})
            })
            .then(response => response.text())
            .then(result => {
                if (result === 'OK') {
                    showStatus('图片上传成功');
                } else {
                    showStatus('图片上传失败', true);
                }
            })
            .catch(error => {
                showStatus('网络错误', true);
            });
        }
        
        function switchToNumberMode() {
            fetch('/switchToNumberMode', {
                method: 'POST'
            })
            .then(response => response.text())
            .then(result => {
                if (result === 'OK') {
                    showStatus('已切换到数字模式');
                }
            })
            .catch(error => {
                showStatus('切换失败', true);
            });
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
    
    // 初始化SPIFFS文件系统
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS初始化失败");
        return;
    }
    
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
    
    // 连接WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    Serial.print("正在连接WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("WiFi已连接，IP地址: ");
    Serial.println(WiFi.localIP());
    
    // 在OLED上显示连接信息
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("WiFi已连接");
    display.println("SSID: " + String(ssid));
    display.println("IP: " + WiFi.localIP().toString());
    display.display();
    
    // 设置Web服务器路由
    server.on("/", handleRoot);
    server.on("/setNumber", handleSetNumber);
    server.on("/getNumber", handleGetNumber);
    server.on("/uploadImage", HTTP_POST, handleUploadImage);
    server.on("/switchToNumberMode", HTTP_POST, handleSwitchToNumberMode);
    
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

void handleUploadImage() {
    if (server.hasArg("plain")) {
        String imageData = server.arg("plain");
        
        // 解析JSON数据
        int startIndex = imageData.indexOf("\"imageData\":\"") + 13;
        int endIndex = imageData.lastIndexOf("\"");
        if (startIndex > 12 && endIndex > startIndex) {
            String binaryData = imageData.substring(startIndex, endIndex);
            
            // 显示图片到OLED
            displayImage(binaryData);
            imageMode = true;
            
            server.send(200, "text/plain", "OK");
            Serial.println("图片已上传并显示");
        } else {
            server.send(400, "text/plain", "无效的图片数据");
        }
    } else {
        server.send(400, "text/plain", "缺少图片数据");
    }
}

void handleSwitchToNumberMode() {
    imageMode = false;
    updateOLEDDisplay(currentNumber);
    server.send(200, "text/plain", "OK");
    Serial.println("已切换到数字模式");
}

void updateOLEDDisplay(int number) {
    if (imageMode) return; // 如果处于图片模式，不更新数字显示
    
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

void displayImage(String binaryData) {
    display.clearDisplay();
    
    // 将二进制字符串转换为像素数据
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int index = y * SCREEN_WIDTH + x;
            if (index < binaryData.length()) {
                if (binaryData.charAt(index) == '1') {
                    display.drawPixel(x, y, SSD1306_WHITE);
                }
            }
        }
    }
    
    display.display();
}
