#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;
const int ledPin = 2; // 板载LED

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  // 读取加速度和陀螺仪数据
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 发送数据到串口，格式: ax,ay,az,gx,gy,gz
  Serial.print(ax); Serial.print(",");
  Serial.print(ay); Serial.print(",");
  Serial.print(az); Serial.print(",");
  Serial.print(gx); Serial.print(",");
  Serial.print(gy); Serial.print(",");
  Serial.println(gz);

  // 检查串口是否有来自Matlab的指令
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == '1') {
      digitalWrite(ledPin, HIGH); // 点亮LED
    } else if (cmd == '0') {
      digitalWrite(ledPin, LOW); // 熄灭LED
    }
  }

  delay(20); // 50Hz
}