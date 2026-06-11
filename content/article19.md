# ESP32 视觉识别项目开发指南

## 项目简介

本项目是基于 ESP32 的视觉识别系统，利用 ESP32 的强大处理能力和摄像头模块，实现图像采集、处理和识别功能。项目支持多种视觉识别应用，包括物体检测、颜色识别、人脸检测等，适用于智能家居、安防监控、工业检测等场景。

## 功能特性

- ✅ **图像采集**：使用 ESP32-CAM 或外接摄像头模块
- ✅ **实时处理**：在 ESP32 上进行图像预处理
- ✅ **物体检测**：识别特定物体并标记位置
- ✅ **颜色识别**：检测和跟踪特定颜色
- ✅ **人脸检测**：识别人脸并进行简单分析
- ✅ **Web 界面**：通过浏览器查看实时视频和识别结果
- ✅ **数据上传**：将识别结果上传到服务器

## 系统架构

```
┌─────────────────┐     WiFi/网络      ┌─────────────────┐
│   ESP32-CAM     │ ◄──────────────► │   服务器/云端    │
│ + 摄像头模块     │                  │                 │
│ + 视觉算法       │                  │ + 数据存储       │
│                 │                  │ + 深度学习模型   │
└─────────────────┘                  └─────────────────┘
        │
        └────── Web 页面 (实时视频流)
```

## 所需材料

### 硬件
- **ESP32-CAM 开发板** 或 **ESP32-S3 + 摄像头模块**
- **OV2640 摄像头模块**（ESP32-CAM 自带）
- **USB 转 TTL 模块**（用于烧录程序）
- **杜邦线** 若干
- **5V 电源**

### 软件
- **Arduino IDE**
- **ESP32 开发板支持包**
- **ESP32 Camera 库**

## 硬件连接

### ESP32-CAM 引脚定义

```
ESP32-CAM
---------
3.3V    ->  电源 3.3V
GND     ->  地线
U0R     ->  USB-TTL TX (烧录时连接)
U0T     ->  USB-TTL RX (烧录时连接)
GPIO 0  ->  GND (烧录时接地，运行时悬空)
```

### 烧录连接图

```
USB-TTL          ESP32-CAM
-------          ---------
3.3V     ->      3.3V
GND      ->      GND
TX       ->      U0R (GPIO3)
RX       ->      U0T (GPIO1)
```

## 软件准备

### 1. 安装 Arduino IDE

从 [Arduino 官网](https://www.arduino.cc/en/software) 下载并安装。

### 2. 添加 ESP32 支持

1. **文件 → 首选项 → 附加开发板管理器网址**：
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
2. **工具 → 开发板 → 开发板管理器 → 搜索 "ESP32" → 安装**

### 3. 安装 ESP32 Camera 库

Arduino IDE 自带 ESP32 Camera 库，无需额外安装。

## 完整代码示例

### 示例 1：摄像头视频流

```cpp
#include "esp_camera.h"
#include <WiFi.h>

// WiFi 配置
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// 摄像头引脚定义 (ESP32-CAM)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// 创建 Web 服务器
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  // 配置摄像头
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // 根据可用内存选择分辨率
  if(psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // 初始化摄像头
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("摄像头初始化失败，错误码: 0x%x", err);
    return;
  }
  
  // 连接 WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi 已连接");
  Serial.print("IP 地址: ");
  Serial.println(WiFi.localIP());
  
  // 启动服务器
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    String currentLine = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // 发送 HTTP 响应头
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
            client.println();
            
            // 持续发送视频帧
            while (client.connected()) {
              camera_fb_t * fb = esp_camera_fb_get();
              if (!fb) {
                Serial.println("获取帧失败");
                break;
              }
              
              client.println("--frame");
              client.println("Content-Type: image/jpeg");
              client.println("Content-Length: " + String(fb->len));
              client.println();
              client.write(fb->buf, fb->len);
              client.println();
              
              esp_camera_fb_return(fb);
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    
    client.stop();
  }
}
```

### 示例 2：颜色识别

```cpp
#include "esp_camera.h"
#include <WiFi.h>

// 摄像头引脚定义（同上，省略）

// 颜色阈值
struct ColorThreshold {
  uint8_t minH, maxH;
  uint8_t minS, maxS;
  uint8_t minV, maxV;
};

// 红色阈值
ColorThreshold redThreshold = {150, 10, 100, 255, 100, 255};
// 绿色阈值
ColorThreshold greenThreshold = {40, 80, 100, 255, 100, 255};
// 蓝色阈值
ColorThreshold blueThreshold = {100, 140, 100, 255, 100, 255};

// RGB 转 HSV
void rgbToHsv(uint8_t r, uint8_t g, uint8_t b, uint8_t &h, uint8_t &s, uint8_t &v) {
  float rf = r / 255.0;
  float gf = g / 255.0;
  float bf = b / 255.0;
  
  float maxVal = max(rf, max(gf, bf));
  float minVal = min(rf, min(gf, bf));
  float diff = maxVal - minVal;
  
  // 计算 V
  v = maxVal * 255;
  
  // 计算 S
  if (maxVal == 0) {
    s = 0;
  } else {
    s = (diff / maxVal) * 255;
  }
  
  // 计算 H
  if (diff == 0) {
    h = 0;
  } else if (maxVal == rf) {
    h = (60 * ((gf - bf) / diff) + 360) / 2;
  } else if (maxVal == gf) {
    h = (60 * ((bf - rf) / diff) + 120) / 2;
  } else {
    h = (60 * ((rf - gf) / diff) + 240) / 2;
  }
  
  if (h > 180) h -= 180;
}

// 检测颜色
String detectColor(uint8_t r, uint8_t g, uint8_t b) {
  uint8_t h, s, v;
  rgbToHsv(r, g, b, h, s, v);
  
  // 检查红色
  if ((h >= 150 || h <= 10) && s >= 100 && v >= 100) {
    return "红色";
  }
  // 检查绿色
  if (h >= 40 && h <= 80 && s >= 100 && v >= 100) {
    return "绿色";
  }
  // 检查蓝色
  if (h >= 100 && h <= 140 && s >= 100 && v >= 100) {
    return "蓝色";
  }
  
  return "未知";
}

// 在图像中心检测颜色
String detectCenterColor(camera_fb_t *fb) {
  if (fb->format != PIXFORMAT_RGB565) {
    return "不支持的颜色格式";
  }
  
  // 获取图像中心点
  int centerX = fb->width / 2;
  int centerY = fb->height / 2;
  int pixelIndex = centerY * fb->width + centerX;
  
  // 获取像素值
  uint16_t pixel = ((uint16_t*)fb->buf)[pixelIndex];
  uint8_t r = ((pixel >> 11) & 0x1F) << 3;
  uint8_t g = ((pixel >> 5) & 0x3F) << 2;
  uint8_t b = (pixel & 0x1F) << 3;
  
  return detectColor(r, g, b);
}

void setup() {
  // 初始化摄像头（同上，省略）
  
  Serial.begin(115200);
  Serial.println("颜色识别系统启动");
}

void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("获取帧失败");
    delay(1000);
    return;
  }
  
  // 检测中心颜色
  String color = detectCenterColor(fb);
  Serial.println("检测到颜色: " + color);
  
  esp_camera_fb_return(fb);
  delay(500);
}
```

### 示例 3：运动检测

```cpp
#include "esp_camera.h"
#include <WiFi.h>

// 摄像头引脚定义（同上，省略）

// 运动检测参数
#define MOTION_THRESHOLD 30    // 像素差异阈值
#define MOTION_PIXELS 100      // 触发运动检测的最小像素数
#define BUFFER_SIZE 10         // 帧缓冲区大小

camera_fb_t* frameBuffer[BUFFER_SIZE];
int bufferIndex = 0;
bool motionDetected = false;

// 计算两帧之间的差异
int calculateFrameDiff(camera_fb_t* frame1, camera_fb_t* frame2) {
  if (frame1->len != frame2->len) return 0;
  
  int diffPixels = 0;
  int sampleStep = 10;  // 采样步长，提高性能
  
  for (int i = 0; i < frame1->len; i += sampleStep) {
    int diff = abs(frame1->buf[i] - frame2->buf[i]);
    if (diff > MOTION_THRESHOLD) {
      diffPixels++;
    }
  }
  
  return diffPixels;
}

// 检测运动
bool detectMotion() {
  if (bufferIndex < 2) return false;
  
  int prevIndex = (bufferIndex - 1 + BUFFER_SIZE) % BUFFER_SIZE;
  int currIndex = bufferIndex % BUFFER_SIZE;
  
  if (frameBuffer[prevIndex] == NULL || frameBuffer[currIndex] == NULL) {
    return false;
  }
  
  int diffPixels = calculateFrameDiff(frameBuffer[prevIndex], frameBuffer[currIndex]);
  
  return diffPixels > MOTION_PIXELS;
}

void setup() {
  // 初始化摄像头（同上，省略）
  
  Serial.begin(115200);
  Serial.println("运动检测系统启动");
  
  // 初始化帧缓冲区
  for (int i = 0; i < BUFFER_SIZE; i++) {
    frameBuffer[i] = NULL;
  }
}

void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("获取帧失败");
    delay(1000);
    return;
  }
  
  // 释放旧的帧
  if (frameBuffer[bufferIndex % BUFFER_SIZE] != NULL) {
    esp_camera_fb_return(frameBuffer[bufferIndex % BUFFER_SIZE]);
  }
  
  // 保存当前帧
  frameBuffer[bufferIndex % BUFFER_SIZE] = fb;
  bufferIndex++;
  
  // 检测运动
  if (detectMotion()) {
    if (!motionDetected) {
      Serial.println("检测到运动!");
      motionDetected = true;
      
      // 可以在这里添加报警逻辑
      // 例如：发送通知、保存图片等
    }
  } else {
    motionDetected = false;
  }
  
  delay(100);
}
```

## 使用方法

### 1. 硬件连接

1. 按照连接图连接 ESP32-CAM 和 USB-TTL 模块
2. 将 GPIO0 接地（进入烧录模式）
3. 连接 5V 电源

### 2. 烧录程序

1. 在 Arduino IDE 中选择 **工具 → 开发板 → ESP32 Arduino → AI Thinker ESP32-CAM**
2. 选择正确的串口
3. 点击上传按钮
4. 等待烧录完成

### 3. 查看视频流

1. 断开 GPIO0 与 GND 的连接（正常运行模式）
2. 按复位按钮
3. 打开串口监视器，查看 IP 地址
4. 在浏览器中访问：`http://ESP32_IP地址`

## 扩展功能

### 1. 人脸识别

使用 ESP-WHO 框架实现人脸识别：

```cpp
#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"

// 初始化人脸识别
mtmn_config_t mtmn_config = {0};
face_id_init(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);

// 检测人脸
dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);
```

### 2. 物体检测

使用 TensorFlow Lite for Microcontrollers：

```cpp
#include "tensorflow/lite/micro/micro_interpreter.h"

// 加载模型
const tflite::Model* model = tflite::GetModel(g_model);

// 运行推理
interpreter->Invoke();
```

### 3. 云端识别

将图像上传到云端进行识别：

```cpp
// 发送图像到服务器
HTTPClient http;
http.begin("http://your-server.com/api/recognize");
http.addHeader("Content-Type", "image/jpeg");
int httpResponseCode = http.POST(fb->buf, fb->len);
```

## 常见问题

### 1. 摄像头初始化失败
- 检查摄像头模块连接是否正确
- 确认引脚定义与开发板匹配
- 检查电源是否稳定（建议使用独立电源）

### 2. 图像质量差
- 调整 `jpeg_quality` 参数（值越小质量越好）
- 选择合适的分辨率
- 检查镜头是否清洁

### 3. WiFi 连接失败
- 确认 WiFi 名称和密码正确
- 检查 WiFi 是否为 2.4GHz
- 尝试靠近路由器

### 4. 内存不足
- 降低图像分辨率
- 减少帧缓冲区大小
- 使用 PSRAM（如果可用）

## 性能优化

### 1. 降低分辨率

```cpp
config.frame_size = FRAMESIZE_VGA;  // 640x480
// 或
config.frame_size = FRAMESIZE_QVGA; // 320x240
```

### 2. 调整 JPEG 质量

```cpp
config.jpeg_quality = 15;  // 0-63，值越小质量越好，文件越大
```

### 3. 使用 PSRAM

```cpp
if(psramFound()) {
  config.fb_count = 2;  // 使用双缓冲
}
```

## 参考文档

- [ESP32 Camera 驱动文档](https://github.com/espressif/esp32-camera)
- [ESP-WHO 人脸识别框架](https://github.com/espressif/esp-who)
- [TensorFlow Lite for Microcontrollers (GitHub)](https://github.com/tensorflow/tflite-micro)

---

**注意**：ESP32-CAM 的 GPIO0 在烧录时需要接地，正常运行时需要悬空或接高电平。使用外部电源时，请确保电源稳定，避免电压波动导致摄像头工作异常。
