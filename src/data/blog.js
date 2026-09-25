// 文章元数据（从原 js/markdown-loader.js 迁移）
// rawMarkdown 仍按需从 content/ 目录 fetch
export const articlesMeta = {
  article1: { title: '深入理解 Typecho 博客系统的设计与实现', author: 'FlowerSea', date: '2026年3月7日', category: '技术教程', comments: '5 条', tags: ['Typecho', 'PHP', '博客', 'CMS', '教程'], summary: 'Typecho 是国内知名的轻量级博客程序，以其简洁、高效、易用而著称。本文将深入探讨 Typecho 的核心设计理念，以及如何利用它搭建一个功能完善的个人博客。' },
  article2: { title: '从零开始搭建个人技术博客的完整指南', author: 'FlowerSea', date: '2026年3月6日', category: '建站教程', comments: '3 条', tags: ['博客', '建站', 'Hexo', 'WordPress', 'Typecho'], summary: '无论是记录学习心得、分享项目经验，还是建立个人品牌，博客都发挥着重要作用。本文将详细介绍如何从零开始，通过多种方式搭建一个属于自己的技术博客。' },
  article3: { title: 'Web 前端开发最佳实践与性能优化', author: 'FlowerSea', date: '2026年3月5日', category: '前端开发', comments: '8 条', tags: ['前端', '性能优化', 'React', 'Vue', '最佳实践'], summary: '作为一名前端工程师，如何在快速变化的技术浪潮中保持竞争力？本文将从代码规范、性能优化、可访问性等多个角度，分享前端开发的最佳实践。' },
  article4: { title: '使用 Git 进行版本控制的实用技巧', author: 'FlowerSea', date: '2026年3月4日', category: '版本控制', comments: '12 条', tags: ['Git', '版本控制', 'GitHub', '协作', '工作流'], summary: 'Git 是现代软件开发中不可或缺的版本控制工具。本文将介绍一些实用的 Git 技巧，帮助你更高效地管理代码版本，提升团队协作效率。' },
  article5: { title: 'BilibiliPotPlayer 插件说明', author: 'FlowerSea', date: '2026年6月7日', category: '软件工具', comments: '0 条', tags: ['PotPlayer', 'Bilibili', '插件'], summary: '适用于 PotPlayer 的 Bilibili 插件。如果配合油猴脚本，可以直接在网页打开 PotPlayer 进行视频播放，提供了极其便利的观影体验。' },
  article6: { title: 'ESP32 I2S MEMS Microphone Arduino IDE Example', author: 'FlowerSea', date: '2026年6月7日', category: '嵌入式开发', comments: '0 条', tags: ['ESP32', 'Arduino', 'Microphone'], summary: 'This repository holds some samples for connecting a I2S MEMS microphone to an ESP32 board, specifically focusing on overcoming the lack of generic ESP32 I2S examples.' },
  article7: { title: 'ESP32 I2S MEMS 麦克风 Arduino IDE 示例', author: 'FlowerSea', date: '2026年6月7日', category: '嵌入式开发', comments: '0 条', tags: ['ESP32', 'Arduino', 'Microphone'], summary: '本仓库包含一些将 I2S MEMS 麦克风连接到 ESP32 开发板的示例代码。弥补了官方仅提供特定开发板示例的不足，方便开发者快速上手音频采集。' },
  article8: { title: 'Flash 下载工具说明', author: 'FlowerSea', date: '2026年6月7日', category: '开发工具', comments: '0 条', tags: ['ESP32', 'Flash', 'Tool'], summary: '详细介绍了 ESP32 Flash 下载工具的使用方法、固件烧录地址配置说明，以及如何正确写入 Bootloader、分区表和主程序固件。' },
  article9: { title: 'YD-ESP32-S3 Python 示例代码说明', author: 'FlowerSea', date: '2026年6月7日', category: '嵌入式开发', comments: '0 条', tags: ['ESP32-S3', 'Python'], summary: '基于 YD-ESP32-S3 开发板的 Python 示例代码集合。包含如何使用 MicroPython 控制板载 NeoPixel RGB 灯等基础入门教程。' },
  article10: { title: 'ESP32 + Adafruit PWM Servo Driver + MPU6050 姿态同步控制项目', author: 'FlowerSea', date: '2026年6月7日', category: '嵌入式开发', comments: '0 条', tags: ['ESP32', 'MPU6050', 'Servo'], summary: '本项目实现了 ESP32 微控制器与 Adafruit 16路 PWM 舵机驱动板和 MPU6050 传感器的连接，用于控制 6 个舵机与 MPU6050 姿态进行实时同步。' },
  article11: { title: 'ESP32 MPU6050 舵机控制系统', author: 'FlowerSea', date: '2026年6月7日', category: '嵌入式开发', comments: '0 条', tags: ['ESP32', 'MPU6050', '舵机'], summary: '实现使用 ESP32 微控制器，通过读取 MPU6050 传感器的加速度计和陀螺仪数据，来实时控制舵机转动的系统，提供了完整的接线与代码说明。' },
  article12: { title: 'ESP32 蓝牙低功耗(BLE)通信入门', author: 'FlowerSea', date: '2026年6月7日', category: '物联网', comments: '0 条', tags: ['ESP32', 'BLE', '蓝牙', '物联网'], summary: '本项目展示了如何使用 ESP32 开发板创建蓝牙低功耗(BLE)服务器，实现与手机或其他蓝牙设备的无线通信。通过简单的代码，即可让 ESP32 广播蓝牙信号，接收和发送数据。' },
  article13: { title: 'ESP32 网络遥控车开发指南', author: 'FlowerSea', date: '2026年6月7日', category: '物联网', comments: '0 条', tags: ['ESP32', 'WiFi', '遥控车', '物联网'], summary: '本项目展示了如何使用 ESP32 开发板构建一个可以通过 WiFi 网络远程控制的智能小车。通过手机或电脑浏览器，即可实现前进、后退、转向等基本控制功能。' },
  article14: { title: 'ESP32 I2S MEMS 麦克风音频采集实战', author: 'FlowerSea', date: '2026年6月7日', category: '嵌入式开发', comments: '0 条', tags: ['ESP32', 'I2S', '麦克风', '音频'], summary: '本项目展示了如何使用 ESP32 开发板连接 I2S MEMS 麦克风（如 INMP441），实现高质量的音频数据采集。通过 I2S 接口，ESP32 可以读取数字音频信号，并进行实时处理。' },
  article15: { title: 'ESP32 Flash 下载工具使用指南', author: 'FlowerSea', date: '2026年6月7日', category: '开发工具', comments: '0 条', tags: ['ESP32', 'Flash', '固件', '工具'], summary: 'ESP32 Flash 下载工具是乐鑫官方提供的固件烧录工具，用于将编译好的固件文件下载到 ESP32 系列芯片的 Flash 存储器中。支持 ESP32 全系列芯片。' },
  article16: { title: 'YD-ESP32-S3 MicroPython 开发入门', author: 'FlowerSea', date: '2026年6月7日', category: '嵌入式开发', comments: '0 条', tags: ['ESP32-S3', 'MicroPython', 'Python'], summary: 'YD-ESP32-S3 是一款基于 ESP32-S3 芯片的开发板，集成了 WiFi、蓝牙、USB OTG 等功能。本项目展示了如何使用 MicroPython 在该开发板上进行快速开发。' },
  article17: { title: '海光杯比赛 - 机械臂视觉检测系统', author: 'FlowerSea', date: '2026年6月7日', category: '计算机视觉', comments: '0 条', tags: ['树莓派', 'Python', '视觉检测', '机械臂', '比赛'], summary: '海光杯比赛作品，基于树莓派和 Python Flask 构建的机械臂视觉检测系统。支持实时视频流显示、缺陷检测、尺寸计算等功能。' },
  article18: { title: '基于51单片机及DS18B20温度传感器的数字温度计设计', author: 'FlowerSea', date: '2026年6月7日', category: '单片机', comments: '0 条', tags: ['51单片机', 'DS18B20', '温度传感器', 'ESP32'], summary: '基于 STC89C52 单片机和 DS18B20 数字温度传感器的完整数字温度计系统。支持温度上下限设置、超限报警、按键音等功能，通过 ESP32 实现 WiFi 联网监控。' },
  article19: { title: 'ESP32 视觉识别项目开发指南', author: 'FlowerSea', date: '2026年6月7日', category: '计算机视觉', comments: '0 条', tags: ['ESP32', '视觉识别', '摄像头', 'AI'], summary: '基于 ESP32-CAM 的视觉识别系统，利用 ESP32 的强大处理能力和摄像头模块，实现图像采集、处理和识别功能。支持物体检测、颜色识别、人脸检测等应用。' },
  article20: { title: 'ESP32 + MPU6050 姿态检测与可视化系统', author: 'FlowerSea', date: '2026年6月11日', category: '传感器', comments: '0 条', tags: ['ESP32', 'MPU6050', '姿态检测', 'MATLAB', '四元数'], summary: '基于 ESP32 和 MPU6050 的完整姿态检测系统，实现传感器数据采集、姿态解算、互补滤波、四元数转换，并通过 MATLAB 进行实时 3D 可视化。包含四电机控制版本，可用于无人机或机器人姿态控制。' },
  article21: { title: 'ESP32 舵机控制全攻略：从基础到高级应用', author: 'FlowerSea', date: '2026年6月11日', category: '嵌入式开发', comments: '0 条', tags: ['ESP32', '舵机', 'MPU6050', 'ESP-NOW', '卡尔曼滤波', 'Python', 'MediaPipe'], summary: '完整的 ESP32 舵机控制教程合集，涵盖单舵机基础控制、MPU6050 有线/无线同步、二维卡尔曼滤波云台、Python 头部追踪控制等五大项目。包含 PCA9685 扩展驱动、平滑滤波、死区控制等进阶技术。' }
}

// 音乐播放列表（从原 js/player.js 迁移）
export const playlist = [
  { title: 'Daisy Crown', artist: 'Empty old City,Wuthering Waves', file: 'Empty old City,Wuthering Waves - Daisy Crown.mp3' },
  { title: '该从何处寻你', artist: '巴音孟克 Maj7_Music', file: '巴音孟克 Maj7_Music - 该从何处寻你.mp3' },
  { title: '月半小夜曲', artist: '陈乐基 Rocky Chan', file: '陈乐基 Rocky Chan - 月半小夜曲.mp3' },
  { title: '麦恩莉', artist: '方大同', file: '方大同 - 麦恩莉.mp3' },
  { title: '相机', artist: '加木', file: '加木 - 相机.mp3' },
  { title: '鱼', artist: '加木', file: '加木 - 鱼.mp3' },
  { title: '出现又离开', artist: '梁博', file: '梁博 - 出现又离开 (Live).mp3' },
  { title: '吻得太逼真', artist: '刘大拿,Wiz_H张子豪', file: '刘大拿,Wiz_H张子豪 - 吻得太逼真.mp3' },
  { title: '戒不掉', artist: '欧阳耀莹', file: '欧阳耀莹 - 戒不掉（原声版）.mp3' },
  { title: '爱错', artist: '王力宏', file: '王力宏 - 爱错.mp3' },
  { title: 'Layla蕾拉', artist: '张杰', file: '张杰 - Layla蕾拉.mp3' },
  { title: '酷爱', artist: '张敬轩', file: '张敬轩 - 酷爱.mp3' },
  { title: '雨夜街头', artist: 'Mikey-18,07Kevin,豪一鸽', file: 'Mikey-18,07Kevin,豪一鸽 - 雨夜街头.mp3' }
]
