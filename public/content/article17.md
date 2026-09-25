# 海光杯比赛 - 机械臂视觉检测系统

## 项目简介

本项目是"海光杯"比赛作品，基于树莓派和 Python Flask 构建的机械臂视觉检测系统。系统通过调用原服务器数据，在本地 7788 端口提供完整的网页服务，支持实时视频流显示、缺陷检测、尺寸计算等功能。

## 功能特性

- ✅ **实时视频流**：显示机械臂工作区域的实时画面
- ✅ **缺陷检测**：自动识别擦伤、真空、脏污、折皱等缺陷
- ✅ **尺寸计算**：计算缺陷和样品的实际尺寸（毫米）
- ✅ **图片上传检测**：支持上传图片进行离线检测
- ✅ **机械臂状态显示**：实时显示机械臂工作状态
- ✅ **Web 界面**：美观的网页控制面板

## 技术架构

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   原服务器       │     │   树莓派         │     │   用户浏览器     │
│ 172.16.68.111:8080│ <--│  video_client_  │ --> │  http://pi:7788 │
│                 │     │  server.py      │     │                 │
└─────────────────┘     └─────────────────┘     └─────────────────┘
        │                       │                       │
        └────── 数据接口 ───────┘                       │
                                └────── Web 页面 ───────┘
```

## 所需材料

### 硬件
- **树莓派** (3B+/4B)
- **摄像头模块** (USB 或 CSI)
- **机械臂** (与视觉系统配合)
- **网线/WiFi**

### 软件
- **Python 3.7+**
- **Flask**
- **OpenCV**
- **NumPy**
- **Requests**

## 项目结构

```
海光杯比赛新网页设计/
├── video_client_server.py  # 主程序
├── run.sh                  # 启动脚本
├── templates/
│   └── new_index.html      # 网页界面
└── 使用说明.txt
```

## 核心代码

### video_client_server.py

```python
from flask import Flask, render_template, Response, jsonify, request
import requests
import cv2
import numpy as np
import threading
import time
import base64
from datetime import datetime

app = Flask(__name__)

# 原服务器地址
ORIGINAL_SERVER = "http://172.16.68.111:8080"
TARGET_PORT = 7788

# 尺寸转换参数
PIXEL_TO_MM = 0.1      # 像素转毫米比例
SAMPLE_WIDTH_MM = 100  # 样品宽度
SAMPLE_HEIGHT_MM = 80  # 样品高度

# 缺陷类型中文映射
defect_names_cn = {
    'ca_shang': '擦伤',
    'zhen_kong': '真空',
    'zang_wu': '脏污',
    'zhe_zhou': '折皱',
    'good': '合格'
}

def calculate_defect_size(loc, img_width, img_height):
    """计算缺陷尺寸"""
    if not loc or len(loc) < 4:
        return None
    
    x1, y1, x2, y2 = loc[0], loc[1], loc[2], loc[3]
    
    pixel_width = abs(x2 - x1)
    pixel_height = abs(y2 - y1)
    
    width_mm = pixel_width * PIXEL_TO_MM
    height_mm = pixel_height * PIXEL_TO_MM
    area_mm2 = width_mm * height_mm
    
    sample_area_mm2 = SAMPLE_WIDTH_MM * SAMPLE_HEIGHT_MM
    area_ratio = (area_mm2 / sample_area_mm2) * 100
    
    return {
        'width_mm': round(width_mm, 2),
        'height_mm': round(height_mm, 2),
        'area_mm2': round(area_mm2, 2),
        'area_ratio': round(area_ratio, 2)
    }

@app.route('/')
def index():
    """主页"""
    return render_template('new_index.html')

@app.route('/video_feed')
def video_feed():
    """视频流"""
    def generate():
        while True:
            # 从原服务器获取视频帧
            try:
                response = requests.get(f"{ORIGINAL_SERVER}/video_feed", timeout=5)
                if response.status_code == 200:
                    yield (b'--frame\r\n'
                           b'Content-Type: image/jpeg\r\n\r\n' + response.content + b'\r\n')
            except Exception as e:
                print(f"获取视频流失败: {e}")
                time.sleep(1)
    
    return Response(generate(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/api/detection')
def detection():
    """获取检测结果"""
    try:
        response = requests.get(f"{ORIGINAL_SERVER}/api/detection", timeout=5)
        if response.status_code == 200:
            data = response.json()
            
            # 处理缺陷数据
            if 'defects' in data:
                for defect in data['defects']:
                    if 'type' in defect:
                        defect['type_cn'] = defect_names_cn.get(defect['type'], defect['type'])
                    if 'location' in defect:
                        size_info = calculate_defect_size(
                            defect['location'],
                            data.get('img_width', 640),
                            data.get('img_height', 480)
                        )
                        if size_info:
                            defect['size'] = size_info
            
            return jsonify(data)
    except Exception as e:
        return jsonify({'error': str(e)})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=TARGET_PORT, debug=False)
```

### run.sh

```bash
#!/bin/bash
cd /home/pi/汇总
python3 video_client_server.py
```

## 部署步骤

### 1. 安装依赖

```bash
sudo apt update
sudo apt install python3 python3-pip
pip3 install flask requests opencv-python numpy
```

### 2. 上传文件

使用 WinSCP 将项目文件夹上传到树莓派 `/home/pi/` 目录。

### 3. 配置权限

```bash
cd /home/pi/汇总
chmod +x run.sh
```

### 4. 启动服务

```bash
./run.sh
```

### 5. 访问系统

在浏览器中访问：
```
http://树莓派IP:7788
```

## 功能说明

### 缺陷类型

| 类型 | 说明 |
|------|------|
| ca_shang | 擦伤 |
| zhen_kong | 真空 |
| zang_wu | 脏污 |
| zhe_zhou | 折皱 |
| good | 合格 |

### 参数配置

在 `video_client_server.py` 中修改以下参数：

```python
PIXEL_TO_MM = 0.1      # 像素转毫米比例
SAMPLE_WIDTH_MM = 100  # 样品宽度
SAMPLE_HEIGHT_MM = 80  # 样品高度
```

## 常见问题

### 1. 权限不足

```bash
chmod +x run.sh
```

### 2. 找不到 Python

```bash
sudo apt install python3
```

### 3. 缺少依赖

```bash
pip3 install flask requests opencv-python numpy
```

### 4. 无法连接原服务器

- 检查网络连接
- 确认原服务器 IP 和端口正确
- 检查防火墙设置

## 扩展功能

1. **数据库存储**：将检测记录保存到数据库
2. **报警系统**：检测到严重缺陷时发送通知
3. **统计分析**：生成检测报告和统计图表
4. **多摄像头支持**：同时监控多个工位

## 参考文档

- [Flask 官方文档](https://flask.palletsprojects.com/)
- [OpenCV Python 教程](https://docs.opencv.org/4.x/d6/d00/tutorial_py_root.html)
- [树莓派官方文档](https://www.raspberrypi.org/documentation/)

---

**注意**：确保树莓派与原服务器在同一网络中，且网络连接稳定。
