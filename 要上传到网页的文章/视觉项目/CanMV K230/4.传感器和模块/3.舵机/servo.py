'''
# Copyright (c) [2025] [01Studio]. Licensed under the MIT License.

实验名称：舵机控制
实验平台：01Studio CanMV K230
说明：通过编程控制舵机旋转到不同角度
版本：v1.1
'''

from machine import Pin, PWM
from machine import FPIOA
import time

#配置引脚42为PWM0功能
#通道0：GPIO42,通道1：GPIO43,通道2：GPIO46,通道3：GPIO47
fpioa = FPIOA()
fpioa.set_function(42,FPIOA.PWM0)

#构建PWM0对象，通道0，频率为50Hz，占空比为0，默认使能输出
S1 = PWM(0, freq=50, duty_ns=0) # 在同一语句下创建和配置PWM

'''
#其它PWM引脚配置参考代码

#配置引脚43为PWM1功能
fpioa = FPIOA()
fpioa.set_function(43,FPIOA.PWM1)

#构建PWM1对象，通道1，频率为50Hz，占空比为0，默认使能输出
S2 = PWM(0, freq=50, duty_ns=0) # 在同一语句下创建和配置PWM
'''

'''
说明：舵机控制函数
功能：180度舵机：angle:-90至90 表示相应的角度
     360连续旋转度舵机：angle:-90至90 旋转方向和速度值。
    【duty_ns】占空比值：0.5ms-2.5ms
'''
def Servo(servo,angle):
    
    servo.duty_ns(int((angle+90)/180*2000000+500000))


while True:

    #-90度
    Servo(S1,-90)
    time.sleep(1)

    #-45度
    Servo(S1,-45)
    time.sleep(1)

    #0度
    Servo(S1,0)
    time.sleep(1)

    #45度
    Servo(S1,45)
    time.sleep(1)

    #90度
    Servo(S1,90)
    time.sleep(1)
