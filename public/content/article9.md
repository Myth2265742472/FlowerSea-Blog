# YD-ESP32-S3 Python 示例代码说明

## 1. main.py - NeoPixel RGB 灯控制

```python
import time
from machine import Pin
from neopixel import NeoPixel

pin = Pin(48, Pin.OUT)   # 设置 GPIO48 为输出，用于驱动 NeoPixels
np = NeoPixel(pin,1)     # 在 GPIO48 上创建 NeoPixel 驱动器，控制 1 个像素
np[0] = (5,5, 5)         # 将第一个像素设置为白色
np.write()               # 将数据写入所有像素

while True:
    np[0] = (100,0, 0)   # 设置为红色
    np.write()
    time.sleep_ms(1000)   # 延时 1 秒

    np[0] = (0,100, 0)   # 设置为绿色
    np.write()
    time.sleep_ms(1000)

    np[0] = (0,0, 100)   # 设置为蓝色
    np.write()
    time.sleep_ms(1000)

    np[0] = (100,100, 100) # 设置为白色
    np.write()
    time.sleep_ms(1000)

    print("循环结束")
```

**功能说明：** 控制连接在 GPIO48 上的 RGB LED 循环显示红、绿、蓝、白四种颜色。

---

## 2. network.py - WiFi 连接（Station 模式）

```python
def do_connect():
    import network
    print('正在连接网络...')
    wlan = network.WLAN(network.STA_IF)  # 创建站点接口
    wlan.active(True)                      # 激活网络接口
    if not wlan.isconnected():
        print('正在连接网络...')
        wlan.connect('TP-LINK_D68D', '12345678')  # 连接到 WiFi
        while not wlan.isconnected():
            pass                           # 等待连接成功
    print('网络配置:', wlan.ifconfig())    # 打印 IP 地址等信息

do_connect()
```

**功能说明：** 连接到指定的 WiFi 热点（路由器），并显示获取到的 IP 地址。

---

## 3. ap.py - 热点模式（AP 模式）

```python
try:
    import usocket as socket
except:
    import socket
from machine import Pin
import network

led = Pin(2, Pin.OUT)  # 使用 LED 指示连接状态
led.value(1)           # LED 亮起表示正在初始化

# 用于绑定端口的变量
selfadd = '0'

# 该函数用于让手机或电脑连接到 ESP32 板子的 WiFi 热点
def do_ap_connet():
    # 创建接入点接口
    ap = network.WLAN(network.AP_IF)
    # 设置 ESP32 板子的 WiFi 名称
    ap.config(essid='ESP-AP')  # 配置实例的 essid 参数
    # 限制可以连接它的客户端数量
    ap.config(max_clients=2)

    # 检查是否有设备连接到 AP
    if not ap.isconnected():
        print('正在连接网络...')
        # 开启无线热点
        ap.active(True)
        while not ap.isconnected():
            pass
    print('网络配置:', ap.ifconfig())
    # 记录 IP 地址
    selfadd = ap.ifconfig()[0]
    led.value(0)  # LED 熄灭表示热点已就绪

# 启动 ESP32 热点，手机或其他设备可以连接
do_ap_connet()
```

**功能说明：** 将 ESP32 配置为 WiFi 热点，手机或电脑可以连接到名为 "ESP-AP" 的网络。

---

## 4. socket.py - Socket 通信（Station 模式）

```python
try:
    import usocket as socket
except:
    import socket
from machine import Pin
import network

led = Pin(2, Pin.OUT)
led.value(1)

# 用于绑定端口
selfadd = '0'

# 手机热点的名称，或路由器 WiFi 名称
essid = 'TP-LINK_D68D'
# 热点的密码，或路由器 WiFi 密码
password = '12345678'

# 该函数用于让 ESP32 连接到手机或路由器的 WiFi
def do_wifi_connet():
    # 创建站点接口
    wlan = network.WLAN(network.STA_IF)
    # 开启网络接口
    wlan.active(True)
    # 检查是否已连接到 WiFi
    if not wlan.isconnected():
        print('正在连接网络...')
        # 连接 WiFi
        wlan.connect(essid, password)
        while not wlan.isconnected():
            pass
    print('网络配置:', wlan.ifconfig())
    # 记录 IP 地址
    selfadd = wlan.ifconfig()[0]
    led.value(0)

# ESP32 连接到手机或路由器的 WiFi
do_wifi_connet()
```

**功能说明：** ESP32 作为客户端连接到指定的 WiFi 网络，准备进行 Socket 通信。

---

## 5. blink-input.py - GPIO 输入输出测试

```python
import machine
import time

pin12 = machine.Pin(12, machine.Pin.OUT)
pin12.value(0)
pin13 = machine.Pin(13, machine.Pin.IN, machine.Pin.PULL_UP)
print(pin13.value())

while True:
    print(pin13.value())
    pin12.value(0)
    time.sleep_ms(500)

    pin12.value(1)
    time.sleep_ms(500)

    print("循环结束")
```

**功能说明：**
- Pin 12 配置为输出模式，每 500ms 切换高低电平
- Pin 13 配置为输入模式，使用内部上拉电阻
- 循环打印 Pin 13 的状态并控制 Pin 12 的输出

---

## 6. test-main.py - 中断处理示例

```python
from machine import Pin
from neopixel import NeoPixel
import time

pin = Pin(48, Pin.OUT)
np = NeoPixel(pin, 1)
np[0] = (10,0,0)
np.write()
r, g, b = np[0]

def handle_interrupt(Pin):
    np[0] = (0, 0, 0)
    np.write()
    time.sleep_ms(150)

    np[0] = (0, 0, 10)
    np.write()
    time.sleep_ms(150)

    np[0] = (0, 0, 10)
    np.write()
    time.sleep_ms(150)

    np[0] = (0, 0, 0)
    np.write()
    time.sleep_ms(150)

    np[0] = (0, 10, 0)
    np.write()
    time.sleep_ms(150)

    print("test-usr key")

# 配置 Pin 0 为中断输入，下降沿触发
p0 = Pin(0)
p0.init(p0.IN, p0.PULL_UP)
p0.irq(trigger=p0.IRQ_FALLING, handler=handle_interrupt)
```

**功能说明：**
- 使用 Pin 0 作为外部中断引脚，配置为上拉输入
- 当检测到下降沿（按键按下）时触发中断
- 中断处理函数使 RGB LED 闪烁不同颜色
- 这是一个典型的按键中断处理示例

---

## 7. boot.py - 启动文件

```python
# 此文件在每次启动时执行（包括从深度睡眠唤醒）
#import esp
#esp.osdebug(None)
#import webrepl
#webrepl.start()
```

**功能说明：**
- 这是 MicroPython 的启动文件，会在系统启动时自动执行
- 当前所有代码都被注释掉了
- 取消注释 `webrepl.start()` 可以启用 WebREPL（基于 Web 的 REPL 终端）

---

## 硬件引脚说明

根据示例代码，YD-ESP32-S3 的主要引脚配置：

| 功能 | 引脚 | 说明 |
|------|------|------|
| LED | GPIO 2 | 内置 LED（通常低电平点亮） |
| NeoPixel | GPIO 48 | RGB 数据输出 |
| 输入引脚 | GPIO 0, 13 | 可配置为中断或普通输入 |
| 输出引脚 | GPIO 12 | 通用输出引脚 |

---

## 使用提示

1. **WiFi 连接**：修改 `network.py` 和 `socket.py` 中的 WiFi 名称和密码
2. **热点配置**：修改 `ap.py` 中的 `essid` 来更改热点名称
3. **中断引脚**：GPIO 0 通常作为 BOOT 按键，适合用于中断测试
4. **LED 控制**：内置 LED 在 GPIO 2 上，可用于状态指示
