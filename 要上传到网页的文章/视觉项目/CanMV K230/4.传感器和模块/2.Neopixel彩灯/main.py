'''
实验名称：RGB彩灯控制
版本：v1.0
平台：CanMV K230
作者：01Studio
说明：通过编程实现灯带不同颜色的变化。
'''
import time
from machine import Pin,Timer
from neopixel import NeoPixel

#定义红、绿、蓝三种颜色
RED=(255,0,0)
GREEN=(0,255,0)
BLUE=(0,0,255)

#42引脚连接灯带，灯珠数量30
#pin = Pin(30, Pin.OUT)
np = NeoPixel(Pin(42), 30)

#设置灯珠RGB颜色，本实验供30个灯珠
def rgb():
    
    global RED,GREEN,BLUE
    
    for i in range(30):
        if i < 10:
            np[i]=RED
        elif i <20:
            np[i]=GREEN
        else:
            np[i]=BLUE

while True:

    np.fill(RED) #红色
    np.write()     # 写入数据
    time.sleep(1)

    np.fill(GREEN) #红色
    np.write()     # 写入数据
    time.sleep(1)

    np.fill(BLUE) #红色
    np.write()     # 写入数据
    time.sleep(1)
    
    #RGB彩色模式
    rgb()
    np.write()
    time.sleep(1)