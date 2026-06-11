'''
# Copyright (c) [2025] [01Studio]. Licensed under the MIT License.

实验名称：二维舵机云台两路舵机控制
实验平台：01Studio CanMV K230 + 二维舵机云台（含pyMotors驱动板）
说明：控制二维舵机云台的两路舵机
'''

from machine import I2C,FPIOA
from servo import Servos
import time

#将GPIO11,12配置为I2C2功能
fpioa = FPIOA()
fpioa.set_function(11, FPIOA.IIC2_SCL)
fpioa.set_function(12, FPIOA.IIC2_SDA)

i2c = I2C(2, freq=10000) #构建I2C对象，推荐频率小于10KHz

#构建二维云台2路舵机对象
servo_x=Servos(i2c,degrees=270) #水平（X轴）使用的是270°舵机
servo_y=Servos(i2c,degrees=180) #垂直（Y轴）使用的是180°舵机

#舵机对象使用用法, 详情参看servo.py文件
#s.position(index, degrees=None)
#index: 0~15表示16路舵机;
#degrees: 角度，0~180/270。

#初始位置，可以修改角度观察现象
servo_x.position(0,135) #水平（X轴）使用使用端口0，转到135°
servo_y.position(1,90) #垂直（Y轴）使用使用端口1，转到90°

while True:
    pass

