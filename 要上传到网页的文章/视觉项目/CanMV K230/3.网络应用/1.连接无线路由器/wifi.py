import network,time
from machine import Pin

#WIFI连接函数
def WIFI_Connect():

    WIFI_LED=Pin(52, Pin.OUT) #初始化WIFI指示灯

    wlan = network.WLAN(network.STA_IF) #STA模式
    wlan.active(True)                   #激活接口

    start_time=time.time() #记录时间做超时判断

    if not wlan.isconnected():

        print('connecting to network...')

        #输入WIFI账号密码（仅支持2.4G信号）, 连接超过5秒为超时
        wlan.connect('123456', '0000000')

        while not wlan.isconnected():

            #超时判断,10秒没连接成功判定为超时
            if time.time()-start_time > 10 :
                print('WIFI Connected Timeout!')
                break

    if wlan.isconnected(): #连接成功

        print('connect success')

        #LED蓝灯点亮
        WIFI_LED.value(1)

        #等待获取IP地址
        while wlan.ifconfig()[0] == '0.0.0.0':
            pass

        #串口打印信息
        print('network information:', wlan.ifconfig())

    else: #连接失败

        #LED闪烁3次提示
        for i in range(3):
            WIFI_LED.value(1)
            time.sleep_ms(300)
            WIFI_LED.value(0)
            time.sleep_ms(300)

        wlan.active(False)

#执行WIFI连接函数
WIFI_Connect()

