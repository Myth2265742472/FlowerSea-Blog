#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

float accPitch, accRoll;
float gyroPitch = 0, gyroRoll = 0, gyroYaw = 0;
float pitch = 0, roll = 0, yaw = 0;
float lastTime = 0;
float offsetPitch = 0, offsetRoll = 0;
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;
const int calibCount = 200;
const float alpha = 0.98; // 互补滤波系数

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  // 校准
  long sumPitch = 0, sumRoll = 0;
  long sumGX = 0, sumGY = 0, sumGZ = 0;
  Serial.println("Calibrating... Keep MPU6050 still.");
  for (int i = 0; i < calibCount; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    float aPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / PI;
    float aRoll = atan2(ay, az) * 180 / PI;
    sumPitch += aPitch;
    sumRoll += aRoll;
    sumGX += gx;
    sumGY += gy;
    sumGZ += gz;
    delay(10);
  }
  offsetPitch = sumPitch / (float)calibCount;
  offsetRoll = sumRoll / (float)calibCount;
  gyroOffsetX = sumGX / (float)calibCount;
  gyroOffsetY = sumGY / (float)calibCount;
  gyroOffsetZ = sumGZ / (float)calibCount;
  Serial.println("Calibration done.");

  lastTime = millis() / 1000.0;
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 加速度计角度
  accPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / PI - offsetPitch;
  accRoll = atan2(ay, az) * 180 / PI - offsetRoll;

  // 陀螺仪角速度（deg/s），减去零偏
  float gPitch = (gy - gyroOffsetY) / 131.0;
  float gRoll  = (gx - gyroOffsetX) / 131.0;
  float gYaw   = (gz - gyroOffsetZ) / 131.0;

  // 时间间隔
  float now = millis() / 1000.0;
  float dt = now - lastTime;
  lastTime = now;

  // 互补滤波
  pitch = alpha * (pitch + gPitch * dt) + (1 - alpha) * accPitch;
  roll  = alpha * (roll  + gRoll  * dt) + (1 - alpha) * accRoll;
  yaw   += gYaw * dt; // Yaw只能用陀螺仪积分，易漂移

  // 限制Yaw在-180~180
  if (yaw > 180) yaw -= 360;
  if (yaw < -180) yaw += 360;

  // 输出整数
  Serial.print("P:"); Serial.print((int)pitch);
  Serial.print(" R:"); Serial.print((int)roll);
  Serial.print(" Y:"); Serial.println((int)yaw);

  delay(20); // 50Hz
}