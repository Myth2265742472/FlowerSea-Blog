import time

import cv2
import mediapipe as mp
import serial


class FingerDirectionDetector:
    def __init__(self, com_port='COM4', baud_rate=115200):
        """
        初始化手指方向检测器

        Args:
            com_port: ESP32串口端口
            baud_rate: 波特率
        """
        self.com_port = com_port
        self.baud_rate = baud_rate
        self.serial_conn = None

        # 初始化MediaPipe
        self.mp_hands = mp.solutions.hands
        self.hands = self.mp_hands.Hands(
            static_image_mode=False,
            max_num_hands=1,
            min_detection_confidence=0.7,
            min_tracking_confidence=0.5
        )
        self.mp_drawing = mp.solutions.drawing_utils

        # 方向状态
        self.current_direction = None
        self.last_direction = None
        self.direction_change_time = 0
        self.direction_hold_time = 0.5  # 方向保持时间（秒）

    def connect_esp32(self):
        """连接ESP32串口"""
        try:
            self.serial_conn = serial.Serial(self.com_port, self.baud_rate, timeout=1)
            print(f"已连接到ESP32: {self.com_port}")
            time.sleep(2)  # 等待ESP32初始化
            return True
        except Exception as e:
            print(f"连接ESP32失败: {e}")
            return False

    def disconnect_esp32(self):
        """断开ESP32连接"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            print("ESP32连接已断开")

    def send_direction(self, direction):
        """发送方向到ESP32"""
        if self.serial_conn and self.serial_conn.is_open:
            try:
                # 发送方向命令
                command = f"DIR:{direction}\n"
                self.serial_conn.write(command.encode())
                print(f"发送方向: {direction}")
            except Exception as e:
                print(f"发送失败: {e}")

    def calculate_finger_direction(self, landmarks):
        """
        计算手指方向

        Args:
            landmarks: MediaPipe手部关键点

        Returns:
            str: 方向 ('up', 'down', 'left', 'right', 'none')
        """
        # 获取关键点坐标
        thumb_tip = landmarks.landmark[self.mp_hands.HandLandmark.THUMB_TIP]
        index_tip = landmarks.landmark[self.mp_hands.HandLandmark.INDEX_FINGER_TIP]
        middle_tip = landmarks.landmark[self.mp_hands.HandLandmark.MIDDLE_FINGER_TIP]
        ring_tip = landmarks.landmark[self.mp_hands.HandLandmark.RING_FINGER_TIP]
        pinky_tip = landmarks.landmark[self.mp_hands.HandLandmark.PINKY_TIP]

        wrist = landmarks.landmark[self.mp_hands.HandLandmark.WRIST]

        # 计算手指相对于手腕的位置
        finger_positions = {
            'thumb': (thumb_tip.x - wrist.x, thumb_tip.y - wrist.y),
            'index': (index_tip.x - wrist.x, index_tip.y - wrist.y),
            'middle': (middle_tip.x - wrist.x, middle_tip.y - wrist.y),
            'ring': (ring_tip.x - wrist.x, ring_tip.y - wrist.y),
            'pinky': (pinky_tip.x - wrist.x, pinky_tip.y - wrist.y)
        }

        # 检测伸出的手指
        extended_fingers = []
        for finger, (dx, dy) in finger_positions.items():
            # 简单的阈值检测
            if abs(dx) > 0.05 or abs(dy) > 0.05:
                extended_fingers.append(finger)

        # 根据伸出的手指判断方向
        if len(extended_fingers) == 1:
            finger = extended_fingers[0]
            dx, dy = finger_positions[finger]

            # 判断方向
            if abs(dx) > abs(dy):
                if dx > 0:
                    return 'right'
                else:
                    return 'left'
            else:
                if dy > 0:
                    return 'down'
                else:
                    return 'up'

        # 多手指或复杂手势
        elif len(extended_fingers) > 1:
            # 计算所有手指的平均方向
            avg_x = sum(finger_positions[f][0] for f in extended_fingers) / len(extended_fingers)
            avg_y = sum(finger_positions[f][1] for f in extended_fingers) / len(extended_fingers)

            if abs(avg_x) > abs(avg_y):
                if avg_x > 0:
                    return 'right'
                else:
                    return 'left'
            else:
                if avg_y > 0:
                    return 'down'
                else:
                    return 'up'

        return 'none'

    def run(self):
        """运行主程序"""
        # 连接ESP32
        if not self.connect_esp32():
            return

        # 初始化摄像头
        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            print("无法打开摄像头")
            return

        print("开始检测手指方向...")
        print("按 'q' 键退出")

        try:
            while True:
                ret, frame = cap.read()
                if not ret:
                    break

                # 翻转图像（镜像效果）
                frame = cv2.flip(frame, 1)
                height, width, _ = frame.shape

                # 转换为RGB
                rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

                # 处理手部检测
                results = self.hands.process(rgb_frame)

                # 绘制手部关键点
                if results.multi_hand_landmarks:
                    for hand_landmarks in results.multi_hand_landmarks:
                        self.mp_drawing.draw_landmarks(
                            frame, hand_landmarks, self.mp_hands.HAND_CONNECTIONS)

                        # 计算手指方向
                        direction = self.calculate_finger_direction(hand_landmarks)

                        # 检查方向是否改变
                        current_time = time.time()
                        if direction != self.current_direction:
                            self.current_direction = direction
                            self.direction_change_time = current_time
                            self.direction_hold_time = 0.5  # 重置保持时间

                        # 如果方向保持足够长时间，发送到ESP32
                        if (self.current_direction != 'none' and
                                self.current_direction != self.last_direction and
                                current_time - self.direction_change_time >= self.direction_hold_time):
                            self.send_direction(self.current_direction)
                            self.last_direction = self.current_direction

                # 在图像上显示当前方向
                if self.current_direction:
                    cv2.putText(frame, f"Direction: {self.current_direction.upper()}",
                                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

                # 显示图像
                cv2.imshow('Finger Direction Detection', frame)

                # 检查退出条件
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break

        except KeyboardInterrupt:
            print("程序被用户中断")
        finally:
            # 清理资源
            cap.release()
            cv2.destroyAllWindows()
            self.disconnect_esp32()


def main():
    """主函数"""
    print("ESP32 OLED 手指方向检测器")
    print("=" * 40)

    # 创建检测器实例
    detector = FingerDirectionDetector(com_port='COM4', baud_rate=115200)

    # 运行检测
    detector.run()


if __name__ == "__main__":
    main()
