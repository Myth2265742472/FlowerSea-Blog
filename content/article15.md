# ESP32 Flash 下载工具使用指南

## 工具简介

ESP32 Flash 下载工具（Flash Download Tools）是乐鑫官方提供的固件烧录工具，用于将编译好的固件文件下载到 ESP32 系列芯片的 Flash 存储器中。本工具支持 ESP32、ESP32-S2、ESP32-S3、ESP32-C2、ESP32-C3、ESP32-C6 以及 ESP8266/ESP8285 等多种芯片。

## 功能特性

- ✅ **多芯片支持**：支持 ESP32 全系列芯片
- ✅ **多固件烧录**：可同时烧录多个固件文件到不同地址
- ✅ **分区表配置**：支持自定义分区表
- ✅ **波特率调节**：支持多种波特率，最高可达 921600
- ✅ **SPI 配置**：可配置 Flash 大小、模式、频率
- ✅ **擦除功能**：支持全片擦除和选择性擦除
- ✅ **校验功能**：烧录后自动校验

## 所需材料

### 硬件
- **ESP32 开发板** (如 ESP32 DevKit V1、YD-ESP32-S3 等)
- **USB 数据线** 一条

### 软件
- **Flash Download Tool** (v3.9.3 或更高版本)
- **待烧录的固件文件** (.bin 格式)

## 下载与安装

### 1. 下载工具

从乐鑫官网下载 Flash Download Tool：
- 官网地址：https://www.espressif.com.cn/zh-hans/support/download/other-tools
- 或直接搜索 "ESP32 Flash Download Tool"

### 2. 解压工具

下载后解压到任意目录，无需安装，直接运行 `flash_download_tool_3.9.3.exe` 即可。

## 固件文件说明

### 典型的固件文件组成

| 文件名 | 烧录地址 | 说明 |
|--------|---------|------|
| bootloader.bin | 0x1000 | 引导加载程序 |
| partition-table.bin | 0x8000 | 分区表 |
| app.bin | 0x10000 | 主应用程序 |
| app2.bin | 0x110000 | 第二个应用程序（可选） |

### 获取固件文件

#### 方式 1：自行编译

使用 ESP-IDF 或 Arduino IDE 编译项目后，在 build 目录中会生成 .bin 文件。

#### 方式 2：下载预编译固件

从项目发布页面或论坛下载已编译好的固件。

#### 方式 3：MicroPython 固件

从 MicroPython 官网下载适用于 ESP32 的固件：
- https://micropython.org/download/?port=esp32

## 烧录步骤

### 1. 连接硬件

1. 使用 USB 数据线将 ESP32 开发板连接到电脑
2. 确认电脑识别到串口（设备管理器中查看 COM 口号）

### 2. 启动工具

双击运行 `flash_download_tool_3.9.3.exe`，选择对应的芯片型号：

```
┌─────────────────────────────────────┐
│  ESP32 Download Tool v3.9.3         │
├─────────────────────────────────────┤
│  请选择芯片型号:                      │
│                                     │
│  ○ ESP32                            │
│  ○ ESP32-S2                         │
│  ● ESP32-S3    ← 选择你的芯片        │
│  ○ ESP32-C2                         │
│  ○ ESP32-C3                         │
│  ○ ESP32-C6                         │
│  ○ ESP8266                          │
│                                     │
│  [确定]  [取消]                      │
└─────────────────────────────────────┘
```

### 3. 配置烧录参数

#### 3.1 选择固件文件

点击每个路径框后面的 "..." 按钮，选择对应的 .bin 文件：

```
┌──────────────────────────────────────────────────────────┐
│  路径                                    地址            │
├──────────────────────────────────────────────────────────┤
│  [浏览...] D:\firmware\bootloader.bin    [0x1000  ]     │
│  [浏览...] D:\firmware\partition-table.bin [0x8000 ]     │
│  [浏览...] D:\firmware\app.bin           [0x10000 ]     │
│  [浏览...]                               [       ]      │
└──────────────────────────────────────────────────────────┘
```

#### 3.2 配置 SPI 参数

```
SPI 速度: 40MHz
SPI 模式: DIO
Flash 大小: 32Mbit (4MB)
```

- **SPI 速度**：一般选择 40MHz 或 80MHz
- **SPI 模式**：DIO（双线）或 QIO（四线），一般选 DIO
- **Flash 大小**：根据实际芯片选择，常见为 4MB (32Mbit)

#### 3.3 配置串口参数

```
COM 端口: COM3 (选择你的端口)
波特率: 921600 (或 460800、115200)
```

### 4. 进入下载模式

#### 自动下载

大部分开发板支持自动下载，直接点击 "START" 即可。

#### 手动下载

如果自动下载失败，需要手动进入下载模式：

1. **按住 BOOT 按钮**（或 GPIO0 接地）
2. **按一下 EN 按钮**（复位）
3. **松开 BOOT 按钮**
4. 点击 "START" 开始烧录

### 5. 开始烧录

点击 "START" 按钮，等待烧录完成：

```
[2024-01-15 10:30:25] 开始烧录...
[2024-01-15 10:30:26] 连接芯片...
[2024-01-15 10:30:27] 芯片型号: ESP32-S3
[2024-01-15 10:30:28] 擦除 Flash...
[2024-01-15 10:30:35] 烧录 bootloader.bin (0x1000)...
[2024-01-15 10:30:38] 烧录 partition-table.bin (0x8000)...
[2024-01-15 10:30:40] 烧录 app.bin (0x10000)...
[2024-01-15 10:30:55] 校验...
[2024-01-15 10:30:58] 烧录完成!
```

### 6. 验证烧录

烧录完成后，按一下 EN 按钮（复位），查看串口输出确认程序正常运行。

## MicroPython 固件烧录

### 1. 下载 MicroPython 固件

从官网下载对应芯片的固件：
- ESP32: `esp32-20240105-v1.22.1.bin`
- ESP32-S3: `ESP32_GENERIC_S3-20240105-v1.22.1.bin`

### 2. 配置烧录

```
路径: D:\firmware\esp32-20240105-v1.22.1.bin
地址: 0x1000
```

注意：MicroPython 固件通常只需要烧录一个文件，地址为 0x1000。

### 3. 烧录并验证

烧录完成后，使用串口工具（如 PuTTY、SecureCRT）连接，波特率 115200，即可看到 MicroPython 交互式解释器：

```
MicroPython v1.22.1 on 2024-01-05; ESP32 module with ESP32
Type "help()" for more information.
>>>
```

## 常见问题

### 1. 无法连接芯片

**现象**：提示 "Failed to connect to ESP32"

**解决方法**：
- 检查 USB 线是否支持数据传输（部分线仅充电）
- 确认 COM 端口选择正确
- 手动进入下载模式（按住 BOOT，按 EN，松开 BOOT）
- 检查驱动是否安装（CH340/CP2102 驱动）

### 2. 烧录失败或校验错误

**现象**：烧录到一半失败，或校验不通过

**解决方法**：
- 降低波特率（如从 921600 降到 460800 或 115200）
- 检查固件文件是否完整
- 确认 Flash 大小设置正确
- 尝试先擦除整个 Flash 再烧录

### 3. 烧录后无法启动

**现象**：烧录完成但程序不运行

**解决方法**：
- 检查烧录地址是否正确
- 确认分区表配置正确
- 检查是否选择了正确的芯片型号
- 查看串口输出（波特率 115200）获取错误信息

### 4. 串口无法识别

**现象**：设备管理器中没有 COM 端口

**解决方法**：
- 安装 USB 转串口驱动：
  - CH340: http://www.wch.cn/downloads/CH341SER_ZIP.html
  - CP2102: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- 更换 USB 端口或数据线
- 检查开发板是否通电（电源指示灯是否亮）

## 高级功能

### 1. 擦除 Flash

点击 "ERASE" 按钮可以擦除整个 Flash，适用于清除旧固件或解决启动问题。

### 2. 读取 Flash

点击 "READ" 按钮可以读取 Flash 内容并保存到文件，用于备份固件。

### 3. 合并固件

使用 `esptool.py merge_bin` 命令可以将多个固件合并为一个文件：

```bash
esptool.py --chip esp32 merge_bin -o merged_firmware.bin \
    --flash_mode dio --flash_size 4MB \
    0x1000 bootloader.bin \
    0x8000 partition-table.bin \
    0x10000 app.bin
```

### 4. 命令行烧录

使用 `esptool.py` 进行命令行烧录：

```bash
# 安装 esptool
pip install esptool

# 烧录固件
esptool.py --chip esp32 --port COM3 --baud 921600 write_flash \
    -z --flash_mode dio --flash_freq 40m --flash_size 4MB \
    0x1000 bootloader.bin \
    0x8000 partition-table.bin \
    0x10000 app.bin

# 擦除 Flash
esptool.py --chip esp32 --port COM3 erase_flash

# 读取 Flash
esptool.py --chip esp32 --port COM3 read_flash 0x0 0x400000 flash_dump.bin
```

## 不同芯片的注意事项

### ESP32
- 经典款，功能全面
- 烧录地址：bootloader 0x1000, partition 0x8000, app 0x10000

### ESP32-S2/S3
- 支持 USB OTG，可直接通过 USB 下载
- ESP32-S3 支持 AI 加速
- 烧录地址与 ESP32 相同

### ESP32-C2/C3/C6
- RISC-V 架构
- 更小的封装，适合物联网应用
- 烧录地址可能不同，请参考具体文档

## 参考文档

- [乐鑫 Flash 下载工具文档](https://docs.espressif.com/projects/esp-test-tools/zh_CN/latest/esp32/production_stage/tools/flash_download_tool.html)
- [esptool.py 文档](https://docs.espressif.com/projects/esptool/en/latest/esp32/)
- [ESP32 启动流程](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/startup.html)

---

**注意**：烧录固件前请确保固件来源可靠，错误的固件可能导致设备无法启动。建议在烧录前备份原有固件。
