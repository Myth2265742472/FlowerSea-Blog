# ESP32 I2S MEMS 麦克风音频采集实战

## 项目简介

本项目展示了如何使用 ESP32 开发板连接 I2S MEMS 麦克风（如 INMP441），实现高质量的音频数据采集。通过 I2S 接口，ESP32 可以读取数字音频信号，并进行实时处理，如音量检测、频谱分析、噪音监测等。

## 功能特性

- ✅ **I2S 数字音频采集**：使用 I2S 接口读取 MEMS 麦克风数据
- ✅ **实时音量检测**：计算音频信号的 RMS 值，显示当前音量
- ✅ **串口绘图器输出**：支持 Arduino 串口绘图器可视化音频波形
- ✅ **VU 表演示**：模拟音量表效果，直观显示音量变化
- ✅ **噪音级别监测**：检测环境噪音水平，可用于噪声监测应用

## 所需材料

### 硬件
- **ESP32 开发板** (如 ESP32 DevKit V1)
- **INMP441 I2S MEMS 麦克风模块**
- **杜邦线** 若干

### 软件
- **Arduino IDE**
- **ESP32 开发板支持包**

## 硬件连接

### INMP441 与 ESP32 连接

```
INMP441       ESP32
-------       -----
VDD     ->    3.3V
GND     ->    GND
SCK     ->    GPIO 14 (BCLK)
WS      ->    GPIO 15 (LRCK)
SD      ->    GPIO 32 (DATA)
L/R     ->    GND (左声道模式)
```

### 连接说明

| INMP441 引脚 | ESP32 引脚 | 功能说明 |
|-------------|-----------|---------|
| VDD | 3.3V | 电源供电（3.3V） |
| GND | GND | 地线 |
| SCK | GPIO 14 | I2S 位时钟 (BCLK) |
| WS | GPIO 15 | I2S 字选择 (LRCK) |
| SD | GPIO 32 | I2S 数据输入 |
| L/R | GND | 声道选择，接地为左声道 |

## 软件准备

### 1. 安装 Arduino IDE

从 [Arduino 官网](https://www.arduino.cc/en/software) 下载并安装。

### 2. 添加 ESP32 支持

1. **文件 → 首选项 → 附加开发板管理器网址**：
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
2. **工具 → 开发板 → 开发板管理器 → 搜索 "ESP32" → 安装**

### 3. 选择开发板

**工具 → 开发板 → ESP32 Arduino → ESP32 Dev Module**

## 完整代码示例

### 示例 1：串口绘图器输出

```cpp
#include <driver/i2s.h>

// I2S 配置
#define I2S_WS  15
#define I2S_SD  32
#define I2S_SCK 14

// I2S 端口
#define I2S_PORT I2S_NUM_0

// 采样配置
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024

void setup() {
  Serial.begin(115200);
  
  // 配置 I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  // 配置 I2S 引脚
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  
  // 安装并启动 I2S 驱动
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  
  Serial.println("I2S 麦克风初始化完成");
}

void loop() {
  int32_t samples[BUFFER_SIZE];
  size_t bytes_read;
  
  // 读取音频数据
  i2s_read(I2S_PORT, &samples, sizeof(samples), &bytes_read, portMAX_DELAY);
  
  // 计算 RMS 值（音量）
  float sum = 0;
  int num_samples = bytes_read / sizeof(int32_t);
  
  for (int i = 0; i < num_samples; i++) {
    // 转换为 16 位有符号整数
    int16_t sample = (int16_t)(samples[i] >> 16);
    sum += sample * sample;
  }
  
  float rms = sqrt(sum / num_samples);
  
  // 输出到串口绘图器
  Serial.println(rms);
  
  delay(10);
}
```

### 示例 2：噪音级别检测

```cpp
#include <driver/i2s.h>

#define I2S_WS  15
#define I2S_SD  32
#define I2S_SCK 14
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024

// 噪音级别阈值
#define NOISE_LOW     500
#define NOISE_MEDIUM  2000
#define NOISE_HIGH    5000

void setup() {
  Serial.begin(115200);
  
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  
  Serial.println("噪音检测器已启动");
  Serial.println("级别: 安静 | 轻微 | 中等 | 嘈杂");
}

void loop() {
  int32_t samples[BUFFER_SIZE];
  size_t bytes_read;
  
  i2s_read(I2S_PORT, &samples, sizeof(samples), &bytes_read, portMAX_DELAY);
  
  float sum = 0;
  int num_samples = bytes_read / sizeof(int32_t);
  
  for (int i = 0; i < num_samples; i++) {
    int16_t sample = (int16_t)(samples[i] >> 16);
    sum += sample * sample;
  }
  
  float rms = sqrt(sum / num_samples);
  
  // 判断噪音级别
  String level;
  if (rms < NOISE_LOW) {
    level = "安静  ";
  } else if (rms < NOISE_MEDIUM) {
    level = "轻微  ";
  } else if (rms < NOISE_HIGH) {
    level = "中等  ";
  } else {
    level = "嘈杂!!";
  }
  
  Serial.print("音量: ");
  Serial.print(rms, 0);
  Serial.print(" | 级别: ");
  Serial.println(level);
  
  delay(200);
}
```

### 示例 3：VU 表演示

```cpp
#include <driver/i2s.h>

#define I2S_WS  15
#define I2S_SD  32
#define I2S_SCK 14
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512

void setup() {
  Serial.begin(115200);
  
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  
  Serial.println("VU 表演示");
}

void loop() {
  int32_t samples[BUFFER_SIZE];
  size_t bytes_read;
  
  i2s_read(I2S_PORT, &samples, sizeof(samples), &bytes_read, portMAX_DELAY);
  
  float sum = 0;
  int num_samples = bytes_read / sizeof(int32_t);
  
  for (int i = 0; i < num_samples; i++) {
    int16_t sample = (int16_t)(samples[i] >> 16);
    sum += sample * sample;
  }
  
  float rms = sqrt(sum / num_samples);
  
  // 绘制 VU 表
  int bars = map(rms, 0, 10000, 0, 50);
  bars = constrain(bars, 0, 50);
  
  Serial.print("|");
  for (int i = 0; i < bars; i++) {
    if (i < 30) Serial.print("=");
    else if (i < 40) Serial.print("+");
    else Serial.print("*");
  }
  for (int i = bars; i < 50; i++) {
    Serial.print(" ");
  }
  Serial.print("| ");
  Serial.println(rms, 0);
  
  delay(50);
}
```

## 代码详解

### 1. I2S 配置

```cpp
i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
  .sample_rate = SAMPLE_RATE,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = I2S_COMM_FORMAT_STAND_I2S,
  // ...
};
```

- **mode**: 主机模式 + 接收模式
- **sample_rate**: 采样率 44.1kHz（CD 音质）
- **bits_per_sample**: 16 位采样精度
- **channel_format**: 仅左声道（单声道）

### 2. 数据读取

```cpp
int32_t samples[BUFFER_SIZE];
size_t bytes_read;
i2s_read(I2S_PORT, &samples, sizeof(samples), &bytes_read, portMAX_DELAY);
```

使用 `i2s_read()` 函数读取音频数据到缓冲区。

### 3. 音量计算

```cpp
float rms = sqrt(sum / num_samples);
```

使用 RMS（均方根）算法计算音量，反映音频信号的能量大小。

## 使用方法

### 1. 硬件连接

按照上面的连接图，将 INMP441 与 ESP32 连接。

### 2. 上传代码

1. 将代码复制到 Arduino IDE
2. 选择正确的开发板和端口
3. 点击上传

### 3. 查看输出

#### 串口绘图器（示例 1）

1. **工具 → 串口绘图器**
2. 波特率设置为 **115200**
3. 对着麦克风说话，观察波形变化

#### 串口监视器（示例 2、3）

1. **工具 → 串口监视器**
2. 波特率设置为 **115200**
3. 查看噪音级别或 VU 表输出

## 扩展功能

### 音频录制到 SD 卡

```cpp
#include <SD.h>
#include <driver/i2s.h>

File audioFile;

void startRecording() {
  audioFile = SD.open("/audio.raw", FILE_WRITE);
}

void recordSample() {
  int32_t samples[BUFFER_SIZE];
  size_t bytes_read;
  i2s_read(I2S_PORT, &samples, sizeof(samples), &bytes_read, portMAX_DELAY);
  audioFile.write((uint8_t*)samples, bytes_read);
}

void stopRecording() {
  audioFile.close();
}
```

### FFT 频谱分析

使用 ArduinoFFT 库进行频谱分析：

```cpp
#include <arduinoFFT.h>

ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, SAMPLES, SAMPLE_RATE);

// 执行 FFT
FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
FFT.compute(FFT_FORWARD);
FFT.complexToMagnitude();

// 获取主要频率
float peak = FFT.majorPeak();
```

### 语音激活检测 (VAD)

```cpp
bool isVoiceActive(float rms) {
  static float threshold = 1000;
  static float noise_floor = 500;
  
  // 动态阈值
  threshold = noise_floor * 2;
  
  return rms > threshold;
}
```

## 常见问题

### 1. 没有音频数据
- 检查 I2S 引脚连接是否正确
- 确认 INMP441 供电正常（3.3V）
- 检查 L/R 引脚是否接地（左声道）

### 2. 噪音过大
- 检查电源是否稳定，建议使用独立电源
- 缩短杜邦线长度，减少干扰
- 在代码中添加数字滤波

### 3. 音量太小
- 检查麦克风方向是否正确
- 确认采样率和位深度设置正确
- 尝试调整增益（部分麦克风模块支持）

### 4. 串口输出乱码
- 确认波特率设置为 115200
- 检查串口监视器编码是否为 UTF-8

## 应用场景

1. **噪音监测**：监测环境噪音水平，用于工业安全
2. **语音控制**：结合语音识别实现智能家居控制
3. **音频录制**：录制环境声音或音乐
4. **频谱分析**：分析音频频率成分
5. **声级计**：制作便携式声级计

## 参考文档

- [ESP32 I2S 官方文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html)
- [INMP441 数据手册](https://invensense.tdk.com/wp-content/uploads/2015/02/INMP441.pdf)
- [ArduinoFFT 库](https://github.com/kosme/arduinoFFT)

---

**注意**：I2S 接口对时序要求较高，建议使用较短的连接线，避免信号干扰。如果音频质量不佳，可以尝试降低采样率或检查硬件连接。
