#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <MPU6050.h>

// ========================= 配置参数 =========================
// PWM Servo Driver I2C引脚配置
static const int PWM_I2C_SDA = 21;
static const int PWM_I2C_SCL = 22;
static const uint32_t PWM_I2C_FREQ = 400000; // 400 kHz

// MPU6050 I2C引脚配置 (使用不同的引脚)
static const int MPU_I2C_SDA = 19;
static const int MPU_I2C_SCL = 18;
static const uint32_t MPU_I2C_FREQ = 400000; // 400 kHz

// PWM Servo Driver配置
static const uint8_t PWM_SERVO_ADDR = 0x40; // Adafruit PWM Servo Driver默认地址
static const int PWM_FREQ = 50; // 50Hz PWM频率，适合舵机控制

// MPU6050配置
static const uint8_t MPU_ADDR = 0x68; // MPU6050 I2C地址
static const float FILTER_ALPHA = 0.98f; // 互补滤波参数
static const float LOOP_HZ = 100.0f; // 主循环频率

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

// MPU6050姿态控制
static const bool ENABLE_MPU_SYNC = true; // 是否启用MPU6050同步
static const bool MAP_PITCH_TO_SERVO = true; // true: pitch轴映射到舵机, false: roll轴映射
static const bool INVERT_AXIS = false; // 是否反转轴方向
static const float ANGLE_SCALE = 1.0f; // 角度缩放因子

// MPU6050角度限制 (直接映射到舵机0-180度)
static const float MPU_MIN_ANGLE = 0.0f;   // MPU6050最小角度
static const float MPU_MAX_ANGLE = 180.0f; // MPU6050最大角度

// ========================= 全局变量 =========================
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PWM_SERVO_ADDR);
MPU6050 mpu;

// 两个独立的I2C总线
TwoWire PWM_I2C = TwoWire(0);  // PWM Servo Driver使用I2C0
TwoWire MPU_I2C = TwoWire(1);  // MPU6050使用I2C1

// 当前舵机角度
float currentAngles[NUM_SERVOS] = {90.0f}; // 初始化为90度
float targetAngles[NUM_SERVOS] = {90.0f};  // 目标角度

// MPU6050姿态数据
float pitchDeg = 90.0f; // 俯仰角 (绕X轴旋转) - 初始化为90度
float rollDeg = 90.0f;  // 横滚角 (绕Y轴旋转) - 初始化为90度
float yawDeg = 0.0f;    // 偏航角 (绕Z轴旋转)

// 串口命令处理
String inputString = "";
boolean stringComplete = false;

// 控制模式
bool mpuSyncMode = true; // true: MPU6050同步模式, false: 手动控制模式

// ========================= 函数声明 =========================
void setupPWMDriver();
void setupMPU6050();
void setServoAngle(int servoIndex, float angle);
void moveServoToAngle(int servoIndex, float targetAngle);
void moveAllServosToAngles(float angles[]);
void readMPU6050Data();
void calculateAngles();
void updateServoFromMPU();
void processSerialCommand();
void printHelp();
void printCurrentAngles();
void printMPUData();

// ========================= 初始化函数 =========================
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("ESP32 + Adafruit PWM Servo Driver + MPU6050 初始化中...");
  
  // 初始化PWM Servo Driver的I2C总线
  PWM_I2C.begin(PWM_I2C_SDA, PWM_I2C_SCL, PWM_I2C_FREQ);
  
  // 初始化MPU6050的I2C总线
  MPU_I2C.begin(MPU_I2C_SDA, MPU_I2C_SCL, MPU_I2C_FREQ);
  
  // 初始化PWM Servo Driver
  setupPWMDriver();
  
  // 初始化MPU6050
  setupMPU6050();
  
  // 将所有舵机移动到初始位置
  moveAllServosToAngles(currentAngles);
  
  Serial.println("初始化完成！");
  printHelp();
  printCurrentAngles();
}

// ========================= 主循环 =========================
void loop() {
  static uint32_t lastMicros = micros();
  uint32_t now = micros();
  float dt = (now - lastMicros) / 1000000.0f;
  if (dt <= 0.0f) dt = 1.0f / LOOP_HZ;
  lastMicros = now;
  
  // 处理串口命令
  processSerialCommand();
  
  // 读取MPU6050数据并计算角度
  if (ENABLE_MPU_SYNC) {
    readMPU6050Data();
    calculateAngles();
    
    // 根据MPU6050姿态更新舵机
    if (mpuSyncMode) {
      updateServoFromMPU();
    }
  }
  
  // 平滑移动舵机到目标位置
  for (int i = 0; i < NUM_SERVOS; i++) {
    if (abs(currentAngles[i] - targetAngles[i]) > 0.1f) {
      moveServoToAngle(i, targetAngles[i]);
    }
  }
  
  // 控制循环频率
  delayMicroseconds((int)(1000000.0f / LOOP_HZ));
}

// ========================= PWM驱动设置 =========================
void setupPWMDriver() {
  // 切换到PWM驱动板的I2C总线
  Wire = PWM_I2C;
  
  // 初始化PWM驱动板
  if (pwm.begin()) {
    Serial.println("PWM Servo Driver 连接成功！");
  } else {
    Serial.println("PWM Servo Driver 连接失败！请检查接线。");
    while (1) delay(1000);
  }
  
  pwm.setOscillatorFrequency(27000000); // 设置振荡器频率
  pwm.setPWMFreq(PWM_FREQ); // 设置PWM频率为50Hz
  
  Serial.println("PWM频率设置为: " + String(PWM_FREQ) + "Hz");
  Serial.println("PWM I2C引脚: SDA=" + String(PWM_I2C_SDA) + ", SCL=" + String(PWM_I2C_SCL));
}

// ========================= MPU6050设置 =========================
void setupMPU6050() {
  // 由于MPU6050库可能不支持自定义Wire实例，我们使用寄存器级访问
  // 首先切换到MPU6050的I2C总线
  Wire = MPU_I2C;
  
  // 初始化MPU6050
  mpu.initialize();
  
  // 检查MPU6050连接是否成功
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 连接失败！请检查接线。");
    return;
  }
  
  Serial.println("MPU6050 连接成功！");
  
  // 配置MPU6050
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
  
  // 校准延迟
  delay(1000);
  
  Serial.println("MPU6050 初始化完成！");
  Serial.println("MPU I2C引脚: SDA=" + String(MPU_I2C_SDA) + ", SCL=" + String(MPU_I2C_SCL));
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
  
  // 切换到PWM驱动板的I2C总线
  Wire = PWM_I2C;
  
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

// ========================= MPU6050数据处理 =========================
void readMPU6050Data() {
  // 切换到MPU6050的I2C总线
  Wire = MPU_I2C;
  
  // 读取MPU6050原始数据
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // 转换为物理量
  // 加速度计: ±4g → 8192 LSB/g
  float accX = (float)ax / 8192.0f;
  float accY = (float)ay / 8192.0f;
  float accZ = (float)az / 8192.0f;
  
  // 陀螺仪: ±500 dps → 65.5 LSB/(°/s)
  float gyroX = (float)gx / 65.5f;
  float gyroY = (float)gy / 65.5f;
  float gyroZ = (float)gz / 65.5f;
  
  // 计算加速度计角度 (转换为0-180度范围)
  float accelAngleX = atan2(accY, sqrt(accX * accX + accZ * accZ)) * 180.0f / PI;
  float accelAngleY = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0f / PI;
  
  // 将加速度计角度从±90度映射到0-180度
  accelAngleX = (accelAngleX + 90.0f); // -90~90 -> 0~180
  accelAngleY = (accelAngleY + 90.0f); // -90~90 -> 0~180
  
  // 互补滤波
  static float dt = 1.0f / LOOP_HZ;
  
  // 应用互补滤波 (在0-180度范围内)
  pitchDeg = FILTER_ALPHA * (pitchDeg + gyroX * dt) + (1.0f - FILTER_ALPHA) * accelAngleX;
  rollDeg = FILTER_ALPHA * (rollDeg + gyroY * dt) + (1.0f - FILTER_ALPHA) * accelAngleY;
  
  // 限制角度范围到0-180度
  if (pitchDeg > MPU_MAX_ANGLE) pitchDeg = MPU_MAX_ANGLE;
  if (pitchDeg < MPU_MIN_ANGLE) pitchDeg = MPU_MIN_ANGLE;
  if (rollDeg > MPU_MAX_ANGLE) rollDeg = MPU_MAX_ANGLE;
  if (rollDeg < MPU_MIN_ANGLE) rollDeg = MPU_MIN_ANGLE;
  
  // 偏航角处理 (简化)
  yawDeg += gyroZ * dt;
  if (yawDeg > 180.0f) yawDeg -= 360.0f;
  if (yawDeg < -180.0f) yawDeg += 360.0f;
}

void calculateAngles() {
  // 角度计算已在readMPU6050Data中完成
  // 这里可以添加额外的角度处理逻辑
}

void updateServoFromMPU() {
  // 选择要映射的轴
  float mpuAngle = MAP_PITCH_TO_SERVO ? pitchDeg : rollDeg;
  
  // 应用缩放和反转
  if (INVERT_AXIS) {
    mpuAngle = MPU_MAX_ANGLE - mpuAngle; // 在0-180度范围内反转
  }
  mpuAngle *= ANGLE_SCALE;
  
  // 确保角度在0-180度范围内
  if (mpuAngle < MPU_MIN_ANGLE) mpuAngle = MPU_MIN_ANGLE;
  if (mpuAngle > MPU_MAX_ANGLE) mpuAngle = MPU_MAX_ANGLE;
  
  // 直接映射到舵机角度 (0-180度一一对应)
  float targetServoAngle = mpuAngle;
  
  // 更新舵机目标角度
  targetAngles[0] = targetServoAngle;
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
      else if (inputString == "mpu") {
        // 显示MPU6050数据
        printMPUData();
      }
      else if (inputString == "sync") {
        // 切换MPU6050同步模式
        mpuSyncMode = !mpuSyncMode;
        Serial.println("MPU6050同步模式: " + String(mpuSyncMode ? "开启" : "关闭"));
      }
      else if (inputString == "axis") {
        // 切换映射轴
        Serial.println("当前映射轴: " + String(MAP_PITCH_TO_SERVO ? "Pitch" : "Roll"));
        Serial.println("提示: 修改代码中的MAP_PITCH_TO_SERVO来切换轴");
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
  Serial.println("\n=== ESP32 PWM Servo Driver + MPU6050 控制命令 ===");
  Serial.println("set <角度>               - 设置舵机目标角度 (0-180°)");
  Serial.println("move <角度>              - 立即移动舵机到指定角度");
  Serial.println("status                   - 显示当前舵机角度");
  Serial.println("center                   - 舵机回到中心位置 (90°)");
  Serial.println("test                     - 运行舵机测试序列");
  Serial.println("mpu                      - 显示MPU6050姿态数据");
  Serial.println("sync                     - 切换MPU6050同步模式");
  Serial.println("axis                     - 显示当前映射轴信息");
  Serial.println("help                     - 显示此帮助信息");
  Serial.println("================================================\n");
}

void printCurrentAngles() {
  Serial.println("当前舵机角度: " + String(currentAngles[0], 1) + "°");
}

void printMPUData() {
  Serial.println("=== MPU6050 姿态数据 ===");
  Serial.println("Pitch (俯仰): " + String(pitchDeg, 2) + "°");
  Serial.println("Roll (横滚): " + String(rollDeg, 2) + "°");
  Serial.println("Yaw (偏航): " + String(yawDeg, 2) + "°");
  Serial.println("同步模式: " + String(mpuSyncMode ? "开启" : "关闭"));
  Serial.println("映射轴: " + String(MAP_PITCH_TO_SERVO ? "Pitch" : "Roll"));
  Serial.println("========================");
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
