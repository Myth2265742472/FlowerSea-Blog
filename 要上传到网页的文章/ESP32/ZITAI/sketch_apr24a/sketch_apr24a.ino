#include <Wire.h>
#include <MPU6050.h>
#include <SimpleKalmanFilter.h>

MPU6050 mpu;

// 卡尔曼滤波器实例
SimpleKalmanFilter kalmanX(1, 1, 0.01);
SimpleKalmanFilter kalmanY(1, 1, 0.01);

// 变量用于存储加速度和陀螺仪数据
int16_t ax, ay, az;
int16_t gx, gy, gz;

// 变量用于存储角度和时间
float angleX = 0;
float angleY = 0;
unsigned long lastTime;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  // 检查连接是否成功
  if (!mpu.testConnection()) {
    Serial.println("MPU6050连接失败");
    while (1);
  }
  Serial.println("MPU6050连接成功");

  // 初始化时间
  lastTime = millis();
}

void loop() {
  // 读取加速度和陀螺仪数据
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 获取当前时间
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0; // 转换为秒
  lastTime = currentTime;

  // 计算x轴和y轴加速度（单位：g）
  float accelX = ax / 16384.0; // 假设±2g范围
  float accelY = ay / 16384.0;
  float accelZ = az / 16384.0;

  // 计算x轴和y轴角速度（单位：度/秒）
  float gyroX = gx / 131.0; // 假设±250度/秒范围
  float gyroY = gy / 131.0;

  // 通过积分计算角度
  angleX += gyroX * dt;
  angleY += gyroY * dt;

  // 计算加速度计的角度
  float accelAngleX = atan2(accelY, accelZ) * 180 / PI;
  float accelAngleY = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180 / PI;

  // 卡尔曼滤波更新
  float filteredAngleX = kalmanX.updateEstimate(accelAngleX);
  float filteredAngleY = kalmanY.updateEstimate(accelAngleY);

  // 更新角度估计（结合陀螺仪数据）
  angleX = filteredAngleX;
  angleY = filteredAngleY;

  // 打印结果，格式为：angleX,angleY
  Serial.print(angleX);
  Serial.print(",");
  Serial.println(angleY);

  delay(20); // 延迟20毫秒（约50Hz）
}