import sys
import time
import threading
import argparse
import serial
from serial.tools import list_ports
import cv2
import numpy as np
import math

# Try to import mediapipe; show a helpful error if missing
try:
	import mediapipe as mp
except Exception as exc:
	print("ERROR: mediapipe not installed. Run: pip install mediapipe")
	raise

# ---------- Serial helpers ----------

def detect_serial_port():
	ports = list(list_ports.comports())
	print("Available serial ports:")
	for p in ports:
		print(f" - {p.device}: {p.description}")
	for p in ports:
		desc = (p.description or "").lower()
		if "usb" in desc or "uart" in desc or "esp" in desc or "ch340" in desc or "cp210" in desc:
			return p.device
	return ports[0].device if ports else None


def open_serial(port: str | None, baud: int, startup_delay_s: float = 2.0):
	if not port:
		port = detect_serial_port()
	if not port:
		print("ERROR: No serial ports found. Plug in ESP32 and try again.")
		sys.exit(1)
	print("Opening serial port:", port)
	ser = serial.Serial(port, baud, timeout=0.1)
	time.sleep(startup_delay_s)
	ser.reset_input_buffer()
	ser.reset_output_buffer()
	return ser, port


# ---------- Yaw estimation via MediaPipe FaceMesh ----------
# Landmarks indices used (MediaPipe FaceMesh 468 landmarks):
# - 33: left eye outer corner
# - 263: right eye outer corner
# - 1: nose tip (approx)
# Using 2D geometry: yaw ~ normalized horizontal offset of nose from eye-center, scaled by eye distance.

class YawEstimator:
	def __init__(self, camera_index: int = 0, width: int | None = None, height: int | None = None):
		self.cap = cv2.VideoCapture(camera_index)
		if width is not None:
			self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
		if height is not None:
			self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
		if not self.cap.isOpened():
			raise RuntimeError(f"Cannot open camera index {camera_index}")
		self.mp_face = mp.solutions.face_mesh
		self.face = self.mp_face.FaceMesh(static_image_mode=False, refine_landmarks=False, max_num_faces=1, min_detection_confidence=0.5, min_tracking_confidence=0.5)
		self.drawing = mp.solutions.drawing_utils

	def read_frame(self):
		ok, frame = self.cap.read()
		if not ok:
			raise RuntimeError("Camera read failed")
		return frame

	def compute_yaw_norm(self, frame) -> tuple[float | None, dict]:
		# Returns yaw_norm in approx range [-1, 1] where + means turn right
		h, w = frame.shape[:2]
		rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
		res = self.face.process(rgb)
		if not res.multi_face_landmarks:
			return None, {}
		lm = res.multi_face_landmarks[0].landmark
		get = lambda idx: (lm[idx].x * w, lm[idx].y * h)
		lx, ly = get(33)
		rx, ry = get(263)
		nx, ny = get(1)
		cx = (lx + rx) * 0.5
		eye_dist = math.hypot(rx - lx, ry - ly)
		if eye_dist <= 1e-6:
			return None, {}
		# Positive when nose is to the right of center (user turned right)
		yaw_norm = (nx - cx) / eye_dist * 2.0  # scale factor calibrated empirically
		aux = {"lx": int(lx), "ly": int(ly), "rx": int(rx), "ry": int(ry), "nx": int(nx), "ny": int(ny)}
		return yaw_norm, aux

	def release(self):
		try:
			self.cap.release()
		except Exception:
			pass


# ---------- Mapping + Serial ----------

def yaw_to_angle(yaw_norm: float, center_angle: int = 90, max_delta_deg: int = 60, gain: float = 1.0) -> int:
	# Clamp yaw_norm then map to servo angle with center at 90
	yaw_clamped = max(-1.0, min(1.0, yaw_norm * gain))
	angle = int(center_angle + yaw_clamped * max_delta_deg)
	return max(0, min(180, angle))


def send_angle(ser: serial.Serial, angle: int, channel: int | None = None):
	angle = max(0, min(180, int(angle)))
	if channel is None:
		cmd = f"ANGLE {angle}\n"
	else:
		cmd = f"CHANGLE {channel} {angle}\n"
	ser.write(cmd.encode("utf-8"))
	print("[TX]", cmd.strip())


def draw_debug(frame, aux: dict, yaw_norm: float | None, angle: int | None):
	if not aux:
		cv2.putText(frame, "No face", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 0, 255), 2)
		return
	lx, ly = aux["lx"], aux["ly"]
	rx, ry = aux["rx"], aux["ry"]
	nx, ny = aux["nx"], aux["ny"]
	cx = (lx + rx) // 2
	cy = (ly + ry) // 2
	cv2.circle(frame, (lx, ly), 3, (0, 255, 0), -1)
	cv2.circle(frame, (rx, ry), 3, (0, 255, 0), -1)
	cv2.circle(frame, (nx, ny), 4, (0, 200, 255), -1)
	cv2.circle(frame, (cx, cy), 3, (255, 255, 0), -1)
	cv2.line(frame, (lx, ly), (rx, ry), (0, 255, 0), 1)
	cv2.line(frame, (cx, 0), (cx, frame.shape[0]), (255, 255, 0), 1)
	if yaw_norm is not None:
		cv2.putText(frame, f"yaw={yaw_norm:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 255, 0), 2)
	if angle is not None:
		cv2.putText(frame, f"angle={angle}", (10, 65), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 255, 255), 2)


def run(camera_index: int, channel: int | None, port: str | None, baud: int, center: int, max_delta: int, gain: float, preview: bool, step_limit: int, send_every_ms: int, width: int | None, height: int | None):
	ser, opened_port = open_serial(port, baud)
	print("Connected:", opened_port)

	tracker = YawEstimator(camera_index, width=width, height=height)
	last_angle = None
	last_send = 0.0

	try:
		while True:
			frame = tracker.read_frame()
			yaw_norm, aux = tracker.compute_yaw_norm(frame)
			angle = None
			if yaw_norm is not None:
				angle = yaw_to_angle(yaw_norm, center_angle=center, max_delta_deg=max_delta, gain=gain)
				if last_angle is None:
					last_angle = angle
				# smooth step
				if abs(angle - last_angle) > step_limit:
					if angle > last_angle:
						angle = last_angle + step_limit
					else:
						angle = last_angle - step_limit
				now = time.time() * 1000
			if angle is not None and (now - last_send) >= send_every_ms and angle != last_angle:
				send_angle(ser, angle, channel)
				last_send = now
				last_angle = angle

			if preview:
				draw_debug(frame, aux, yaw_norm, angle)
				cv2.imshow('Head Yaw Tracking', frame)
				key = cv2.waitKey(1) & 0xFF
				if key == 27:  # ESC
					break
	finally:
		tracker.release()
		try:
			ser.close()
		except Exception:
			pass
		if preview:
			cv2.destroyAllWindows()


if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="Webcam head yaw tracking to servo angle")
	parser.add_argument("--port", type=str, default=None, help="Serial port (e.g., COM5). Auto-detect if omitted.")
	parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
	parser.add_argument("--camera", type=int, default=0, help="Camera index (0=default)")
	parser.add_argument("--channel", type=int, default=0, help="PCA9685 channel 0..15; use -1 for direct ANGLE")
	parser.add_argument("--center", type=int, default=90, help="Center angle (servo neutral)")
	parser.add_argument("--max-delta", type=int, default=60, help="Max deviation from center in degrees")
	parser.add_argument("--gain", type=float, default=1.0, help="Yaw gain multiplier")
	parser.add_argument("--no-preview", action="store_true", help="Disable video preview window")
	parser.add_argument("--step-limit", type=int, default=4, help="Max degrees per update to smooth motion")
	parser.add_argument("--send-every-ms", type=int, default=60, help="Minimum interval between serial sends")
	parser.add_argument("--width", type=int, default=None, help="Camera capture width")
	parser.add_argument("--height", type=int, default=None, help="Camera capture height")
	args = parser.parse_args()

	channel = None if args.channel < 0 else int(args.channel)
	run(camera_index=args.camera,
		channel=channel,
		port=args.port,
		baud=args.baud,
		center=args.center,
		max_delta=args.max_delta,
		gain=args.gain,
		preview=(not args.no_preview),
		step_limit=args.step_limit,
		send_every_ms=args.send_every_ms,
		width=args.width,
		height=args.height)
