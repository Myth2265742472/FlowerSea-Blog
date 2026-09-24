# 基于51单片机及DS18B20温度传感器的数字温度计设计

## 项目简介

本项目是一个完整的数字温度计系统，基于经典的 51 单片机（STC89C52）和 DS18B20 数字温度传感器。系统采用四位共阴数码管显示温度，支持温度上下限设置、超限报警、按键音等功能。同时通过 ESP32 实现 WiFi 联网，可在网页端实时查看温度数据。

## 功能特性

- ✅ **实时温度测量**：使用 DS18B20 传感器，精度 ±0.5°C
- ✅ **数码管显示**：四位数码管显示温度，带摄氏度符号
- ✅ **温度上下限设置**：支持设置温度报警上下限
- ✅ **超限报警**：温度超出范围时蜂鸣器报警
- ✅ **按键音反馈**：按键操作带声音提示
- ✅ **WiFi 联网**：通过 ESP32 实现无线数据传输
- ✅ **网页监控**：手机/电脑浏览器实时查看温度

## 系统架构

```
┌─────────────────┐     串口通信      ┌─────────────────┐     WiFi      ┌─────────────────┐
│   51单片机       │ ◄──────────────► │   ESP32         │ ◄──────────► │   网页端         │
│ STC89C52        │    9600波特率     │                 │              │  温度监控页面     │
│ + DS18B20       │                  │ + WiFi模块       │              │                 │
│ + 数码管        │                  │                 │              │                 │
│ + 按键          │                  │                 │              │                 │
└─────────────────┘                  └─────────────────┘              └─────────────────┘
```

## 所需材料

### 硬件
- **STC89C52 单片机** 或 AT89S52
- **DS18B20 温度传感器**
- **四位共阴数码管**
- **ESP32 开发板**（用于联网功能）
- **蜂鸣器**
- **按键** x4
- **电阻、电容** 若干
- **杜邦线**
- **5V 电源**

### 软件
- **Keil uVision**（51单片机开发）
- **Arduino IDE**（ESP32开发）
- **STC-ISP**（烧录工具）

## 硬件连接

### 51单片机部分

```
STC89C52
--------
P0.0-P0.7  -> 数码管段选 (a-g, dp)
P2.4       -> 数码管位选1 (千位)
P2.5       -> 数码管位选2 (百位)
P2.6       -> 数码管位选3 (十位)
P2.7       -> 数码管位选4 (个位/符号)
P2.2       -> DS18B20 数据线
P2.3       -> 蜂鸣器
P3.4       -> 按键 K2 (模式切换)
P3.5       -> 按键 K3 (减/显示下限)
P3.6       -> 按键 K4 (加/显示上限)
P3.7       -> 按键 K5 (按键音开关)
P3.0       -> ESP32 TX (串口接收)
P3.1       -> ESP32 RX (串口发送)
```

### ESP32 部分

```
ESP32
-----
GPIO 17 (TX) -> 51单片机 P3.0 (RX)
GPIO 16 (RX) -> 51单片机 P3.1 (TX)
GPIO 2       -> LED 指示灯（可选）
```

## 51单片机代码

### 核心代码

```c
#include <reg52.h>
#include <stdio.h>

#define uchar unsigned char
#define uint unsigned int
#define dula P0		//段选信号的锁存器控制

// 数码管段码表
unsigned char code table[] = {
    0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,
    0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71
};

// 带小数点的段码表
unsigned char code table1[] = {
    0xbf,0x86,0xdb,0xcf,0xe6,0xed,0xfd,
    0x87,0xff,0xef
};

// 数码管位选信号
sbit wei1 = P2^4;  // 第1位
sbit wei2 = P2^5;  // 第2位
sbit wei3 = P2^6;  // 第3位
sbit wei4 = P2^7;  // 第4位
sbit DS = P2^2;    // DS18B20数据线

// 按键定义
sbit K2 = P3^4;    // 模式切换键
sbit K3 = P3^5;    // 减键/显示下限键
sbit K4 = P3^6;    // 加键/显示上限键
sbit K5 = P3^7;    // 按键音开关键

// 蜂鸣器
sbit BEEP = P2^3;  // 蜂鸣器控制

// 全局变量
uint temp;         // 温度变量
uint temp_high = 300;  // 温度上限，默认30.0°C
uint temp_low = 0;     // 温度下限，默认0.0°C
uchar mode = 0;        // 0:正常模式 1:设置上限 2:设置下限
bit show_high_flag = 0; // 显示上限标志
bit show_low_flag = 0;  // 显示下限标志
uint show_counter = 0;  // 显示计数器
bit key_sound_enabled = 1; // 按键音开关，默认开启

// 延时函数
void delay(uint count) {
    uint i;
    while(count) {
        i = 200;
        while(i > 0)
            i--;
        count--;
    }
}

// 毫秒延时函数
void delay_ms(uint ms) {
    uint i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 120; j++);
}

// 按键音函数
void play_key_sound() {
    if(key_sound_enabled) {
        BEEP = 0;    // 打开蜂鸣器
        delay_ms(50); // 短暂发声
        BEEP = 1;    // 关闭蜂鸣器
    }
}

// DS18B20复位
void dsreset(void) {
    uint i;
    DS = 0;		              
    i = 103;				   // 将总线拉低480us~960us
    while(i > 0) i--;
    DS = 1;				   // 然后拉高总线
    i = 4;				   // 15us~60us等待
    while(i > 0) i--;
}

// 读取一个位
bit tmpreadbit(void) {
    uint i;
    bit dat;
    DS = 0;
    i++;          // i++ for delay
    DS = 1;
    i++;
    i++;
    dat = DS;
    i = 8;
    while(i > 0) i--;
    return(dat);
}

// 读取一个字节
uchar tmpread(void) {
    uchar i, j, dat;
    dat = 0;
    for(i = 1; i <= 8; i++) {
        j = tmpreadbit();
        dat = (j << 7) | (dat >> 1);   // 读出的数据最低位在最前面
    }
    return(dat);
}

// 写一个字节到DS18B20
void tmpwritebyte(uchar dat) {
    uint i;
    uchar j;
    bit testb;
    for(j = 1; j <= 8; j++) {
        testb = dat & 0x01;
        dat = dat >> 1;
        if(testb) {    // write 1
            DS = 0;
            i++;
            i++;
            DS = 1;
            i = 8;
            while(i > 0) i--;
        } else {
            DS = 0;       // write 0
            i = 8;
            while(i > 0) i--;
            DS = 1;
            i++;
            i++;
        }
    }
}

// DS18B20开始温度转换
void tmpchange(void) {
    dsreset();
    delay(1);
    tmpwritebyte(0xcc);  // address all drivers on bus
    tmpwritebyte(0x44);  // initiates a single temperature conversion
}

// 获取温度值
uint tmp() {
    float tt;
    uchar a, b;
    dsreset();
    delay(1);
    tmpwritebyte(0xcc);
    tmpwritebyte(0xbe);
    a = tmpread();    // 低八位
    b = tmpread();    // 高八位
    temp = b;
    temp <<= 8;       // two byte compose a int variable
    temp = temp | a;
    tt = temp * 0.0625; // 算出来的是测到的温度
    temp = tt * 10 + 0.5; // 为了显示温度后的小数点后一位并作出四舍五入
    return temp;
}

// 检测按键
void check_keys() {
    // K2键：模式切换
    if(K2 == 0) {
        delay_ms(10); // 消抖
        if(K2 == 0) {
            uchar old_mode = mode;
            mode = (mode + 1) % 3; // 循环切换模式
            
            // 从设置模式切换回正常模式时，清除显示标志
            if((old_mode == 1 || old_mode == 2) && mode == 0) {
                show_high_flag = 0;
                show_low_flag = 0;
                show_counter = 0;
            }
            
            play_key_sound(); // 播放按键音
            while(!K2); // 等待按键释放
        }
    }
    
    // K3键：减键/显示下限键
    if(K3 == 0) {
        delay_ms(10); // 消抖
        if(K3 == 0) {
            if(mode == 1) { // 设置上限模式
                if(temp_high > temp_low + 10) temp_high -= 10; // 减1°C
            } else if(mode == 2) { // 设置下限模式
                if(temp_low > 0) temp_low -= 10; // 减1°C，最低0°C
            } else { // 正常模式，显示下限温度1秒
                show_low_flag = 1;
                show_counter = 100; // 显示约1秒
            }
            play_key_sound(); // 播放按键音
            while(!K3); // 等待按键释放
        }
    }
    
    // K4键：加键/显示上限键
    if(K4 == 0) {
        delay_ms(10); // 消抖
        if(K4 == 0) {
            if(mode == 1) { // 设置上限模式
                if(temp_high < 500) temp_high += 10; // 加1°C，最高50.0°C
            } else if(mode == 2) { // 设置下限模式
                if(temp_low < temp_high - 10) temp_low += 10; // 加1°C
            } else { // 正常模式，显示上限温度1秒
                show_high_flag = 1;
                show_counter = 100; // 显示约1秒
            }
            play_key_sound(); // 播放按键音
            while(!K4); // 等待按键释放
        }
    }
    
    // K5键：按键音开关
    if(K5 == 0) {
        delay_ms(10); // 消抖
        if(K5 == 0) {
            key_sound_enabled = !key_sound_enabled; // 切换按键音状态
            // 无论当前状态如何，都播放一次声音作为反馈
            BEEP = 0;
            delay_ms(100);
            BEEP = 1;
            while(!K5); // 等待按键释放
        }
    }
}

// 显示温度函数
void display(uint temp_val, uchar disp_mode) {
    uchar bai, shi;
    uint int_temp = temp_val / 10;  // 获取温度整数部分
    
    bai = int_temp / 10;  // 温度的十位
    shi = int_temp % 10;  // 温度的个位

    // 完全关闭所有数码管
    wei1 = 0;
    wei2 = 0;
    wei3 = 0;
    wei4 = 0;
    P0 = 0x00; // 关闭所有段
    delay(1);

    // 显示十位
    wei1 = 1;		
    wei2 = 0;
    wei3 = 0;
    wei4 = 0;
    
    if(disp_mode == 1) { // 设置上限模式
        P0 = table[bai] | 0x80; // 显示H符号（小数点）
    } else if(disp_mode == 2) { // 设置下限模式
        P0 = table[bai] | 0x80; // 使用小数点来标识
    } else { // 正常温度显示
        P0 = table[bai];
    }
    delay(2);

    // 完全关闭所有数码管
    wei1 = 0;
    wei2 = 0;
    wei3 = 0;
    wei4 = 0;
    P0 = 0x00; // 关闭所有段
    delay(1);

    // 显示个位
    wei1 = 0;		
    wei2 = 1;
    wei3 = 0;
    wei4 = 0;
    P0 = table[shi];
    delay(2);
    
    // 完全关闭所有数码管
    wei1 = 0;
    wei2 = 0;
    wei3 = 0;
    wei4 = 0;
    P0 = 0x00; // 关闭所有段
    delay(1);
    
    // 显示空
    wei1 = 0;		
    wei2 = 0;
    wei3 = 1;
    wei4 = 0;
    P0 = 0x00; // 不显示任何内容
    delay(2);
    
    // 完全关闭所有数码管
    wei1 = 0;
    wei2 = 0;
    wei3 = 0;
    wei4 = 0;
    P0 = 0x00; // 关闭所有段
    delay(1);
    
    // 显示摄氏度符号
    wei1 = 0;		
    wei2 = 0;
    wei3 = 0;
    wei4 = 1;
    P0 = 0x39; // 显示C
    delay(2);
}

// 检查温度报警
void check_alarm(uint current_temp) {
    if(current_temp > temp_high || current_temp < temp_low) {
        BEEP = 0; // 打开蜂鸣器
    } else {
        BEEP = 1; // 关闭蜂鸣器
    }
}

// 串口初始化函数
void init_com(void) {
    TMOD = 0x20;		   // 设T1为方式2
    SCON = 0x50;
    TH1 = 0xFD;
    TL1 = 0xFD;
    TR1 = 1;			   // 开启定时器
    TI = 1;
    EA = 1;			   // 开启总中断
}

// 主函数
void main() {
    uchar a;
    uint temp_value = 0; // 温度
    uint display_temp = 0; // 显示的温度
    
    init_com();
    
    do {
        tmpchange();    // 让18b20开始转换温度
        temp_value = tmp();
        
        // 确保温度在0-50度范围内
        if(temp_value > 500) temp_value = 500; // 限制最高50.0°C
        
        // 检查按键
        check_keys();
        
        // 检查温度报警
        check_alarm(temp_value);
        
        // 决定显示什么温度
        if(show_high_flag && show_counter > 0) {
            display_temp = temp_high; // 显示上限温度
            show_counter--;
            if(show_counter == 0) show_high_flag = 0;
        } else if(show_low_flag && show_counter > 0) {
            display_temp = temp_low; // 显示下限温度
            show_counter--;
            if(show_counter == 0) show_low_flag = 0;
        } else if(mode == 1) {
            display_temp = temp_high; // 设置上限模式，显示上限温度
        } else if(mode == 2) {
            display_temp = temp_low;  // 设置下限模式，显示下限温度
        } else {
            display_temp = temp_value; // 正常模式，显示当前温度
        }
        
        // 显示温度，循环显示一段时间保证稳定
        for(a = 50; a > 0; a--) {
            display(display_temp, mode);
        }
        
        // 串口输出当前温度
        printf("$51,TMS%d#", temp_value / 10);
    } while(1);
}
```

## ESP32 联网代码

```cpp
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

void setup() {
    Serial.begin(115200);
    Serial2.begin(9600);  // 与单片机通信的串口
    
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
}

void loop() {
    server.handleClient();
    
    // 读取串口数据
    while (Serial2.available()) {
        char c = Serial2.read();
        rawData += c;
        
        // 检查是否收到完整的数据包
        if (c == '#') {
            parseSerialData(rawData);
            rawData = "";
        }
        
        if (rawData.length() > 100) {
            rawData = "";
        }
    }
}

// 解析串口数据
void parseSerialData(String data) {
    if (data.startsWith("$51,TMS") && data.endsWith("#")) {
        int tempStart = data.indexOf("TMS") + 3;
        int tempEnd = data.lastIndexOf("#");
        
        if (tempStart < tempEnd) {
            String tempStr = data.substring(tempStart, tempEnd);
            int tempValue = tempStr.toInt();
            
            if (tempValue >= 0 && tempValue <= 500) {
                lastTemperature = String(tempValue / 1.0, 1);
                lastUpdateTime = getCurrentTime();
                dataReceived = true;
            }
        }
    }
}

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
```

## 使用方法

### 1. 硬件搭建

1. 按照电路图连接 51 单片机、DS18B20、数码管、按键和蜂鸣器
2. 连接 ESP32 与 51 单片机的串口
3. 接通 5V 电源

### 2. 烧录程序

1. 使用 Keil 编译 51 单片机程序，生成 HEX 文件
2. 使用 STC-ISP 工具将 HEX 文件烧录到 STC89C52
3. 使用 Arduino IDE 将 ESP32 程序烧录到 ESP32 开发板

### 3. 使用系统

1. 系统启动后，数码管显示当前温度
2. 按 K2 键切换模式（正常/设置上限/设置下限）
3. 按 K3/K4 键调整温度上下限
4. 按 K5 键开关按键音
5. 手机连接 WiFi "Myth"，密码 "12345678"
6. 浏览器访问 ESP32 的 IP 地址查看温度

## 通信协议

### 串口数据格式

```
$51,TMS[温度值]#

示例：
$51,TMS25#  ->  温度 25.0°C
$51,TMS30#  ->  温度 30.0°C
```

### 数据解析

- `$51`：设备标识
- `TMS`：温度数据标识
- `[温度值]`：温度值的 10 倍（用于传输整数）
- `#`：数据结束符

## 扩展功能

1. **数据记录**：将温度数据保存到 SD 卡
2. **远程报警**：温度超限时发送邮件或短信
3. **历史曲线**：网页端显示温度变化曲线
4. **多传感器**：支持多个 DS18B20 同时测量

## 参考文档

- [DS18B20 数据手册](https://www.analog.com/media/en/technical-documentation/data-sheets/ds18b20.pdf)
- [STC89C52 数据手册](http://www.stcmcudata.com/)
- [Keil C51 开发指南](https://www.keil.com/support/man/docs/c51/)

---

**注意**：DS18B20 数据线需要接上拉电阻（4.7KΩ），否则无法正常通信。蜂鸣器报警时请注意音量，避免长时间高分贝声音。
