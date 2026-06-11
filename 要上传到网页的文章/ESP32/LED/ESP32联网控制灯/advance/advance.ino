#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

Adafruit_MPU6050 mpu;

// 卡尔曼滤波参数
float Q_angle = 0.001; // 角度过程噪声协方差
float Q_gyro  = 0.003; // 角速度过程噪声协方差
float R_angle = 0.03;  // 观测噪声协方差

// roll
float angle_roll = 0, bias_roll = 0, P_roll[2][2] = {{0,0},{0,0}};
// pitch
float angle_pitch = 0, bias_pitch = 0, P_pitch[2][2] = {{0,0},{0,0}};
// yaw
float angle_yaw = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // 初始化roll/pitch
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  angle_roll = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  angle_pitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;
  angle_yaw = 0;
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  static unsigned long lastTime = 0;
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;
  if (dt <= 0 || dt > 1) dt = 0.02; // 防止异常

  // 1. 角度观测值
  float roll_acc  = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  float pitch_acc = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;

  // 2. 卡尔曼滤波roll
  kalmanUpdate(angle_roll, bias_roll, P_roll, roll_acc, g.gyro.x * 180 / PI, dt);

  // 3. 卡尔曼滤波pitch
  kalmanUpdate(angle_pitch, bias_pitch, P_pitch, pitch_acc, g.gyro.y * 180 / PI, dt);

  // 4. yaw仅积分
  angle_yaw += g.gyro.z * 180 / PI * dt;

  // 5. 输出
  Serial.print(angle_roll); Serial.print(",");
  Serial.print(angle_pitch); Serial.print(",");
  Serial.print(angle_yaw); Serial.print(",");
  Serial.println(dt);

  delay(20);
}

// 卡尔曼滤波更新函数
void kalmanUpdate(float &angle, float &bias, float P[2][2], float newAngle, float newRate, float dt) {
  // 预测
  angle += dt * (newRate - bias);
  P[0][0] += dt * (dt*P[1][1] - P[0][1] - P[1][0] + Q_angle);
  P[0][1] -= dt * P[1][1];
  P[1][0] -= dt * P[1][1];
  P[1][1] += Q_gyro * dt;

  // 更新
  float S = P[0][0] + R_angle;
  float K0 = P[0][0] / S;
  float K1 = P[1][0] / S;
  float y = newAngle - angle;
  angle += K0 * y;
  bias  += K1 * y;
  float P00_temp = P[0][0];
  float P01_temp = P[0][1];
  P[0][0] -= K0 * P00_temp;
  P[0][1] -= K0 * P01_temp;
  P[1][0] -= K1 * P00_temp;
  P[1][1] -= K1 * P01_temp;
}