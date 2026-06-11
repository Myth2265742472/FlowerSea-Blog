#ifndef CONFIG_H
#define CONFIG_H

// ========================= ESP32 配置 =========================
// I2C引脚配置
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_FREQUENCY 400000  // 400 kHz

// 串口配置
#define SERIAL_BAUD_RATE 115200

// ========================= PWM Servo Driver 配置 =========================
// I2C地址 (可通过跳线修改)
#define PWM_SERVO_ADDRESS 0x40

// PWM频率 (Hz) - 50Hz适合标准舵机
#define PWM_FREQUENCY 50

// 振荡器频率 (Hz) - PCA9685内部振荡器频率
#define OSCILLATOR_FREQUENCY 27000000

// ========================= 舵机配置 =========================
// 舵机数量
#define NUM_SERVOS 1

// PWM通道分配 (0-15)
#define SERVO_CHANNELS {0}

// 舵机PWM脉冲宽度范围 (12位分辨率: 0-4095)
#define SERVO_MIN_PULSE 150   // 0.5ms (150/4096 * 20ms = 0.73ms)
#define SERVO_MAX_PULSE 600   // 2.5ms (600/4096 * 20ms = 2.93ms)

// 舵机角度范围
#define SERVO_MIN_ANGLE 0.0f
#define SERVO_MAX_ANGLE 180.0f

// 舵机移动控制
#define MOVE_DELAY_MS 20      // 舵机移动间隔
#define MOVE_STEP_DEG 1.0f    // 每次移动的角度步长

// ========================= 调试配置 =========================
// 是否启用调试输出
#define DEBUG_ENABLED true

// 状态输出间隔 (ms)
#define STATUS_PRINT_INTERVAL 1000

// ========================= 安全配置 =========================
// 最大角度变化速度 (度/秒)
#define MAX_ANGLE_VELOCITY 90.0f

// 舵机保护超时 (ms) - 防止舵机卡死
#define SERVO_TIMEOUT_MS 5000

// ========================= 高级配置 =========================
// 是否启用平滑移动
#define SMOOTH_MOVEMENT_ENABLED true

// 是否启用角度限制
#define ANGLE_LIMITS_ENABLED true

// 是否启用串口命令
#define SERIAL_COMMANDS_ENABLED true

// 命令缓冲区大小
#define COMMAND_BUFFER_SIZE 64

#endif // CONFIG_H
