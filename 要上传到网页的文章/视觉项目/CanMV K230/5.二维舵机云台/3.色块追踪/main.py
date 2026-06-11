'''
# Copyright (c) [2025] [01Studio]. Licensed under the MIT License.

实验名称：K230二维舵机云台（色块追踪）
实验平台：01Studio CanMV K230 + 3.5寸显示屏 + 二维舵机云台（含pyMotors驱动板）
说明：编程实现色块追踪，让色块保持在显示屏中央位置。（仅支持单个色块）
'''

import time, os, sys

from media.sensor import * #导入sensor模块，使用摄像头相关接口
from media.display import * #导入display模块，使用display相关接口
from media.media import * #导入media模块，使用meida相关接口

#舵机相关库
from machine import I2C,FPIOA
from servo import Servos
import time

#将GPIO11,12配置为I2C2功能
fpioa = FPIOA()
fpioa.set_function(11, FPIOA.IIC2_SCL)
fpioa.set_function(12, FPIOA.IIC2_SDA)

i2c = I2C(2,freq=10000) #构建I2C对象

#构建16路舵机对象
servo_x=Servos(i2c,degrees=270) #X轴使用的是270°舵机
servo_y=Servos(i2c,degrees=180) #Y轴使用的是180°舵机

#舵机对象使用用法, 详情参看servo.py文件
#
#s.position(index, degrees=None)
#index: 0~15表示16路舵机;
#degrees: 角度，0~180/270。

#云台初始位置，水平（X轴）135°和垂直（Y轴）90°，均居中。
x_angle = 135
y_angle = 90

servo_x.position(0,x_angle) #水平（X轴）使用使用端口0，转到135°
servo_y.position(1,y_angle) #垂直（Y轴）使用使用端口1，转到90°

# PID参数 (水平和垂直方向分别设置)
class PID:
    def __init__(self, p=0.05, i=0.01, d=0.01):
        self.kp = p
        self.ki = i
        self.kd = d
        self.target = 0
        self.error = 0
        self.last_error = 0
        self.integral = 0
        self.output = 0

    def update(self, current_value):
        self.error = self.target - current_value

        #变化小于10不响应
        if abs(self.error)<10:
            return 0

        self.integral += self.error
        derivative = self.error - self.last_error

        # 计算PID输出
        self.output = (self.kp * self.error) + (self.ki * self.integral) + (self.kd * derivative)

        self.last_error = self.error
        return self.output

    def set_target(self, target):
        self.target = target
        self.integral = 0
        self.last_error = 0

# 初始化PID控制器
x_pid = PID(p=0.01, i=0.0, d=0.001)  # 水平方向PID
y_pid = PID(p=0.015, i=0.0, d=0.001) # 垂直方向PID

# 设置目标位置 (图像中心), 3.5寸LCD分辨率为800x480
x_pid.set_target(800/2)
y_pid.set_target(480/2)

# 颜色识别阈值 (L Min, L Max, A Min, A Max, B Min, B Max) LAB模型
# 下面的阈值元组是用来识别 红、绿、蓝三种颜色，当然你也可以调整让识别变得更好。
thresholds = [(30, 100, 15, 127, 15, 127), # 红色阈值
              (30, 100, -64, -8, 50, 70), # 绿色阈值
              (0, 40, 0, 90, -128, -20)] # 蓝色阈值

sensor = Sensor() #构建摄像头对象
sensor.reset() #复位和初始化摄像头
sensor.set_framesize(width=800, height=480) #设置帧大小为LCD分辨率(800x480)，默认通道0
sensor.set_pixformat(Sensor.RGB565) #设置输出图像格式，默认通道0

Display.init(Display.ST7701, width=800, height=480, to_ide=True) #同时使用3.5寸mipi屏和IDE缓冲区显示图像，800x480分辨率
#Display.init(Display.VIRT, sensor.width(), sensor.height()) #只使用IDE缓冲区显示图像

MediaManager.init() #初始化media资源管理器

sensor.run() #启动sensor

clock = time.clock()

################
## 这里编写代码 ##
################

while True:

    clock.tick()

    img = sensor.snapshot() #拍摄一张图片

    blobs = img.find_blobs([thresholds[0]],area_threshold=400) # 0,1,2分别表示红，绿，蓝色。

    if blobs:

        for b in blobs: #画矩形和箭头表示

            tmp=img.draw_rectangle(b[0:4], thickness = 4, color = (0, 255, 0))
            tmp=img.draw_cross(b[5], b[6], thickness = 2, color = (0, 255, 0))

            x_center= b[0]+b[2]/2
            y_center=b[1]+b[3]/2

            #print(x_center,y_center)

            # 更新水平（X轴）舵机角度
            x_output = x_pid.update(x_center)
            x_angle = round(max(0, min(abs(x_angle + x_output),270)),1)
            servo_x.position(0,x_angle)

            # 更新垂直（Y轴）舵机角度
            y_output = y_pid.update(y_center)
            y_angle =  round(max(0, min(abs(y_angle - y_output),180)),1)
            servo_y.position(1,y_angle)


    #img.draw_string_advanced(0, 0, 30, 'FPS: '+str("%.3f"%(clock.fps())), color = (255, 255, 255))

    Display.show_image(img) #显示图片

    print(clock.fps()) #打印FPS
