# Flash 下载工具说明

## 地址配置说明

固件烧录地址配置（地址.txt）：

```
0x0     WiFiScan.ino.bootloader     # 引导加载程序（Bootloader）
0x8000  WiFiScan.ino.partitions     # 分区表（Partition Table）
0xe000  boot_app0                    # 应用程序引导程序
0x10000 WiFiScan.ino                # 主程序固件
```

### 地址说明：

1. **0x0 (Bootloader)**
   - ESP32 启动时首先执行的代码
   - 负责初始化硬件并加载主程序

2. **0x8000 (Partition Table)**
   - 定义 Flash 存储的分区信息
   - 包含应用程序分区、数据分区等配置

3. **0xe000 (Boot App)**
   - 第二阶段引导程序
   - 负责跳转到应用程序分区

4. **0x10000 (Application)**
   - 用户主程序存储位置
   - 从此地址开始运行用户代码

## Flash 下载工具使用

### 工具版本
Flash Download Tool v3.9.3

### 支持的芯片
- ESP32
- ESP32-S3
- ESP32-S2
- ESP32-C3
- ESP32-C2
- ESP8266
- ESP8285

### 使用步骤

1. **打开工具**
   - 运行 `flash_download_tool_3.9.3` 目录下的可执行文件

2. **选择芯片类型**
   - 在左侧选择对应的芯片型号（如 ESP32-S3）

3. **配置固件**
   - 点击 "..." 按钮选择要烧录的固件文件
   - 输入对应的烧录地址
   - 可以添加多个固件文件（按烧录顺序）

4. **连接设备**
   - 使用 USB 线连接 ESP32 开发板到电脑
   - 确认 COM 端口号

5. **选择端口和配置**
   - 选择正确的 COM 端口
   - 波特率设置（默认 115200 或 460800）
   - SPI 速度（默认 40MHz）

6. **开始烧录**
   - 点击 "START" 按钮开始烧录
   - 如果是手动下载模式，需要按住 BOOT 键再按 RST 键

### MicroPython 固件

#### 固件类型
- **GENERIC_S3-20220618-v1.19.1.bin** - 通用 ESP32-S3 MicroPython 固件
- **feathers3-20220618-v1.19.1.bin** - Feather S3 开发板专用固件
- **pros3-20220618-v1.19.1.bin** - Pro S3 开发板专用固件
- **YD-ESP32-S3-N16R8-MPY-V1.1.bin** - YD-ESP32-S3 16MB Flash/8MB PSRAM 版本
- **YD-ESP32-S3-N8R2-MPY-V1.1.bin** - YD-ESP32-S3 8MB Flash/2MB PSRAM 版本
- **YD-ESP32-S3-N8R8-MPY-V1.1.bin** - YD-ESP32-S3 8MB Flash/8MB PSRAM 版本

#### 烧录 MicroPython 固件

MicroPython 固件通常只需要烧录一个文件：

```
0x0   firmware.bin   # MicroPython 固件（包含 bootloader）
```

### 配置文件说明

#### security.conf
- 安全配置文件
- 包含 Flash 加密、安全启动等设置

#### spi_download.conf
- SPI 下载配置
- 定义 SPI Flash 的速度、模式等参数

#### utility.conf
- 工具配置文件
- 包含下载工具的各种设置

### 日志文件

工具会在 `logs` 目录下记录每次烧录的详细信息：
- 连接状态
- 烧录进度
- 错误信息
- 校验结果

### 常见问题

1. **无法检测到设备**
   - 检查 USB 线是否连接良好
   - 确认是否安装了正确的 USB 驱动
   - 尝试更换 USB 端口

2. **烧录失败**
   - 确认地址配置正确
   - 降低波特率尝试
   - 检查固件文件是否完整

3. **下载模式**
   - 自动下载模式：按住 RST 键
   - 手动下载模式：先按住 BOOT 键，再按 RST 键，然后松开 RST，最后松开 BOOT

### 固件验证

烧录完成后，可以使用串口工具（如 PuTTY、Tera Term 或 Arduino IDE 串口监视器）连接到 ESP32：
- 波特率：115200
- 如果看到 MicroPython 提示符 `>>>`，说明固件烧录成功

### 参考文档

- Flash Download Tool 中文文档：`doc/Flash_Download_Tool__cn.pdf`
- Flash Download Tool 英文文档：`doc/Flash_Download_Tool__en.pdf`
