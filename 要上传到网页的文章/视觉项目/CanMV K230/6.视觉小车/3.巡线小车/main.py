'''
实验名称：K230巡线小车
实验平台：01Studio CanMV K230 + 小车底盘
'''

import time, os, sys, math

from media.sensor import * #导入sensor模块，使用摄像头相关接口
from media.display import * #导入display模块，使用display相关接口
from media.media import * #导入media模块，使用meida相关接口

#舵机相关库
from machine import I2C,FPIOA
from motor import Motors
import time

#将GPIO11,12配置为I2C2功能
fpioa = FPIOA()
fpioa.set_function(11, FPIOA.IIC2_SCL)
fpioa.set_function(12, FPIOA.IIC2_SDA)

i2c = I2C(2,freq=100000) #构建I2C对象

# 构建4路直流电机对象
m = Motors(i2c)

move_speed = 1800 #默认前进速度,PWM值,最大4095，太大容易跑飞

#小车停止
def stop():

    m.speed(0,0)
    m.speed(1,0)
    m.speed(2,0)
    m.speed(3,0)

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

        #坐标变化小于10不响应
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

# 初始化X轴PID控制器
x_pid = PID(p=0.05, i=0.0, d=0.001)  # 水平方向PID

# 设置X轴目标位置,0度偏移。
x_pid.set_target(0)

#3.5寸mipi屏分辨率定义
lcd_width = 800
lcd_height = 480

# 追踪黑线。使用 [(128, 255)] 追踪白线.
GRAYSCALE_THRESHOLD = [(0, 95)]

# 下面是一个roi【区域】元组列表。每个 roi 用 (x, y, w, h)表示的矩形。
#采样图像为640x480，列表把roi把图像分成3个矩形，越靠近的摄像头视野（通常为图像下方）的矩形权重越大。
ROIS = [ # [ROI, weight]
       (0, 400, 640, 80, 0.7), # 可以根据不同机器人情况进行调整。
       (0,  200, 640, 80, 0.3),
       (0,   0, 640, 80, 0.1)
      ]

# 计算以上3个矩形的权值【weight】的和，和不需要一定为1.
weight_sum = 0
for r in ROIS: weight_sum += r[4] # r[4] 为矩形权重值.

time.sleep_ms(100)
sensor = Sensor(width=1280, height=960) #构建摄像头对象，将摄像头长宽设置为4:3
sensor.reset() #复位和初始化摄像头
sensor.set_framesize(width=640, height=480) #设置帧大小，默认通道0
sensor.set_pixformat(Sensor.GRAYSCALE) #设置输出图像格式，默认通道0

Display.init(Display.ST7701, width=lcd_width, height=lcd_height, to_ide=True) #同时使用mipi屏和IDE缓冲区显示图像

MediaManager.init() #初始化media资源管理器

sensor.run() #启动sensor

clock = time.clock()

while True:

    ################
    ## 这里编写代码 ##
    ################
    clock.tick()

    img = sensor.snapshot() #拍摄一张图片

    centroid_sum = 0
    blob_detected = False  # 标记是否检测到黑线

    for r in ROIS:
        blobs = img.find_blobs(GRAYSCALE_THRESHOLD, roi=r[0:4], merge=True) # r[0:4] 是上面定义的roi元组.

        if blobs:

            blob_detected = True
            # Find the blob with the most pixels.
            largest_blob = max(blobs, key=lambda b: b.pixels())

            # Draw a rect around the blob.
            img.draw_rectangle(largest_blob.rect())
            img.draw_cross(largest_blob.cx(),
                           largest_blob.cy())

            centroid_sum += largest_blob.cx() * r[4] # r[4] 是每个roi的权重值.

    #丢线处理：未检测到黑线 → 立即停车（核心解决偶发识别失败）
    if not blob_detected:

        print("未检测到黑线，停车！")
        stop()

        img.draw_string_advanced(2,2,50, 'STOP', color=(255,255,255))

        #显示图片，仅用于LCD居中方式显示
        Display.show_image(img, x=round((lcd_width-sensor.width())/2),y=round((lcd_height-sensor.height())/2))
        continue

    center_pos = (centroid_sum / weight_sum) # 确定直线的中心.

    # 将直线中心位置转换成角度，便于机器人处理.
    deflection_angle = 0

    # 使用反正切函数计算直线中心偏离角度。可以自行画图理解
    #权重X坐标落在图像左半部分记作正偏，落在右边部分记为负偏，所以计算结果加负号。

    deflection_angle = -math.atan((center_pos-320)/240) #采用图像为 640*480时候使用

    # 将偏离值转换成偏离角度.
    deflection_angle = math.degrees(deflection_angle)

    # 计算偏离角度后可以控制机器人进行调整.
    print("Turn Angle: %f" % deflection_angle)

    # 更新水平（X轴）角度
    x_output = x_pid.update(deflection_angle)
    print(x_output)

    #控制左右电机转速
    left_speed = min(int(move_speed + x_output*1000),4095)
    right_speed = min(int(move_speed - x_output*1000),4095)
    print(left_speed,right_speed)

    # 驱动电机
    m.speed(0, left_speed)
    m.speed(1, left_speed)
    m.speed(2, right_speed)
    m.speed(3, right_speed)

    # LCD显示偏移角度,scale参数可以改变字体大小
    img.draw_string_advanced(2,2,30, str('%.1f' % deflection_angle)+"'", color=(255,255,255))
    img.draw_string_advanced(2,50,30, 'L: '+str(left_speed), color=(255,255,255))
    img.draw_string_advanced(2,90,30, 'R: '+str(right_speed), color=(255,255,255))

    #显示图片，仅用于LCD居中方式显示
    Display.show_image(img, x=round((lcd_width-sensor.width())/2),y=round((lcd_height-sensor.height())/2))

    print(clock.fps()) #打印FPS