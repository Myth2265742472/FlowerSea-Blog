/*
 * ESP32-S3 MPU6050 舵机同步控制项目
 * 硬件连接：
 * - ESP32-S3-N16R8
 * - MPU6050:
 *   VCC -> 3.3V
 *   GND -> GND
 *   SCL -> GPIO6 (或指定引脚)
 *   SDA -> GPIO5 (或指定引脚)
 *   INT -> 不使用
 * - SG90 舵机:
 *   红线(VCC) -> 5V (或3.3V，取决于舵机型号)
 *   棕线(GND) -> GND
 *   橙线(信号) -> GPIO7 (或指定引脚)
 */

#include <Wire.h>
#include <ESP32Servo.h>

// 引脚定义
#define MPU6050_SDA_PIN 5
#define MPU6050_SCL_PIN 6
#define SERVO_PIN 7

// MPU6050 I2C地址
#define MPU6050_ADDR 0x68

// MPU6050寄存器地址
#define MPU6050_WHO_AM_I 0x75
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_ACCEL_XOUT_L 0x3C
#define MPU6050_ACCEL_YOUT_H 0x3D
#define MPU6050_ACCEL_YOUT_L 0x3E
#define MPU6050_ACCEL_ZOUT_H 0x3F
#define MPU6050_ACCEL_ZOUT_L 0x40
#define MPU6050_TEMP_OUT_H 0x41
#define MPU6050_TEMP_OUT_L 0x42
#define MPU6050_GYRO_XOUT_H 0x43
#define MPU6050_GYRO_XOUT_L 0x44
#define MPU6050_GYRO_YOUT_H 0x45
#define MPU6050_GYRO_YOUT_L 0x46
#define MPU6050_GYRO_ZOUT_H 0x47
#define MPU6050_GYRO_ZOUT_L 0x48

// 舵机对象
Servo myservo;

// 全局变量
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;
float angleX, angleY, angleZ;
float offsetX = 0, offsetY = 0, offsetZ = 0;

// 用于平滑处理
#define FILTER_SAMPLES 5
float angleX_history[FILTER_SAMPLES];
float angleY_history[FILTER_SAMPLES];
float angleZ_history[FILTER_SAMPLES];
int filter_index = 0;

// 上次更新时间
unsigned long lastTime = 0;
const int updateInterval = 50; // 更新间隔(ms)

void setup() {
  // 初始化串口
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n=== MPU6050 舵机同步控制系统 ===");
  
  // 初始化I2C
  Wire.begin(MPU6050_SDA_PIN, MPU6050_SCL_PIN);
  Serial.println("I2C 初始化完成");
  
  // 检测MPU6050
  if (checkMPU6050()) {
    Serial.println("MPU6050 检测成功!");
  } else {
    Serial.println("MPU6050 检测失败! 请检查连接");
    while (1) delay(100);
  }
  
  // 初始化MPU6050
  initMPU6050();
  
  // 初始化舵机
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50); // 标准舵机频率50Hz
  myservo.attach(SERVO_PIN, 500, 2400); // 最小500us，最大2400us
  
  // 归零校准
  Serial.println("正在进行归零校准...");
  calibrateOffsets();
  
  // 初始化历史数据
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    angleX_history[i] = 0;
    angleY_history[i] = 0;
    angleZ_history[i] = 0;
  }
  
  // 将舵机移到初始位置
  myservo.write(90);
  delay(1000);
  
  Serial.println("系统初始化完成!");
  Serial.println("开始运行...");
}

void loop() {
  unsigned long currentTime = millis();
  
  // 按固定间隔更新
  if (currentTime - lastTime >= updateInterval) {
    lastTime = currentTime;
    
    // 读取MPU6050数据
    readMPU6050Data();
    
    // 计算角度
    calculateAngles();
    
    // 平滑滤波
    smoothAngles();
    
    // 映射角度到舵机范围 (0-180度)
    int servoAngle = mapAngleToServo(angleX);
    
    // 控制舵机
    myservo.write(servoAngle);
    
    // 输出数据
    printData(servoAngle);
  }
}

// 检测MPU6050是否存在
bool checkMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_WHO_AM_I);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 1);
  
  if (Wire.available()) {
    byte whoAmI = Wire.read();
    Serial.printf("WHO_AM_I: 0x%02X\n", whoAmI);
    return (whoAmI == 0x68 || whoAmI == 0x70);
  }
  
  return false;
}

// 初始化MPU6050
void initMPU6050() {
  // 唤醒MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_PWR_MGMT_1);
  Wire.write(0x00); // 取消睡眠模式，选择内部8MHz振荡器
  Wire.endTransmission();
  
  delay(100);
  
  // 配置陀螺仪 (±500度/秒)
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_GYRO_CONFIG);
  Wire.write(0x08); // 00001000: ±500°/s
  Wire.endTransmission();
  
  delay(10);
  
  // 配置加速度计 (±4g)
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_ACCEL_CONFIG);
  Wire.write(0x08); // 00001000: ±4g
  Wire.endTransmission();
  
  delay(10);
  
  Serial.println("MPU6050 配置完成");
}

// 读取MPU6050数据
void readMPU6050Data() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 14);
  
  if (Wire.available() >= 14) {
    int16_t rawAccelX = (Wire.read() << 8) | Wire.read();
    int16_t rawAccelY = (Wire.read() << 8) | Wire.read();
    int16_t rawAccelZ = (Wire.read() << 8) | Wire.read();
    int16_t rawTemp = (Wire.read() << 8) | Wire.read();
    int16_t rawGyroX = (Wire.read() << 8) | Wire.read();
    int16_t rawGyroY = (Wire.read() << 8) | Wire.read();
    int16_t rawGyroZ = (Wire.read() << 8) | Wire.read();
    
    // 转换加速度数据 (±4g)
    accelX = rawAccelX / 8192.0;
    accelY = rawAccelY / 8192.0;
    accelZ = rawAccelZ / 8192.0;
    
    // 转换陀螺仪数据 (±500度/秒)
    gyroX = rawGyroX / 65.5;
    gyroY = rawGyroY / 65.5;
    gyroZ = rawGyroZ / 65.5;
  }
}

// 计算角度 (使用加速度计计算倾角)
void calculateAngles() {
  // 使用加速度计计算X轴和Y轴的角度 (简单方法)
  angleX = atan2(accelY, accelZ) * 180.0 / PI;
  angleY = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;
  angleZ = 0; // Z轴偏航角无法仅用加速度计获得
  
  // 应用偏移量
  angleX -= offsetX;
  angleY -= offsetY;
  
  // 限制角度范围
  if (angleX > 90) angleX = 90;
  if (angleX < -90) angleX = -90;
  if (angleY > 90) angleY = 90;
  if (angleY < -90) angleY = -90;
}

// 平滑滤波
void smoothAngles() {
  angleX_history[filter_index] = angleX;
  angleY_history[filter_index] = angleY;
  angleZ_history[filter_index] = angleZ;
  
  filter_index = (filter_index + 1) % FILTER_SAMPLES;
  
  // 计算平均值
  float sumX = 0, sumY = 0, sumZ = 0;
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    sumX += angleX_history[i];
    sumY += angleY_history[i];
    sumZ += angleZ_history[i];
  }
  
  angleX = sumX / FILTER_SAMPLES;
  angleY = sumY / FILTER_SAMPLES;
  angleZ = sumZ / FILTER_SAMPLES;
}

// 映射角度到舵机角度 (0-180度)
int mapAngleToServo(float angle) {
  // 将MPU6050的X轴角度 (-90到90度) 映射到舵机角度 (0到180度)
  int servoAngle = (int)((angle + 90) * 180.0 / 180.0);
  
  // 限制范围
  if (servoAngle < 0) servoAngle = 0;
  if (servoAngle > 180) servoAngle = 180;
  
  return servoAngle;
}

// 归零校准
void calibrateOffsets() {
  float sumX = 0, sumY = 0, sumZ = 0;
  const int samples = 100;
  
  for (int i = 0; i < samples; i++) {
    readMPU6050Data();
    calculateAngles();
    sumX += angleX;
    sumY += angleY;
    delay(10);
  }
  
  offsetX = sumX / samples;
  offsetY = sumY / samples;
  
  Serial.printf("校准偏移量: X=%.2f, Y=%.2f\n", offsetX, offsetY);
}

// 打印数据
void printData(int servoAngle) {
  Serial.printf("MPU6050: X=%.1f°, Y=%.1f°, Z=%.1f° | 舵机: %d°\n", 
                angleX, angleY, angleZ, servoAngle);
}
