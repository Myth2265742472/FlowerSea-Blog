#include <Wire.h>
#include <MPU6050.h>
#include <Servo.h>
#include <math.h>

// 定义舵机引脚
#define SERVO_PIN 13

// 创建MPU6050对象
MPU6050 mpu;

// 创建舵机对象
Servo myServo;

// 定义变量
int16_t ax, ay, az;
int16_t gx, gy, gz;
int angle = 90; // 初始舵机角度
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;

// 卡尔曼滤波器类
class KalmanFilter {
public:
  KalmanFilter(float Q_angle, float Q_gyro, float R_angle, float dt)
    : Q_angle(Q_angle), Q_gyro(Q_gyro), R_angle(R_angle), dt(dt),
      angle(0.0f), bias(0.0f), rate(0.0f),
      P[2][2] = {{1, 0}, {0, 1}} {};

  float update(float newAngle, float newRate) {
    // 预测步骤
    rate = newRate - bias;
    angle += dt * rate;

    // 更新误差协方差矩阵
    P[0][0] += dt * (dt*P[1][1] - P[0][1] - P[1][0] + Q_angle);
    P[0][1] -= dt * P[1][1];
    P[1][0] -= dt * P[1][1];
    P[1][1] += Q_gyro * dt;

    // 计算卡尔曼增益
    float S = P[0][0] + R_angle; // 估计误差
    float K[2]; // 卡尔曼增益
    K[0] = P[0][0] / S;
    K[1] = P[1][0] / S;

    // 更新估计值
    float y = newAngle - angle; // 测量残差
    angle += K[0] * y;
    bias += K[1] * y;

    // 更新误差协方差矩阵
    float P00_temp = P[0][0];
    float P01_temp = P[0][1];

    P[0][0] -= K[0] * P00_temp;
    P[0][1] -= K[0] * P01_temp;
    P[1][0] -= K[1] * P00_temp;
    P[1][1] -= K[1] * P01_temp;

    return angle;
  }

private:
  float Q_angle; // 角度测量噪声协方差
  float Q_gyro;  // 角速度测量噪声协方差
  float R_angle; // 测量噪声协方差
  float dt;      // 采样时间间隔
  float angle;   // 估计角度
  float bias;    // 角速度偏差
  float rate;    // 无偏角速度
  float P[2][2]; // 误差协方差矩阵
};

// 创建卡尔曼滤波器对象
// 参数: Q_angle, Q_gyro, R_angle, dt
float dt = 0.05; // 采样时间（秒），对应50ms延迟
KalmanFilter kalmanX(0.02, 0.02, 0.5, dt);

// 延时补偿相关变量
unsigned long prevTime = 0;
float delayCompensation = 0;
float filteredAngle = 0;

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  
  // 等待串口连接
  while (!Serial) {
    ; // 对于基于Leonardo/Micro/Zero的板，等待串口连接
  }
  
  // 初始化I2C通信
  Wire.begin();
  
  // 初始化MPU6050
  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  
  // 初始化舵机
  myServo.attach(SERVO_PIN);
  myServo.write(angle); // 设置初始角度
  
  // 校准MPU6050
  mpu.calibrateGyro();
  mpu.setThreshold(3);
  
  Serial.println("MPU6050 and Servo initialized successfully!");
  Serial.println("Kalman Filter and Delay Compensation are enabled");
}

void loop() {
  // 计算实际的采样时间间隔（用于延时补偿）
  unsigned long currentTime = millis();
  float actualDt = (currentTime - prevTime) / 1000.0f; // 转换为秒
  prevTime = currentTime;
  
  // 读取MPU6050传感器数据
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // 转换为实际加速度值（单位：g）
  accelX = ax / 16384.0;
  accelY = ay / 16384.0;
  accelZ = az / 16384.0;
  
  // 转换为实际角速度值（单位：度/秒）
  // 注意：这里需要根据MPU6050的量程进行调整
  // 我们使用的是2000DPS量程，所以除以16.4
  gyroX = gx / 16.4;
  gyroY = gy / 16.4;
  gyroZ = gz / 16.4;
  
  // 计算基于加速度的倾斜角度
  float accelAngle = atan2(accelY, sqrt(accelX*accelX + accelZ*accelZ)) * 180 / PI;
  
  // 使用卡尔曼滤波器滤波角度数据
  // 在首次运行时初始化角度
  static bool firstRun = true;
  if (firstRun) {
    // 初始化卡尔曼滤波器的角度
    for (int i = 0; i < 10; i++) {
      filteredAngle = kalmanX.update(accelAngle, gyroX);
    }
    firstRun = false;
  } else {
    // 正常更新卡尔曼滤波
    filteredAngle = kalmanX.update(accelAngle, gyroX);
  }
  
  // 延时补偿算法
  // 预测当前实际角度（考虑卡尔曼滤波和处理时间的延迟）
  // 这里使用角速度来预测当前时刻的实际角度
  float compensatedAngle = filteredAngle + gyroX * delayCompensation;
  
  // 动态调整延时补偿参数
  // 这里根据实际的处理时间来调整补偿量
  // 处理时间包括传感器读取、计算和通信等
  delayCompensation = constrain(actualDt * 1.5, 0.01, 0.1); // 限制在合理范围内
  
  // 将补偿后的角度映射到舵机的0-180度范围
  angle = map(compensatedAngle, -90, 90, 0, 180);
  
  // 确保角度在合法范围内
  angle = constrain(angle, 0, 180);
  
  // 控制舵机转动
  myServo.write(angle);
  
  // 打印数据到串口，用于调试
  Serial.print("Raw Angle: ");
  Serial.print(accelAngle);
  Serial.print(" Filtered Angle: ");
  Serial.print(filteredAngle);
  Serial.print(" Compensated Angle: ");
  Serial.print(compensatedAngle);
  Serial.print(" Servo Angle: ");
  Serial.print(angle);
  Serial.print(" DT: ");
  Serial.print(actualDt);
  Serial.print(" Compensation: ");
  Serial.println(delayCompensation);
  
  // 小延时，减少CPU负载，但不过度影响实时性
  delay(5);
}