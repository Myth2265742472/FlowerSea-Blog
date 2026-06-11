#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi配置
const char* ssid = "Myth";
const char* password = "12345678";

// 创建Web服务器
WebServer server(80);

// 全局变量
String lastTemperature = "--.-";
String lastUpdateTime = "--:--:--";
bool dataReceived = false;
String rawData = "";

// HTML页面内容
const char* htmlPage = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>温度监控系统</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            margin: 0;
            padding: 20px;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 15px 35px rgba(0,0,0,0.1);
            padding: 40px;
            text-align: center;
            max-width: 500px;
            width: 100%;
        }
        .title {
            color: #333;
            margin-bottom: 30px;
            font-size: 28px;
            font-weight: bold;
        }
        .temperature-display {
            font-size: 72px;
            font-weight: bold;
            color: #e74c3c;
            margin: 20px 0;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.1);
        }
        .temperature-unit {
            font-size: 24px;
            color: #666;
            margin-left: 10px;
        }
        .status {
            margin: 20px 0;
            padding: 15px;
            border-radius: 10px;
            font-size: 16px;
        }
        .status.connected {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .status.disconnected {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .update-time {
            color: #666;
            font-size: 14px;
            margin-top: 20px;
        }
        .raw-data {
            background: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 8px;
            padding: 15px;
            margin: 20px 0;
            font-family: monospace;
            font-size: 12px;
            color: #495057;
            text-align: left;
        }
        .connection-info {
            background: #e7f3ff;
            border: 1px solid #b3d7ff;
            border-radius: 8px;
            padding: 15px;
            margin: 20px 0;
            font-size: 14px;
            color: #004085;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1 class="title">🌡️ 温度监控系统</h1>
        
        <div class="temperature-display">
            <span id="temperature">--.-</span>
            <span class="temperature-unit">°C</span>
        </div>
        
        <div id="status" class="status disconnected">
            <span id="statusText">未连接设备</span>
        </div>
        
        <div class="connection-info">
            <strong>WiFi:</strong> Myth<br>
            <strong>设备状态:</strong> <span id="connectionStatus">未连接</span>
        </div>
        
        <div class="update-time">
            最后更新: <span id="updateTime">--:--:--</span>
        </div>
        
        <div class="raw-data">
            <strong>原始数据:</strong><br>
            <span id="rawData">等待数据...</span>
        </div>
    </div>

    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temperature').textContent = data.temperature;
                    document.getElementById('updateTime').textContent = data.updateTime;
                    document.getElementById('rawData').textContent = data.rawData;
                    
                    const status = document.getElementById('status');
                    const statusText = document.getElementById('statusText');
                    const connectionStatus = document.getElementById('connectionStatus');
                    
                    if (data.connected) {
                        status.className = 'status connected';
                        statusText.textContent = '✅ 设备已连接';
                        connectionStatus.textContent = '在线';
                    } else {
                        status.className = 'status disconnected';
                        statusText.textContent = '❌ 设备未连接';
                        connectionStatus.textContent = '离线';
                    }
                })
                .catch(error => {
                    console.error('获取数据失败:', error);
                    document.getElementById('status').className = 'status disconnected';
                    document.getElementById('statusText').textContent = '❌ 连接错误';
                    document.getElementById('connectionStatus').textContent = '错误';
                });
        }
        
        // 页面加载完成后立即更新数据
        updateData();
        
        // 每3秒更新一次数据
        setInterval(updateData, 3000);
    </script>
</body>
</html>
)";

void setup() {
    Serial.begin(115200);
    Serial2.begin(9600);  // 与单片机通信的串口，波特率9600
    
    Serial.println("正在启动ESP32温度监控系统...");
    
    // 创建WiFi接入点
    WiFi.softAP(ssid, password);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP地址: ");
    Serial.println(IP);
    
    // 设置Web服务器路由
    server.on("/", handleRoot);
    server.on("/data", handleData);
    
    // 启动Web服务器
    server.begin();
    Serial.println("Web服务器已启动");
    Serial.println("请连接到WiFi: Myth");
    Serial.println("密码: 12345678");
}

void loop() {
    // 处理Web服务器请求
    server.handleClient();
    
    // 读取串口数据
    while (Serial2.available()) {
        char c = Serial2.read();
        rawData += c;
        
        // 检查是否收到完整的数据包
        if (c == '#') {
            parseSerialData(rawData);
            rawData = "";  // 清空缓冲区
        }
        
        // 防止缓冲区过长
        if (rawData.length() > 100) {
            rawData = "";
        }
    }
}

// 处理根页面请求
void handleRoot() {
    server.send(200, "text/html", htmlPage);
}

// 处理数据请求
void handleData() {
    StaticJsonDocument<200> doc;
    
    // 添加温度数据
    doc["temperature"] = lastTemperature;
    doc["updateTime"] = lastUpdateTime;
    doc["connected"] = dataReceived;
    doc["rawData"] = rawData;
    
    // 转换为JSON字符串
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

// 解析串口数据（直接显示原始温度）
void parseSerialData(String data) {
    Serial.println("收到数据: " + data);
    
    // 检查数据格式: $51,TMS[温度值]#
    if (data.startsWith("$51,TMS") && data.endsWith("#")) {
        // 提取温度值部分
        int tempStart = data.indexOf("TMS") + 3;
        int tempEnd = data.lastIndexOf("#");
        
        if (tempStart < tempEnd) {
            String tempStr = data.substring(tempStart, tempEnd);
            int tempValue = tempStr.toInt();
            
            // 直接显示原始温度值
            if (tempValue >= 0 && tempValue <= 500) {
                float displayTemp = tempValue / 1;
                lastTemperature = String(displayTemp, 1);
                lastUpdateTime = getCurrentTime();
                dataReceived = true;
                
                Serial.println("解析成功 - 温度: " + lastTemperature + "°C");
            } else {
                Serial.println("温度值超出范围: " + String(tempValue));
            }
        }
    } else {
        Serial.println("数据格式不正确");
        dataReceived = false;
    }
}

// 获取当前时间字符串
String getCurrentTime() {
    unsigned long now = millis();
    unsigned long seconds = now / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    seconds = seconds % 60;
    minutes = minutes % 60;
    hours = hours % 24;
    
    String timeStr = "";
    if (hours < 10) timeStr += "0";
    timeStr += String(hours) + ":";
    if (minutes < 10) timeStr += "0";
    timeStr += String(minutes) + ":";
    if (seconds < 10) timeStr += "0";
    timeStr += String(seconds);
    
    return timeStr;
}