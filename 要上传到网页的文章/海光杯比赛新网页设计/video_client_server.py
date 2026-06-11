# -*- coding: utf-8 -*-
"""
新网页综合服务
功能：调取原服务器(172.16.68.111:8080)的所有数据，在7788端口提供完整网页服务
支持缺陷尺寸和样品尺寸计算与显示
依赖：Flask, requests, OpenCV, numpy
"""

from flask import Flask, render_template, Response, jsonify, request
import requests
import cv2
import numpy as np
import threading
import time
import base64
from datetime import datetime

app = Flask(__name__)

ORIGINAL_SERVER = "http://172.16.68.111:8080"
TARGET_PORT = 7788

PIXEL_TO_MM = 0.1
SAMPLE_WIDTH_MM = 100
SAMPLE_HEIGHT_MM = 80

defect_names_cn = {
    'ca_shang': '擦伤',
    'zhen_kong': '真空',
    'zang_wu': '脏污',
    'zhe_zhou': '折皱',
    'good': '合格'
}

def calculate_defect_size(loc, img_width, img_height):
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
        "width_mm": round(width_mm, 2),
        "height_mm": round(height_mm, 2),
        "area_mm2": round(area_mm2, 2),
        "area_ratio": round(area_ratio, 2),
        "pixel_width": pixel_width,
        "pixel_height": pixel_height
    }

def draw_detection_with_size(image, results):
    if image is None:
        return None
    
    h, w = image.shape[:2]
    
    for r in results:
        loc = r.get('loc')
        if not loc or len(loc) < 4:
            continue
        
        x1, y1, x2, y2 = int(loc[0]), int(loc[1]), int(loc[2]), int(loc[3])
        class_name = r.get('class_name', 'unknown')
        score = r.get('score', 0)
        
        is_defect = class_name != 'good'
        color = (0, 0, 255) if is_defect else (0, 255, 0)
        
        cv2.rectangle(image, (x1, y1), (x2, y2), color, 2)
        
        size_info = calculate_defect_size(loc, w, h)
        
        label = f"{defect_names_cn.get(class_name, class_name)} {score*100:.1f}%"
        if size_info and is_defect:
            label += f" {size_info['width_mm']}x{size_info['height_mm']}mm"
        
        font = cv2.FONT_HERSHEY_SIMPLEX
        font_scale = 0.6
        thickness = 2
        
        (text_w, text_h), baseline = cv2.getTextSize(label, font, font_scale, thickness)
        
        cv2.rectangle(image, (x1, y1 - text_h - 10), (x1 + text_w + 5, y1), color, -1)
        cv2.putText(image, label, (x1 + 2, y1 - 5), font, font_scale, (255, 255, 255), thickness)
        
        if size_info and is_defect:
            size_text = f"Area: {size_info['area_mm2']}mm2 ({size_info['area_ratio']}%)"
            cv2.putText(image, size_text, (x1, y2 + 20), font, 0.5, color, 1)
    
    cv2.rectangle(image, (10, 10), (w - 10, h - 10), (255, 255, 0), 2)
    cv2.putText(image, f"Sample: {SAMPLE_WIDTH_MM}x{SAMPLE_HEIGHT_MM}mm", (20, 35), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 0), 2)
    
    return image


class DataFetcher:
    """数据获取器：获取原服务器所有数据"""

    def __init__(self, server_url):
        self.server_url = server_url
        self.current_frame = None
        self.detection_results = []
        self.last_detection_image = None
        self.last_detection_image_with_size = None
        self.system_status = {
            "connection": "未连接",
            "video_stream": "未连接",
            "detection_service": "未知",
            "arm_status": "未知",
            "camera_status": "未知",
            "last_update": None
        }
        self.arm_info = {
            "joint_angles": [0, 0, 0, 0, 0, 0],
            "gripper_status": "未知",
            "current_position": "未知",
            "load": 0,
            "temperature": 0
        }
        self.detection_stats = {
            "total": 0,
            "qualified": 0,
            "unqualified": 0,
            "pass_rate": 0,
            "defect_distribution": {}
        }
        self.defect_sizes = []
        self.lock = threading.Lock()
        self.running = False
        self.thread = None

    def start(self):
        if not self.running:
            self.running = True
            self.thread = threading.Thread(target=self._fetch_loop)
            self.thread.daemon = True
            self.thread.start()

    def stop(self):
        self.running = False
        if self.thread:
            self.thread.join(timeout=1)

    def _fetch_loop(self):
        while self.running:
            try:
                try:
                    resp = requests.get(f"{self.server_url}/hello", timeout=1)
                    if resp.status_code == 200:
                        self.system_status["connection"] = "已连接"
                        self.system_status["detection_service"] = "运行中"
                except:
                    self.system_status["connection"] = "未连接"
                    self.system_status["detection_service"] = "离线"

                try:
                    resp = requests.get(f"{self.server_url}/video_feed", timeout=0.5, stream=True)
                    if resp.status_code == 200:
                        self.system_status["video_stream"] = "运行中"
                        data = resp.raw.read(1024 * 10)
                        if data:
                            nparr = np.frombuffer(data, np.uint8)
                            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
                            if frame is not None:
                                with self.lock:
                                    self.current_frame = frame
                except:
                    self.system_status["video_stream"] = "未连接"

                try:
                    resp = requests.get(f"{self.server_url}/results", timeout=1)
                    if resp.status_code == 200:
                        data = resp.json()
                        with self.lock:
                            self.detection_results = data.get("results", [])
                            self._update_stats(data.get("results", []))
                            self._calculate_defect_sizes(data.get("results", []))
                except:
                    pass

                try:
                    resp = requests.get(f"{self.server_url}/detection_image", timeout=1)
                    if resp.status_code == 200:
                        with self.lock:
                            self.last_detection_image = resp.content
                            nparr = np.frombuffer(resp.content, np.uint8)
                            img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
                            if img is not None:
                                annotated = draw_detection_with_size(img.copy(), self.detection_results)
                                if annotated is not None:
                                    _, buf = cv2.imencode('.jpg', annotated)
                                    self.last_detection_image_with_size = buf.tobytes()
                except:
                    pass

                try:
                    resp = requests.get(f"{self.server_url}/arm_status", timeout=1)
                    if resp.status_code == 200:
                        data = resp.json()
                        with self.lock:
                            self.arm_info = data
                except:
                    pass

                self.system_status["last_update"] = datetime.now().strftime("%H:%M:%S")

            except Exception as e:
                self.system_status["connection"] = f"错误"

            time.sleep(0.5)

    def _update_stats(self, results):
        self.detection_stats["total"] = len(results)
        qualified = sum(1 for r in results if r.get("class_name") == "good")
        unqualified = len(results) - qualified
        self.detection_stats["qualified"] = qualified
        self.detection_stats["unqualified"] = unqualified
        if len(results) > 0:
            self.detection_stats["pass_rate"] = round(qualified / len(results) * 100, 1)

        defects = {}
        for r in results:
            cls = r.get("class_name", "unknown")
            if cls != "good":
                defects[cls] = defects.get(cls, 0) + 1
        self.detection_stats["defect_distribution"] = defects

    def _calculate_defect_sizes(self, results):
        self.defect_sizes = []
        for r in results:
            loc = r.get('loc')
            if loc and r.get('class_name') != 'good':
                size = calculate_defect_size(loc, 640, 480)
                if size:
                    self.defect_sizes.append({
                        "class_name": r.get('class_name'),
                        "class_cn": defect_names_cn.get(r.get('class_name'), r.get('class_name')),
                        "size": size
                    })

    def get_frame(self):
        with self.lock:
            return self.current_frame

    def get_detection_results(self):
        with self.lock:
            return self.detection_results

    def get_detection_image(self):
        with self.lock:
            return self.last_detection_image

    def get_detection_image_with_size(self):
        with self.lock:
            return self.last_detection_image_with_size

    def get_system_status(self):
        return self.system_status

    def get_arm_info(self):
        return self.arm_info

    def get_detection_stats(self):
        return self.detection_stats

    def get_defect_sizes(self):
        return self.defect_sizes


fetcher = DataFetcher(ORIGINAL_SERVER)


@app.route('/')
def index():
    return render_template('new_index.html')


@app.route('/status')
def status():
    return jsonify({
        "system": fetcher.get_system_status(),
        "arm": fetcher.get_arm_info(),
        "detection": fetcher.get_detection_stats(),
        "recent_results": fetcher.get_detection_results()[-10:],
        "defect_sizes": fetcher.get_defect_sizes(),
        "sample_size": {
            "width_mm": SAMPLE_WIDTH_MM,
            "height_mm": SAMPLE_HEIGHT_MM
        }
    })


@app.route('/system_status')
def system_status():
    return jsonify(fetcher.get_system_status())


@app.route('/arm_status')
def arm_status():
    return jsonify(fetcher.get_arm_info())


@app.route('/detection_stats')
def detection_stats():
    return jsonify(fetcher.get_detection_stats())


@app.route('/video_feed')
def video_feed():
    def generate():
        while True:
            frame = fetcher.get_frame()
            if frame is not None:
                ret, jpeg = cv2.imencode('.jpg', frame)
                if ret:
                    yield (b'--frame\r\n'
                           b'Content-Type: image/jpeg\r\n\r\n' + jpeg.tobytes() + b'\r\n')
            time.sleep(0.033)
    return Response(generate(), mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/frame')
def frame():
    frame = fetcher.get_frame()
    if frame is not None:
        ret, jpeg = cv2.imencode('.jpg', frame)
        if ret:
            return Response(jpeg.tobytes(), mimetype='image/jpeg')
    return Response(b'', mimetype='image/jpeg')


@app.route('/detection_results')
def detection_results():
    results = fetcher.get_detection_results()
    results_with_size = []
    for r in results:
        r_copy = r.copy()
        loc = r.get('loc')
        if loc and r.get('class_name') != 'good':
            size = calculate_defect_size(loc, 640, 480)
            r_copy['size'] = size
        results_with_size.append(r_copy)
    
    return jsonify({
        "results": results_with_size,
        "count": len(results_with_size),
        "sample_size": {
            "width_mm": SAMPLE_WIDTH_MM,
            "height_mm": SAMPLE_HEIGHT_MM
        }
    })


@app.route('/detection_image')
def detection_image():
    img_data = fetcher.get_detection_image()
    if img_data:
        return Response(img_data, mimetype='image/jpeg')
    return Response(b'No image', mimetype='text/plain')


@app.route('/detection_image_with_size')
def detection_image_with_size():
    img_data = fetcher.get_detection_image_with_size()
    if img_data:
        return Response(img_data, mimetype='image/jpeg')
    return Response(b'No image', mimetype='text/plain')


@app.route('/defect_sizes')
def defect_sizes():
    return jsonify({
        "defect_sizes": fetcher.get_defect_sizes(),
        "sample_size": {
            "width_mm": SAMPLE_WIDTH_MM,
            "height_mm": SAMPLE_HEIGHT_MM
        },
        "pixel_to_mm_ratio": PIXEL_TO_MM
    })


@app.route('/api/request_detection', methods=['POST'])
def request_detection():
    try:
        resp = requests.post(f"{ORIGINAL_SERVER}/predict", timeout=5)
        if resp.status_code == 200:
            return jsonify(resp.json())
        else:
            return jsonify({"error": f"服务器返回{resp.status_code}"})
    except Exception as e:
        return jsonify({"error": str(e)})


@app.route('/api/send_image', methods=['POST'])
def send_image():
    if 'image' not in request.files:
        return jsonify({"error": "没有图片文件"})

    file = request.files['image']
    try:
        files = {'image_file': (file.filename, file.read(), 'image/jpeg')}
        resp = requests.post(f"{ORIGINAL_SERVER}/predict", files=files, timeout=5)
        if resp.status_code == 200:
            return jsonify(resp.json())
        else:
            return jsonify({"error": f"服务器返回{resp.status_code}"})
    except Exception as e:
        return jsonify({"error": str(e)})


@app.route('/control')
def control():
    return jsonify({
        "app_name": "机械臂视觉检测系统 - 完整版(含尺寸)",
        "original_server": ORIGINAL_SERVER,
        "new_port": TARGET_PORT,
        "sample_size": {
            "width_mm": SAMPLE_WIDTH_MM,
            "height_mm": SAMPLE_HEIGHT_MM
        },
        "pixel_to_mm_ratio": PIXEL_TO_MM,
        "all_endpoints": {
            "GET /": "完整网页界面",
            "GET /status": "完整状态信息",
            "GET /system_status": "系统状态",
            "GET /arm_status": "机械臂状态",
            "GET /detection_stats": "检测统计",
            "GET /video_feed": "视频流",
            "GET /frame": "单帧图像",
            "GET /detection_results": "检测结果列表(含尺寸)",
            "GET /detection_image": "检测图片",
            "GET /detection_image_with_size": "检测图片(含尺寸标注)",
            "GET /defect_sizes": "缺陷尺寸列表",
            "POST /api/request_detection": "请求检测",
            "POST /api/send_image": "上传图片检测"
        },
        "defect_types": {
            "ca_shang": "擦伤",
            "zhen_kong": "真空",
            "zang_wu": "脏污",
            "zhe_zhou": "折皱"
        }
    })


if __name__ == "__main__":
    print("=" * 60)
    print("  机械臂视觉检测系统 - 完整版(含尺寸显示)")
    print("=" * 60)
    print(f"  数据源: {ORIGINAL_SERVER}")
    print(f"  端口: {TARGET_PORT}")
    print(f"  样品尺寸: {SAMPLE_WIDTH_MM}x{SAMPLE_HEIGHT_MM}mm")
    print(f"  像素转换: {PIXEL_TO_MM}mm/像素")
    print(f"  访问: http://172.16.68.111:{TARGET_PORT}")
    print("=" * 60)

    fetcher.start()
    app.run(host="0.0.0.0", port=TARGET_PORT, threaded=True)
