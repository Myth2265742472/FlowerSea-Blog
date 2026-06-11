/*
 * 云台发送端程序 (MPU6050 + ESP-NOW发送)
 * 功能：读取MPU6050姿态数据，通过ESP-NOW发送到接收端
 * 使用纯I2C方式（兼容性更好），保留卡尔曼滤波器和改进功能
 */

#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <math.h>

// ============= 引脚配置 =============
#define I2C_SDA_PIN        17  // I2C数据线
#define I2C_SCL_PIN        18  // I2C时钟线

// ============= ESP-NOW配置 =============
// 接收端的MAC地址（使用广播地址）
uint8_t receiverMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ============= 卡尔曼滤波器参数（平衡版：稳定性与响应速度）============
const float Q_angle = 0.0005;   // 角度过程噪声（降低，提高响应速度）
const float Q_gyro = 0.002;     // 陀螺仪过程噪声（降低，提高响应速度）
const float R_angle = 0.015;    // 测量噪声（降低，提高响应速度）

// ============= MPU6050配置 =============
#define MPU6050_ADDR       0x68
#define MPU6050_WHO_AM_I    0x75
#define MPU6050_PWR_MGMT_1  0x6B
#define MPU6050_GYRO_CONFIG  0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B

// ============= 控制参数 =============
const int BAUD_RATE = 115200;
const int updateInterval = 10;     // 数据发送间隔（毫秒，100Hz，提高响应速度）

// ============= 数据结构 =============
typedef struct struct_message {
  float angleX;  // Roll
  float angleY;  // Pitch
  float angleZ;  // Yaw
} struct_message;

struct_message myData;

// ============= 卡尔曼滤波器变量 =============
float angle_roll = 0, bias_roll = 0;
float P_roll[2][2] = {{0, 0}, {0, 0}};

float angle_pitch = 0, bias_pitch = 0;
float P_pitch[2][2] = {{0, 0}, {0, 0}};

float angle_yaw = 0;

// ============= 归零校准变量 =============
float offsetX = 0;  // Roll偏移量
float offsetY = 0;   // Pitch偏移量

// ============= 陀螺仪零偏校准变量 =============
float gyroBiasX = 0;  // X轴陀螺仪零偏
float gyroBiasY = 0;  // Y轴陀螺仪零偏
float gyroBiasZ = 0;  // Z轴陀螺仪零偏

// ============= 运动检测变量（简化版）============
#define MOTION_THRESHOLD 1.5  // 运动检测阈值（度/秒，单个轴）
bool isMoving = false;
unsigned long lastMotionTime = 0;
#define MOTION_TIMEOUT 500  // 运动超时时间（毫秒）

// ============= Yaw角优化变量 =============
float yawDriftAccumulator = 0;  // 漂移累积器
unsigned long yawLastResetTime = 0;

// ============= 全局变量 =============
unsigned long lastTime = 0;

// ============= 发送状态 =============
bool dataSent = false;

// ============= 接收端peer信息 =============
esp_now_peer_info_t peerInfo;

// ============= ESP-NOW回调 =============
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    dataSent = true;
  } else {
    // 发送失败，静默处理
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  while (!Serial);
  Serial.println("\n\n=== 云台发送端（ESP-NOW） ===");

  // 初始化WiFi (ESP-NOW需要WiFi初始化)
  WiFi.mode(WIFI_STA);

  // 初始化ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW初始化失败");
    return;
  }
  Serial.println("ESP-NOW初始化成功");

  // 注册发送回调
  esp_now_register_send_cb(OnDataSent);

  // 注册接收端 (新版本ESP-NOW不需要角色参数)
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.println("已添加接收端");
  } else {
    Serial.println("添加接收端失败，将在运行时重试");
  }

  // 初始化I2C
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);
  Serial.println("I2C 初始化完成");

  // 检测并初始化MPU6050
  if (checkMPU6050()) {
    Serial.println("MPU6050 检测成功!");
    initMPU6050();
    calibrateGyroBias();  // 先校准陀螺仪零偏
    calibrateOffsets();    // 再校准加速度计偏移
  } else {
    Serial.println("MPU6050 检测失败! 请检查连接");
    while (1) delay(100);
  }

  Serial.println("系统初始化完成!");
}

void loop() {
  unsigned long currentTime = millis();

  // 按固定间隔读取并发送数据
  if (currentTime - lastTime >= updateInterval) {
    lastTime = currentTime;

    // 读取MPU6050数据
    readMPU6050Data();

    // 准备数据
    myData.angleX = angle_roll;
    myData.angleY = angle_pitch;
    myData.angleZ = angle_yaw;

    // 发送数据
    if (sendDataToReceiver()) {
      // 打印调试信息（包含运动状态）
      const char* motionStatus = isMoving ? "运动" : "静止";
      Serial.printf("发送: X=%.1f°, Y=%.1f°, Z=%.1f° [%s]\n",
                    angle_roll, angle_pitch, angle_yaw, motionStatus);
    }
  }
}

// 发送数据到接收端
bool sendDataToReceiver() {
  esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    dataSent = true;
    return true;
  } else {
    Serial.println("ESP-NOW发送失败");
    return false;
  }
}

// 检测MPU6050
bool checkMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_WHO_AM_I);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 1);

  if (Wire.available()) {
    byte whoAmI = Wire.read();
    return (whoAmI == 0x68 || whoAmI == 0x70);
  }
  return false;
}

// 初始化MPU6050
void initMPU6050() {
  // 唤醒MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_PWR_MGMT_1);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(100);

  // 配置陀螺仪 (±500度/秒)
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_GYRO_CONFIG);
  Wire.write(0x08);
  Wire.endTransmission();
  delay(10);

  // 配置加速度计 (±8g)
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_ACCEL_CONFIG);
  Wire.write(0x10);
  Wire.endTransmission();
  delay(10);

  // 配置数字低通滤波器（21Hz带宽，减少噪声）
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x1A);  // DLPF_CFG寄存器
  Wire.write(0x04);  // 21Hz带宽
  Wire.endTransmission();
  delay(10);

  Serial.println("MPU6050 配置完成");

  // 让传感器稳定
  Serial.println("等待MPU6050稳定...");
  delay(2000);

  // 初始化姿态角度（只使用加速度计，多次采样取平均）
  Serial.println("读取初始姿态...");
  readMPU6050Initial();

  // 初始化卡尔曼滤波器状态（设置合理的初始值）
  bias_roll = 0;
  bias_pitch = 0;
  P_roll[0][0] = 1.0;  // 初始协方差矩阵
  P_roll[0][1] = 0.0;
  P_roll[1][0] = 0.0;
  P_roll[1][1] = 1.0;
  P_pitch[0][0] = 1.0;
  P_pitch[0][1] = 0.0;
  P_pitch[1][0] = 0.0;
  P_pitch[1][1] = 1.0;

  // 初始化Yaw角相关变量
  angle_yaw = 0;
  yawDriftAccumulator = 0;
  yawLastResetTime = millis();
  lastMotionTime = millis();

  Serial.printf("初始角度: Roll=%.2f°, Pitch=%.2f°\n", angle_roll, angle_pitch);
}

// 读取MPU6050初始角度（多次采样取平均）
void readMPU6050Initial() {
  float sumRoll = 0, sumPitch = 0;
  const int samples = 50;

  for (int i = 0; i < samples; i++) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(MPU6050_ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 14);

    if (Wire.available() >= 14) {
      int16_t rawAccelX = (Wire.read() << 8) | Wire.read();
      int16_t rawAccelY = (Wire.read() << 8) | Wire.read();
      int16_t rawAccelZ = (Wire.read() << 8) | Wire.read();
      // 跳过温度和陀螺仪数据
      for (int j = 0; j < 8; j++) Wire.read();

      // 转换加速度数据（±8g，所以除以4096）
      float accelX = rawAccelX / 4096.0;
      float accelY = rawAccelY / 4096.0;
      float accelZ = rawAccelZ / 4096.0;

      // 计算角度
      sumRoll += atan2(accelY, accelZ) * 180.0 / PI;
      sumPitch += atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;
    }

    delay(5);  // 采样间隔
  }

  // 计算平均初始角度
  angle_roll = sumRoll / samples;
  angle_pitch = sumPitch / samples;

  // 限制初始角度范围
  if (angle_roll > 90) angle_roll = 90;
  if (angle_roll < -90) angle_roll = -90;
  if (angle_pitch > 90) angle_pitch = 90;
  if (angle_pitch < -90) angle_pitch = -90;
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
    // 跳过温度
    int16_t temp = (Wire.read() << 8) | Wire.read();
    int16_t rawGyroX = (Wire.read() << 8) | Wire.read();
    int16_t rawGyroY = (Wire.read() << 8) | Wire.read();
    int16_t rawGyroZ = (Wire.read() << 8) | Wire.read();

    // 转换加速度数据（±8g，所以除以4096）
    float accelX = rawAccelX / 4096.0;
    float accelY = rawAccelY / 4096.0;
    float accelZ = rawAccelZ / 4096.0;

    // 转换陀螺仪数据（±500度/秒，所以除以65.5）并应用零偏校准
    float gyroX = rawGyroX / 65.5 - gyroBiasX;
    float gyroY = rawGyroY / 65.5 - gyroBiasY;
    float gyroZ = rawGyroZ / 65.5 - gyroBiasZ;

    // 计算时间间隔（秒）
    static unsigned long lastReadTime = 0;
    unsigned long now = millis();
    float dt = (now - lastReadTime) / 1000.0;
    lastReadTime = now;

    // 防止异常的时间间隔（更严格的限制）
    if (dt <= 0.005 || dt > 0.1) {  // 5ms到100ms之间
      dt = 0.02;  // 使用默认时间间隔
    }

    // 从加速度计计算角度
    float roll_acc = atan2(accelY, accelZ) * 180.0 / PI;
    float pitch_acc = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;

    // 使用卡尔曼滤波器更新姿态角
    kalmanUpdate(angle_roll, bias_roll, P_roll, roll_acc, gyroX, dt);
    kalmanUpdate(angle_pitch, bias_pitch, P_pitch, pitch_acc, gyroY, dt);

    // 运动检测：简化版，任意轴超过阈值即认为运动
    if (abs(gyroX) > MOTION_THRESHOLD || abs(gyroY) > MOTION_THRESHOLD || abs(gyroZ) > MOTION_THRESHOLD) {
      isMoving = true;
      lastMotionTime = millis();
    } else {
      // 超过超时时间，认为静止
      if (millis() - lastMotionTime > MOTION_TIMEOUT) {
        isMoving = false;
      }
    }

    // Yaw角使用陀螺仪积分（简化版，提高响应速度）
    if (abs(gyroZ) > 0.3) {  // 死区阈值，过滤小噪声
      angle_yaw += gyroZ * dt;
    }

    // 静止时，每3秒校正一次漂移（简化逻辑）
    if (!isMoving && millis() - yawLastResetTime > 3000) {
      angle_yaw -= yawDriftAccumulator * 0.3;  // 校正30%的漂移
      yawDriftAccumulator = 0;
      yawLastResetTime = millis();
    } else {
      yawDriftAccumulator += gyroZ * dt;
    }

    // 限制yaw角度在-180到180度之间
    if (angle_yaw > 180.0) angle_yaw -= 360.0;
    if (angle_yaw < -180.0) angle_yaw += 360.0;

    // 应用校准偏移量
    angle_roll -= offsetX;
    angle_pitch -= offsetY;

    // 限制范围
    if (angle_roll > 90) angle_roll = 90;
    if (angle_roll < -90) angle_roll = -90;
    if (angle_pitch > 90) angle_pitch = 90;
    if (angle_pitch < -90) angle_pitch = -90;
  }
}

// 卡尔曼滤波器更新函数
void kalmanUpdate(float &angle, float &bias, float P[2][2],
                  float newAngle, float newRate, float dt) {
  // 预测步骤
  angle += dt * (newRate - bias);
  P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
  P[0][1] -= dt * P[1][1];
  P[1][0] -= dt * P[1][1];
  P[1][1] += Q_gyro * dt;

  // 更新步骤
  float S = P[0][0] + R_angle;
  float K0 = P[0][0] / S;
  float K1 = P[1][0] / S;
  float y = newAngle - angle;
  angle += K0 * y;
  bias += K1 * y;

  float P00_temp = P[0][0];
  float P01_temp = P[0][1];
  P[0][0] -= K0 * P00_temp;
  P[0][1] -= K0 * P01_temp;
  P[1][0] -= K1 * P00_temp;
  P[1][1] -= K1 * P01_temp;
}

// 陀螺仪零偏校准
void calibrateGyroBias() {
  Serial.println("正在进行陀螺仪零偏校准...");
  Serial.println("请保持MPU6050完全静止！");
  Serial.println("2秒后开始...");

  delay(2000);

  float sumX = 0, sumY = 0, sumZ = 0;
  const int samples = 300;  // 减少采样次数，加快启动（保持足够精度）

  Serial.printf("采样中... (%d次)\n", samples);

  for (int i = 0; i < samples; i++) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(MPU6050_ACCEL_XOUT_H + 8);  // 直接读取陀螺仪数据
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 6);

    if (Wire.available() >= 6) {
      int16_t rawGyroX = (Wire.read() << 8) | Wire.read();
      int16_t rawGyroY = (Wire.read() << 8) | Wire.read();
      int16_t rawGyroZ = (Wire.read() << 8) | Wire.read();

      // 转换为度/秒
      sumX += rawGyroX / 65.5;
      sumY += rawGyroY / 65.5;
      sumZ += rawGyroZ / 65.5;
    }

    delay(2);  // 采样间隔

    if ((i + 1) % 100 == 0) {
      Serial.printf("  采样进度: %d/%d\n", i + 1, samples);
    }
  }

  gyroBiasX = sumX / samples;
  gyroBiasY = sumY / samples;
  gyroBiasZ = sumZ / samples;

  Serial.printf("陀螺仪零偏校准完成！X=%.4f°/s, Y=%.4f°/s, Z=%.4f°/s\n",
                gyroBiasX, gyroBiasY, gyroBiasZ);
  Serial.println();
}

// 归零校准
void calibrateOffsets() {
  Serial.println("正在进行归零校准...");
  Serial.println("请保持MPU6050水平静止！");
  Serial.println("3秒后开始...");

  delay(3000);

  float sumX = 0, sumY = 0;
  const int samples = 100;

  Serial.printf("采样中... (%d次)\n", samples);

  for (int i = 0; i < samples; i++) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(MPU6050_ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 14);

    if (Wire.available() >= 14) {
      int16_t rawAccelX = (Wire.read() << 8) | Wire.read();
      int16_t rawAccelY = (Wire.read() << 8) | Wire.read();
      int16_t rawAccelZ = (Wire.read() << 8) | Wire.read();
      // 跳过其余数据
      for (int j = 0; j < 8; j++) Wire.read();

      float accelX = rawAccelX / 4096.0;
      float accelY = rawAccelY / 4096.0;
      float accelZ = rawAccelZ / 4096.0;

      float roll_acc = atan2(accelY, accelZ) * 180.0 / PI;
      float pitch_acc = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;

      // 限制范围，避免异常值
      if (roll_acc > 90) roll_acc = 90;
      if (roll_acc < -90) roll_acc = -90;
      if (pitch_acc > 90) pitch_acc = 90;
      if (pitch_acc < -90) pitch_acc = -90;

      sumX += roll_acc;
      sumY += pitch_acc;

      if ((i + 1) % 20 == 0) {
        Serial.printf("  采样进度: %d/%d\n", i + 1, samples);
      }
    }

    delay(10);
  }

  offsetX = sumX / samples;
  offsetY = sumY / samples;

  Serial.printf("校准完成！偏移量: X=%.2f, Y=%.2f\n", offsetX, offsetY);

  // 应用校准到初始角度
  angle_roll -= offsetX;
  angle_pitch -= offsetY;

  Serial.printf("校准后角度: Roll=%.2f°, Pitch=%.2f°\n", angle_roll, angle_pitch);
  Serial.println();
}
