# YD-ESP32-S3 MicroPython 开发入门

## 项目简介

YD-ESP32-S3 是一款基于 ESP32-S3 芯片的开发板，集成了 WiFi、蓝牙、USB OTG 等功能。本项目展示了如何使用 MicroPython 在该开发板上进行快速开发，包括 LED 控制、WiFi 连接、网络通信等基础功能。

## 功能特性

- ✅ **LED 控制**：控制板载 RGB LED 和 GPIO  LED
- ✅ **WiFi 连接**：支持 Station 模式和 AP 模式
- ✅ **网络通信**：Socket 客户端通信
- ✅ **GPIO 控制**：数字输入输出、外部中断
- ✅ **WebREPL**：通过浏览器远程控制开发板

## 所需材料

### 硬件
- **YD-ESP32-S3 开发板**
- **USB Type-C 数据线** 一条
- **LED** 若干（可选）
- **杜邦线** 若干（可选）

### 软件
- **MicroPython 固件**
- **Thonny IDE** 或 **ampy**
- **Flash Download Tool**（用于烧录固件）

## 硬件规格

### YD-ESP32-S3 主要特性

| 特性 | 参数 |
|------|------|
| 芯片 | ESP32-S3-WROOM-1 |
| CPU | Xtensa LX7 双核 @ 240MHz |
| WiFi | 802.11 b/g/n |
| 蓝牙 | BLE 5.0 + 经典蓝牙 |
| Flash | 8MB |
| PSRAM | 8MB |
| USB | USB OTG + USB 串口 |
| 工作电压 | 3.3V |

### 板载资源

| 资源 | 引脚 | 说明 |
|------|------|------|
| RGB LED | GPIO 48 | 板载 NeoPixel |
| LED | GPIO 2 | 板载指示灯 |
| 按键 | GPIO 0 | Boot 按键 |

## 软件准备

### 1. 下载 MicroPython 固件

从 MicroPython 官网下载 ESP32-S3 固件：
- 官网：https://micropython.org/download/?port=esp32
- 选择 `ESP32_GENERIC_S3` 或 `YD_ESP32_S3` 固件

### 2. 烧录固件

使用 Flash Download Tool 烧录：

```
固件文件: YD_ESP32_S3-20240105-v1.22.1.bin
烧录地址: 0x1000
```

详细步骤请参考《ESP32 Flash 下载工具使用指南》。

### 3. 安装 Thonny IDE

Thonny 是推荐的 MicroPython 开发工具：

1. 下载地址：https://thonny.org/
2. 安装后配置解释器：**工具 → 选项 → 解释器 → MicroPython (ESP32)**
3. 选择正确的串口

## 完整代码示例

### 示例 1：RGB LED 控制

```python
from machine import Pin
from neopixel import NeoPixel
import time

# 配置 RGB LED
LED_PIN = 48
led = NeoPixel(Pin(LED_PIN), 1)

def set_color(r, g, b):
    """设置 RGB 颜色"""
    led[0] = (r, g, b)
    led.write()

def rainbow_cycle(wait=0.05):
    """彩虹循环效果"""
    for j in range(255):
        for i in range(1):
            pixel_index = (i * 256 // 1) + j
            led[i] = wheel(pixel_index & 255)
        led.write()
        time.sleep(wait)

def wheel(pos):
    """生成彩虹颜色"""
    if pos < 85:
        return (255 - pos * 3, pos * 3, 0)
    elif pos < 170:
        pos -= 85
        return (0, 255 - pos * 3, pos * 3)
    else:
        pos -= 170
        return (pos * 3, 0, 255 - pos * 3)

# 主程序
print("RGB LED 测试")

# 红绿蓝白循环
colors = [
    (255, 0, 0),    # 红
    (0, 255, 0),    # 绿
    (0, 0, 255),    # 蓝
    (255, 255, 255) # 白
]

for color in colors:
    set_color(*color)
    print(f"颜色: RGB{color}")
    time.sleep(1)

# 彩虹效果
print("彩虹效果...")
for _ in range(3):
    rainbow_cycle()

# 关闭 LED
set_color(0, 0, 0)
print("测试完成")
```

### 示例 2：WiFi Station 模式

```python
import network
import time

# WiFi 配置
SSID = 'YOUR_WIFI_SSID'
PASSWORD = 'YOUR_WIFI_PASSWORD'

def connect_wifi(ssid, password):
    """连接 WiFi"""
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    
    if not wlan.isconnected():
        print(f'正在连接 WiFi: {ssid}...')
        wlan.connect(ssid, password)
        
        # 等待连接，最多 30 秒
        timeout = 30
        while not wlan.isconnected() and timeout > 0:
            time.sleep(1)
            timeout -= 1
            print('.', end='')
    
    if wlan.isconnected():
        print('\nWiFi 连接成功!')
        print(f'IP 地址: {wlan.ifconfig()[0]}')
        print(f'子网掩码: {wlan.ifconfig()[1]}')
        print(f'网关: {wlan.ifconfig()[2]}')
        print(f'DNS: {wlan.ifconfig()[3]}')
        return True
    else:
        print('\nWiFi 连接失败!')
        return False

# 连接 WiFi
if connect_wifi(SSID, PASSWORD):
    # 网络连接成功后的操作
    print("可以进行网络通信了")
else:
    print("请检查 WiFi 配置")
```

### 示例 3：WiFi AP 模式

```python
import network
import time

# AP 配置
AP_SSID = 'ESP32-AP'
AP_PASSWORD = '12345678'
AP_CHANNEL = 6

def create_ap(ssid, password, channel=6):
    """创建 WiFi 热点"""
    ap = network.WLAN(network.AP_IF)
    ap.active(True)
    
    # 配置 AP
    ap.config(
        essid=ssid,
        password=password,
        channel=channel,
        authmode=network.AUTH_WPA_WPA2_PSK
    )
    
    print(f'AP 模式已启动')
    print(f'热点名称: {ssid}')
    print(f'密码: {password}')
    print(f'IP 地址: {ap.ifconfig()[0]}')
    print(f'子网掩码: {ap.ifconfig()[1]}')
    
    return ap

# 创建热点
ap = create_ap(AP_SSID, AP_PASSWORD)

# 查看连接的设备数量
while True:
    connected = ap.status('stations')
    print(f'已连接设备: {len(connected)}')
    for station in connected:
        print(f'  MAC: {station}')
    time.sleep(5)
```

### 示例 4：Socket 通信

```python
import socket
import network
import time

def http_get(url):
    """HTTP GET 请求"""
    # 解析 URL
    _, _, host, path = url.split('/', 3)
    
    # 获取 IP 地址
    addr = socket.getaddrinfo(host, 80)[0][-1]
    
    # 创建 socket
    s = socket.socket()
    s.connect(addr)
    
    # 发送 HTTP 请求
    request = f'GET /{path} HTTP/1.0\r\nHost: {host}\r\n\r\n'
    s.send(request.encode())
    
    # 接收响应
    response = b''
    while True:
        data = s.recv(1024)
        if data:
            response += data
        else:
            break
    
    s.close()
    return response.decode()

# 使用示例（需要先连接 WiFi）
# response = http_get('http://api.example.com/data')
# print(response)

# 创建 TCP 服务器
def start_server(port=8080):
    """启动 TCP 服务器"""
    addr = socket.getaddrinfo('0.0.0.0', port)[0][-1]
    
    s = socket.socket()
    s.bind(addr)
    s.listen(5)
    
    print(f'服务器启动在端口 {port}')
    
    while True:
        conn, addr = s.accept()
        print(f'客户端连接: {addr}')
        
        request = conn.recv(1024)
        print(f'收到数据: {request}')
        
        # 发送响应
        response = 'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n'
        response += '<h1>Hello from ESP32-S3!</h1>'
        conn.send(response.encode())
        
        conn.close()

# 启动服务器（在新线程中运行）
# import _thread
# _thread.start_new_thread(start_server, ())
```

### 示例 5：GPIO 输入输出

```python
from machine import Pin
import time

# 配置 GPIO
OUTPUT_PIN = 12  # 输出引脚
INPUT_PIN = 13   # 输入引脚

# 创建输出引脚
output = Pin(OUTPUT_PIN, Pin.OUT)

# 创建输入引脚（带内部上拉）
input_pin = Pin(INPUT_PIN, Pin.IN, Pin.PULL_UP)

def blink_led(times=5, delay=0.5):
    """LED 闪烁"""
    for i in range(times):
        output.value(1)  # 高电平
        print(f"LED ON ({i+1}/{times})")
        time.sleep(delay)
        
        output.value(0)  # 低电平
        print("LED OFF")
        time.sleep(delay)

def read_input():
    """读取输入状态"""
    value = input_pin.value()
    print(f"输入状态: {value}")
    return value

# 测试输出
print("测试 GPIO 输出...")
blink_led(3)

# 测试输入
print("\n测试 GPIO 输入...")
print("请改变引脚状态（连接/断开）")
for _ in range(10):
    read_input()
    time.sleep(1)
```

### 示例 6：外部中断

```python
from machine import Pin
import time

# 配置中断引脚
INTERRUPT_PIN = 0  # Boot 按键

# 中断计数器
counter = 0

def interrupt_handler(pin):
    """中断处理函数"""
    global counter
    counter += 1
    print(f"中断触发! 计数: {counter}")

# 创建引脚并配置中断
button = Pin(INTERRUPT_PIN, Pin.IN, Pin.PULL_UP)

# 配置下降沿触发中断
button.irq(trigger=Pin.IRQ_FALLING, handler=interrupt_handler)

print("外部中断测试")
print("按下 Boot 按键触发中断...")
print("按 Ctrl+C 停止")

# 主循环
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("\n测试结束")
    button.irq(handler=None)  # 禁用中断
```

### 示例 7：启动文件配置

```python
# boot.py - 启动时自动运行
import network
import webrepl

# 连接 WiFi
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
if not wlan.isconnected():
    wlan.connect('YOUR_WIFI_SSID', 'YOUR_WIFI_PASSWORD')
    while not wlan.isconnected():
        pass

print('WiFi 已连接:', wlan.ifconfig()[0])

# 启动 WebREPL
webrepl.start()
print('WebREPL 已启动')
```

## 代码详解

### 1. NeoPixel RGB LED

```python
from neopixel import NeoPixel

led = NeoPixel(Pin(48), 1)  # GPIO48, 1 个 LED
led[0] = (255, 0, 0)        # 设置红色
led.write()                  # 写入数据
```

NeoPixel 使用 WS2812B 协议，通过单线控制 RGB LED。

### 2. WiFi 配置

```python
wlan = network.WLAN(network.STA_IF)  # Station 模式
wlan = network.WLAN(network.AP_IF)   # AP 模式
```

ESP32-S3 支持双模 WiFi，可同时作为 Station 和 AP。

### 3. Socket 通信

```python
s = socket.socket()
s.connect(addr)
s.send(data)
response = s.recv(1024)
```

MicroPython 的 socket 接口与标准 Python 兼容。

## 使用方法

### 1. 连接开发板

1. 使用 USB Type-C 线连接 YD-ESP32-S3 到电脑
2. 确认设备管理器中识别到串口

### 2. 配置 Thonny

1. 打开 Thonny IDE
2. **工具 → 选项 → 解释器**
3. 选择 **MicroPython (ESP32)**
4. 选择正确的串口
5. 点击 **确定**

### 3. 运行代码

1. 在 Thonny 中编写或粘贴代码
2. 点击 **运行** 按钮（或按 F5）
3. 选择 **MicroPython 设备** 保存到开发板

### 4. 上传文件

```bash
# 使用 ampy 上传文件
ampy --port COM3 put main.py

# 使用 webrepl_cli
webrepl_cli.py -p password main.py 192.168.1.100:
```

## 扩展功能

### 使用传感器

```python
from machine import ADC, Pin
import time

# 配置 ADC
adc = ADC(Pin(36))
adc.atten(ADC.ATTN_11DB)  # 0-3.3V
adc.width(ADC.WIDTH_12BIT)  # 12 位精度

# 读取模拟值
while True:
    value = adc.read()
    voltage = value / 4095 * 3.3
    print(f"ADC 值: {value}, 电压: {voltage:.2f}V")
    time.sleep(1)
```

### PWM 控制

```python
from machine import Pin, PWM

# 配置 PWM
pwm = PWM(Pin(2))
pwm.freq(1000)  # 1kHz 频率

# 渐变效果
for duty in range(0, 1024, 10):
    pwm.duty(duty)
    time.sleep(0.01)

pwm.deinit()
```

### I2C 通信

```python
from machine import I2C, Pin

# 配置 I2C
i2c = I2C(0, scl=Pin(22), sda=Pin(21), freq=400000)

# 扫描设备
devices = i2c.scan()
print(f"发现设备: {devices}")

# 读写数据
i2c.writeto(0x50, b'hello')
data = i2c.readfrom(0x50, 5)
```

## 常见问题

### 1. 无法连接串口
- 检查 USB 线是否支持数据传输
- 安装 CH340/CP2102 驱动
- 确认串口未被其他程序占用

### 2. 代码运行报错
- 检查语法是否正确
- 确认引脚编号正确
- 查看错误信息定位问题

### 3. WiFi 连接失败
- 确认 WiFi 名称和密码正确
- 检查 WiFi 是否为 2.4GHz
- 确认信号强度足够

### 4. 内存不足
- 优化代码，减少全局变量
- 使用 `gc.collect()` 手动回收内存
- 避免创建大对象

## 参考文档

- [MicroPython ESP32 文档](https://docs.micropython.org/en/latest/esp32/quickref.html)
- [YD-ESP32-S3 开发板资料](https://github.com/vcc-gnd/YD-ESP32-S3)
- [ESP32-S3 技术规格书](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf)

---

**注意**：使用 MicroPython 时请注意内存管理，ESP32-S3 虽然有 8MB PSRAM，但 MicroPython 默认使用内部 RAM。大程序可能需要优化或使用外部存储。
