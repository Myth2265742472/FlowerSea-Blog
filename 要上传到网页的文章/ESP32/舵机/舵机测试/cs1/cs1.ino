#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ========================= 配置参数 =========================
// I2C引脚配置 (ESP32默认)
static const int I2C_SDA = 21;
static const int I2C_SCL = 22;
static const uint32_t I2C_FREQ = 400000; // 400 kHz

// PWM Servo Driver配置
static const uint8_t PWM_SERVO_ADDR = 0x40; // Adafruit PWM Servo Driver默认地址
static const int PWM_FREQ = 55; // 50Hz PWM频率，适合舵机控制

// 舵机配置
static const int NUM_SERVOS = 1; // 控制1个舵机
static const int SERVO_CHANNELS[NUM_SERVOS] = {0}; // 使用PWM通道0
static const int SERVO_MIN_PULSE = 150;  // 最小脉冲宽度 (0.5ms)
static const int SERVO_MAX_PULSE = 600;  // 最大脉冲宽度 (2.5ms)

// 舵机角度范围
static const float SERVO_MIN_ANGLE = 0.0f;
static const float SERVO_MAX_ANGLE = 180.0f;

// 舵机移动速度控制
static const int MOVE_DELAY = 20; // 舵机移动间隔(ms)
static const float MOVE_STEP = 1.0f; // 每次移动的角度步长

// ========================= 全局变量 =========================
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PWM_SERVO_ADDR);

// 当前舵机角度
float currentAngles[NUM_SERVOS] = {90.0f}; // 初始化为90度
float targetAngles[NUM_SERVOS] = {90.0f};  // 目标角度

// 串口命令处理
String inputString = "";
boolean stringComplete = false;

// ========================= 函数声明 =========================
void setupPWMDriver();
void setServoAngle(int servoIndex, float angle);
void moveServoToAngle(int servoIndex, float targetAngle);
void moveAllServosToAngles(float angles[]);
void processSerialCommand();
void printHelp();
void printCurrentAngles();

// ========================= 初始化函数 =========================
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("ESP32 + Adafruit PWM Servo Driver (单舵机) 初始化中...");
  
  // 初始化I2C
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);
  
  // 初始化PWM Servo Driver
  setupPWMDriver();
  
  // 将所有舵机移动到初始位置
  moveAllServosToAngles(currentAngles);
  
  Serial.println("初始化完成！");
  printHelp();
  printCurrentAngles();
}

// ========================= 主循环 =========================
void loop() {
  // 处理串口命令
  processSerialCommand();
  
  // 平滑移动舵机到目标位置
  for (int i = 0; i < NUM_SERVOS; i++) {
    if (abs(currentAngles[i] - targetAngles[i]) > 0.1f) {
      moveServoToAngle(i, targetAngles[i]);
    }
  }
  
  delay(MOVE_DELAY);
}

// ========================= PWM驱动设置 =========================
void setupPWMDriver() {
  if (pwm.begin()) {
    Serial.println("PWM Servo Driver 连接成功！");
  } else {
    Serial.println("PWM Servo Driver 连接失败！请检查接线。");
    while (1) delay(1000);
  }
  
  pwm.setOscillatorFrequency(27000000); // 设置振荡器频率
  pwm.setPWMFreq(PWM_FREQ); // 设置PWM频率为50Hz
  
  Serial.println("PWM频率设置为: " + String(PWM_FREQ) + "Hz");
}

// ========================= 舵机控制函数 =========================
void setServoAngle(int servoIndex, float angle) {
  if (servoIndex < 0 || servoIndex >= NUM_SERVOS) {
    Serial.println("错误：舵机索引超出范围！");
    return;
  }
  
  // 限制角度范围
  if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
  if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;
  
  // 将角度转换为PWM脉冲宽度
  float pulseWidth = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  
  // 设置PWM输出
  pwm.setPWM(SERVO_CHANNELS[servoIndex], 0, pulseWidth);
  
  // 更新当前角度
  currentAngles[servoIndex] = angle;
}

void moveServoToAngle(int servoIndex, float targetAngle) {
  if (servoIndex < 0 || servoIndex >= NUM_SERVOS) return;
  
  float currentAngle = currentAngles[servoIndex];
  float diff = targetAngle - currentAngle;
  
  if (abs(diff) > MOVE_STEP) {
    // 平滑移动
    float newAngle = currentAngle + (diff > 0 ? MOVE_STEP : -MOVE_STEP);
    setServoAngle(servoIndex, newAngle);
  } else {
    // 直接设置到目标角度
    setServoAngle(servoIndex, targetAngle);
  }
}

void moveAllServosToAngles(float angles[]) {
  for (int i = 0; i < NUM_SERVOS; i++) {
    setServoAngle(i, angles[i]);
  }
}

// ========================= 串口命令处理 =========================
void processSerialCommand() {
  if (stringComplete) {
    inputString.trim();
    
    if (inputString.length() > 0) {
      // 解析命令
      if (inputString.startsWith("set")) {
        // 格式: set <angle>
        // 例如: set 90
        int firstSpace = inputString.indexOf(' ');
        
        if (firstSpace > 0) {
          float angle = inputString.substring(firstSpace + 1).toFloat();
          targetAngles[0] = angle;
          Serial.println("设置舵机目标角度: " + String(angle) + "°");
        } else {
          Serial.println("错误：命令格式应为 'set <角度>'");
        }
      }
      else if (inputString.startsWith("move")) {
        // 格式: move <angle>
        // 例如: move 45
        int firstSpace = inputString.indexOf(' ');
        
        if (firstSpace > 0) {
          float angle = inputString.substring(firstSpace + 1).toFloat();
          setServoAngle(0, angle);
          targetAngles[0] = angle;
          Serial.println("移动舵机到角度: " + String(angle) + "°");
        } else {
          Serial.println("错误：命令格式应为 'move <角度>'");
        }
      }
      else if (inputString == "status") {
        printCurrentAngles();
      }
      else if (inputString == "help") {
        printHelp();
      }
      else if (inputString == "center") {
        // 舵机回到中心位置
        targetAngles[0] = 90.0f;
        Serial.println("舵机回到中心位置 (90°)");
      }
      else if (inputString == "test") {
        // 舵机测试序列
        Serial.println("开始舵机测试序列...");
        testSequence();
      }
      else {
        Serial.println("未知命令。输入 'help' 查看可用命令。");
      }
    }
    
    inputString = "";
    stringComplete = false;
  }
}

// ========================= 串口数据接收 =========================
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}

// ========================= 辅助函数 =========================
void printHelp() {
  Serial.println("\n=== ESP32 PWM Servo Driver 单舵机控制命令 ===");
  Serial.println("set <角度>               - 设置舵机目标角度 (0-180°)");
  Serial.println("move <角度>              - 立即移动舵机到指定角度");
  Serial.println("status                   - 显示当前舵机角度");
  Serial.println("center                   - 舵机回到中心位置 (90°)");
  Serial.println("test                     - 运行舵机测试序列");
  Serial.println("help                     - 显示此帮助信息");
  Serial.println("==============================================\n");
}

void printCurrentAngles() {
  Serial.println("当前舵机角度: " + String(currentAngles[0], 1) + "°");
}

void testSequence() {
  Serial.println("测试序列开始...");
  
  // 测试1: 舵机移动到0度
  Serial.println("测试1: 移动到0度");
  setServoAngle(0, 0);
  targetAngles[0] = 0;
  delay(2000);
  
  // 测试2: 舵机移动到90度
  Serial.println("测试2: 移动到90度");
  setServoAngle(0, 90);
  targetAngles[0] = 90;
  delay(2000);
  
  // 测试3: 舵机移动到180度
  Serial.println("测试3: 移动到180度");
  setServoAngle(0, 180);
  targetAngles[0] = 180;
  delay(2000);
  
  // 测试4: 回到中心位置
  Serial.println("测试4: 回到中心位置");
  setServoAngle(0, 90);
  targetAngles[0] = 90;
  
  Serial.println("测试序列完成！");
}
