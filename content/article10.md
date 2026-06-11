# ESP32 + Adafruit PWM Servo Driver + MPU6050 姿态同步控制项目

## 项目简介

本项目实现了ESP32微控制器与Adafruit 16路PWM Servo Driver和MPU6050传感器的连接，用于控制1个舵机与MPU6050姿态同步。项目提供了完整的串口命令控制接口，支持手动控制、姿态同步、平滑移动等功能，非常适合姿态控制应用场景。

## 功能特性

- ✅ **姿态同步控制**: 舵机与MPU6050姿态实时同步
- ✅ **双控制模式**: 支持手动控制和姿态同步模式
- ✅ **串口命令接口**: 通过串口发送命令控制舵机
- ✅ **平滑移动**: 舵机平滑移动到目标位置，避免突然跳动
- ✅ **角度限制**: 自动限制舵机角度在0-180度范围内
- ✅ **实时状态**: 实时显示舵机角度和MPU6050姿态数据
- ✅ **测试功能**: 内置舵机测试序列
- ✅ **安全保护**: 包含超时保护和错误处理

## 硬件要求

### 主要组件
- **ESP32开发板** (如ESP32 DevKit V1)
- **Adafruit PWM Servo Driver** (PCA9685)
- **MPU6050传感器模块** (6轴陀螺仪+加速度计)
- **标准PWM舵机** x1 (如SG90, MG996R)
- **外部5V电源** (用于舵机供电)
- **杜邦线** 若干

### 连接图

```
ESP32          PWM Servo Driver    MPU6050    舵机
-----          ----------------    -------    ----
3.3V    ->     VCC                 VCC
GND     ->     GND                 GND    ->  电源负极
GPIO21  ->     SDA
GPIO22  ->     SCL
GPIO19                      ->     SDA
GPIO18                      ->     SCL
                V+ (5V)                    ->  电源正极
                PWM0                       ->  舵机信号线
```

## 软件要求

### Arduino IDE设置
1. 安装ESP32开发板包
2. 安装以下库：
   - Adafruit PWM Servo Driver Library
   - Wire Library (内置)

### 库安装步骤
1. 打开Arduino IDE
2. 工具 → 管理库
3. 搜索并安装以下库：
   - "Adafruit PWM Servo Driver Library"
   - "MPU6050" (by Electronic Cats 或 Jeff Rowberg)

## 使用方法

### 1. 硬件连接
按照连接图连接所有组件，确保：
- 所有设备共地连接
- 舵机使用外部5V电源供电
- PWM驱动板I2C连接 (SDA→GPIO21, SCL→GPIO22)
- MPU6050 I2C连接 (SDA→GPIO19, SCL→GPIO18)
- 使用两个独立的I2C总线避免冲突

### 2. 代码上传
1. 打开 `esp32_servo_driver.ino`
2. 选择正确的ESP32开发板
3. 选择正确的串口
4. 上传代码

### 3. 串口命令控制

打开串口监视器 (115200波特率)，可以使用以下命令：

#### 基本命令
```
help                    - 显示帮助信息
status                  - 显示当前舵机角度
center                  - 舵机回到中心位置 (90°)
test                    - 运行舵机测试序列
```

#### 舵机控制命令
```
set <角度>               - 设置舵机目标角度 (平滑移动)
move <角度>              - 立即移动舵机到指定角度
```

#### MPU6050控制命令
```
mpu                      - 显示MPU6050姿态数据
sync                     - 切换MPU6050同步模式
axis                     - 显示当前映射轴信息
```

#### 命令示例
```
set 45                   # 设置舵机目标角度为45度
move 90                  # 立即移动舵机到90度
center                   # 舵机回到90度
test                     # 运行测试序列
mpu                      # 显示MPU6050姿态数据
sync                     # 切换同步模式
```

## 代码结构

### 主要文件
- `esp32_servo_driver.ino` - 主程序文件
- `config.h` - 配置文件
- `libraries.txt` - 库依赖说明
- `README.md` - 项目说明文档

### 核心功能模块
1. **PWM驱动初始化** - 设置PWM频率和I2C通信
2. **舵机控制** - 角度设置和平滑移动
3. **串口命令处理** - 解析和执行用户命令
4. **状态监控** - 实时显示舵机状态

## 配置参数

### 可调整参数 (在代码中)
```cpp
// 舵机数量
static const int NUM_SERVOS = 1;

// PWM频率
static const int PWM_FREQ = 50; // Hz

// 舵机角度范围
static const float SERVO_MIN_ANGLE = 0.0f;
static const float SERVO_MAX_ANGLE = 180.0f;

// 移动速度控制
static const int MOVE_DELAY = 20; // ms
static const float MOVE_STEP = 1.0f; // 度
```

## 故障排除

### 常见问题

1. **PWM Servo Driver连接失败**
   - 检查I2C连接 (SDA, SCL)
   - 确认电源连接
   - 检查I2C地址是否正确 (默认0x40)

2. **舵机不转动**
   - 检查舵机电源连接
   - 确认PWM信号线连接
   - 检查舵机是否损坏

3. **舵机转动不准确**
   - 调整PWM脉冲宽度范围
   - 检查舵机规格是否匹配
   - 校准舵机中位

4. **串口命令无响应**
   - 检查串口波特率 (115200)
   - 确认命令格式正确
   - 检查串口连接

### 调试技巧
- 使用 `status` 命令查看当前状态
- 使用 `test` 命令验证硬件连接
- 检查串口输出中的错误信息

## 扩展功能

### 可能的改进
1. **Web界面控制** - 添加WiFi和Web服务器
2. **传感器集成** - 添加MPU6050等传感器
3. **动作序列** - 预定义动作序列播放
4. **PID控制** - 更精确的位置控制
5. **多舵机扩展** - 扩展到更多舵机

### 代码扩展示例
```cpp
// 扩展到多个舵机
static const int NUM_SERVOS = 4; // 扩展到4个舵机

// 添加传感器控制
void controlWithSensor() {
    // 根据传感器数据控制舵机
}

// 添加动作序列
void playActionSequence() {
    // 播放预定义的动作序列
}
```

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 贡献

欢迎提交Issue和Pull Request来改进这个项目。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 创建GitHub Issue
- 发送邮件至项目维护者

---

**注意**: 使用舵机时请注意安全，避免舵机卡死或过载。建议在测试时使用较小的舵机，并确保电源能够提供足够的电流。
