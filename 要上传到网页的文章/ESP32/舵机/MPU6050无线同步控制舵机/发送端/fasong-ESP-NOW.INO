/*
 * ESP32-S3 MPU6050 ESP-NOW发送端
 * 功能: 读取MPU6050数据并通过ESP-NOW发送
 *
 * 硬件连接:
 * - ESP32-S3-N16R8
 * - MPU6050:
 *   VCC -> 3.3V
 *   GND -> GND
 *   SCL -> GPIO6
 *   SDA -> GPIO5
 */

#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

// ESP-NOW配置
uint8_t receiverMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // 接收端MAC地址

// 数据结构
typedef struct struct_message {
  float angleX;
  float angleY;
  float angleZ;
} struct_message;

struct_message myData;

// MPU6050配置
#define MPU6050_ADDR 0x68
#define MPU6050_SDA_PIN 5
#define MPU6050_SCL_PIN 6

// MPU6050寄存器地址
#define MPU6050_WHO_AM_I 0x75
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B

// 全局变量
float angleX = 0, angleY = 0, angleZ = 0;
float offsetX = 0, offsetY = 0;
unsigned long lastTime = 0;
const int updateInterval = 30; // 约33Hz更新率

// 发送状态
bool dataSent = false;

// 接收端peer信息
esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n=== MPU6050 ESP-NOW发送端 ===");

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
    Serial.println("添加接收端失败");
  }

  // 初始化I2C
  Wire.begin(MPU6050_SDA_PIN, MPU6050_SCL_PIN);
  Serial.println("I2C 初始化完成");

  // 检测并初始化MPU6050
  if (checkMPU6050()) {
    Serial.println("MPU6050 检测成功!");
    initMPU6050();
    calibrateOffsets();
  } else {
    Serial.println("MPU6050 检测失败! 请检查连接");
    while (1) delay(100);
  }

  Serial.println("系统初始化完成!");
  Serial.println("等待接收端MAC地址...");
}

void loop() {
  unsigned long currentTime = millis();

  // 按固定间隔读取并发送数据
  if (currentTime - lastTime >= updateInterval) {
    lastTime = currentTime;

    // 读取MPU6050数据
    readMPU6050Data();

    // 准备数据
    myData.angleX = angleX;
    myData.angleY = angleY;
    myData.angleZ = angleZ;

    // 发送数据
    if (sendDataToReceiver()) {
      // 打印调试信息
      Serial.printf("发送: X=%.1f°, Y=%.1f°, Z=%.1f°\n", angleX, angleY, angleZ);
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

// 发送回调函数
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    dataSent = true;
  } else {
    Serial.println("发送失败");
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

  // 配置加速度计 (±4g)
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_ACCEL_CONFIG);
  Wire.write(0x08);
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
    // 跳过温度和陀螺仪数据
    for (int i = 0; i < 8; i++) Wire.read();

    // 转换加速度数据
    float accelX = rawAccelX / 8192.0;
    float accelY = rawAccelY / 8192.0;
    float accelZ = rawAccelZ / 8192.0;

    // 计算角度
    angleX = atan2(accelY, accelZ) * 180.0 / PI;
    angleY = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;

    // 应用偏移量
    angleX -= offsetX;
    angleY -= offsetY;

    // 限制范围
    if (angleX > 90) angleX = 90;
    if (angleX < -90) angleX = -90;
    if (angleY > 90) angleY = 90;
    if (angleY < -90) angleY = -90;
  }
}

// 归零校准
void calibrateOffsets() {
  Serial.println("正在进行归零校准...");

  float sumX = 0, sumY = 0;
  const int samples = 100;

  for (int i = 0; i < samples; i++) {
    readMPU6050Data();
    sumX += angleX;
    sumY += angleY;
    delay(10);
  }

  offsetX = sumX / samples;
  offsetY = sumY / samples;

  Serial.printf("校准偏移量: X=%.2f, Y=%.2f\n", offsetX, offsetY);
}
