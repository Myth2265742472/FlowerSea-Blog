# ESP32 蓝牙低功耗(BLE)通信入门

## 项目简介

本项目展示了如何使用 ESP32 开发板创建蓝牙低功耗(BLE)服务器，实现与手机或其他蓝牙设备的无线通信。通过简单的代码，即可让 ESP32 广播蓝牙信号，接收和发送数据，非常适合物联网(IoT)应用场景。

## 功能特性

- ✅ **BLE 服务器模式**：ESP32 作为蓝牙服务器等待连接
- ✅ **自定义服务与特征**：支持读写和通知属性
- ✅ **连接状态回调**：实时监测设备连接/断开
- ✅ **数据通知**：定时向连接设备发送消息
- ✅ **自动重连**：断开后自动恢复广播

## 所需材料

### 硬件
- **ESP32 开发板** (如 ESP32 DevKit V1)
- **USB 数据线** 一条

### 软件
- **Arduino IDE**
- **ESP32 开发板支持包**
- **BLE 库** (Arduino 内置)

## 硬件连接

无需额外连接，直接使用 ESP32 内置蓝牙模块。

```
ESP32
-----
USB -> 电脑（用于供电和上传代码）
```

## 软件准备

### 1. 安装 Arduino IDE

从 [Arduino 官网](https://www.arduino.cc/en/software) 下载并安装 Arduino IDE。

### 2. 添加 ESP32 开发板支持

1. 打开 Arduino IDE，进入 **文件 → 首选项**
2. 在"附加开发板管理器网址"中添加：
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
3. 进入 **工具 → 开发板 → 开发板管理器**
4. 搜索 "ESP32" 并安装 "ESP32 by Espressif Systems"

### 3. 选择开发板

1. **工具 → 开发板 → ESP32 Arduino → ESP32 Dev Module**
2. **工具 → 端口** 选择正确的 COM 端口

## 完整代码

```cpp
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// 定义服务和特征的 UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

// 连接状态回调类
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("设备已连接");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("设备已断开");
      // 断开后重新开启广播
      pServer->getAdvertising()->start();
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("启动ESP32蓝牙...");

  // 创建BLE设备
  BLEDevice::init("ESP32-BLE");

  // 创建BLE服务器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 创建BLE服务
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 创建BLE特征
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // 添加描述符
  pCharacteristic->addDescriptor(new BLE2902());

  // 启动服务
  pService->start();

  // 开始广播
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("ESP32蓝牙已启动，等待连接...");
}

void loop() {
  if (deviceConnected) {
    // 当设备连接时，每2秒发送一次数据
    String message = "Hello from ESP32";
    pCharacteristic->setValue(message.c_str());
    pCharacteristic->notify();
    delay(2000);
  }
  delay(1000);
}
```

## 代码详解

### 1. UUID 定义

```cpp
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
```

UUID（通用唯一识别码）用于标识蓝牙服务和特征。可以使用在线 UUID 生成器创建自己的 UUID。

### 2. 连接状态回调

```cpp
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("设备已连接");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("设备已断开");
      pServer->getAdvertising()->start();  // 重新广播
    }
};
```

当手机或其他设备连接/断开时，会自动调用这些回调函数。

### 3. 特征属性

```cpp
BLECharacteristic::PROPERTY_READ   |  // 可读
BLECharacteristic::PROPERTY_WRITE  |  // 可写
BLECharacteristic::PROPERTY_NOTIFY     // 可通知
```

支持三种操作模式：
- **READ**：客户端可以读取特征值
- **WRITE**：客户端可以写入特征值
- **NOTIFY**：服务器可以主动推送数据

## 使用方法

### 1. 上传代码

1. 将代码复制到 Arduino IDE
2. 选择正确的开发板和端口
3. 点击"上传"按钮

### 2. 查看串口输出

打开 **工具 → 串口监视器**，波特率设置为 **115200**，应看到：

```
启动ESP32蓝牙...
ESP32蓝牙已启动，等待连接...
```

### 3. 使用手机连接

1. 下载 BLE 调试助手 APP（如 "nRF Connect" 或 "LightBlue"）
2. 打开 APP，搜索蓝牙设备
3. 找到 "ESP32-BLE" 并连接
4. 查看服务和特征 UUID
5. 可以读取/写入数据，或接收通知

## 扩展功能

### 添加多个特征

```cpp
#define CHARACTERISTIC_UUID_2 "beb5483e-36e1-4688-b7f5-ea07361b26a9"

BLECharacteristic* pCharacteristic2 = pService->createCharacteristic(
    CHARACTERISTIC_UUID_2,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
);
pCharacteristic2->setValue("第二个特征");
```

### 接收客户端数据

```cpp
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      Serial.print("收到数据: ");
      Serial.println(value.c_str());
    }
};

pCharacteristic->setCallbacks(new MyCallbacks());
```

### 修改广播名称

```cpp
BLEDevice::init("My-IoT-Device");  // 修改为你的设备名称
```

## 常见问题

### 1. 无法找到蓝牙设备
- 确认代码已成功上传到 ESP32
- 检查串口输出是否有错误信息
- 确保手机蓝牙已开启
- 尝试重启 ESP32

### 2. 连接后断开
- 检查电源是否稳定
- 确认 UUID 是否正确
- 尝试降低广播间隔

### 3. 无法接收通知
- 确认特征属性包含 PROPERTY_NOTIFY
- 检查是否已添加 BLE2902 描述符
- 在 APP 中启用通知功能

## 应用场景

1. **智能家居**：手机控制 ESP32 连接的继电器、LED 等
2. **传感器数据**：定时发送温度、湿度等传感器数据
3. **无线调试**：通过蓝牙查看设备日志和状态
4. **可穿戴设备**：与智能手环、手表等设备通信

## 参考文档

- [ESP32 BLE 官方文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/bt_le.html)
- [Arduino ESP32 BLE 官方库](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE)

---

**注意**：蓝牙和 WiFi 共用 2.4GHz 频段，同时使用可能会产生干扰。建议根据实际需求选择通信方式。
