// ESP32 Arduino sketch: control servos via serial commands
// Commands (newline-terminated):
//   ANGLE <angle>                -> set single on-board servo angle (0-180)
//   INC <delta> / DEC <delta>    -> adjust angle for single on-board servo
//   CHANGLE <ch> <angle>         -> set PCA9685 channel angle (0..15, 0-180)
//   CHINC <ch> <delta>           -> increment angle for channel
//   CHDEC <ch> <delta>           -> decrement angle for channel
// Use with ESP32Servo for direct PWM, and optional PCA9685 for 16 channels.

#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ---------- Configuration ----------
static const int SERVO_PIN = 14; // direct servo on ESP32 (optional)
static const int DEFAULT_ANGLE = 90;
static const int BAUD = 115200;

// PCA9685 config
static const uint8_t PCA9685_ADDR = 0x40; // default address
static const int PCA9685_FREQ = 50;       // 50Hz standard
static const int SERVO_MIN_US = 500;      // microseconds
static const int SERVO_MAX_US = 2500;     // microseconds

Servo servo;
int currentAngle = DEFAULT_ANGLE;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PCA9685_ADDR);
int chAngle[16];

static int clampInt(int v, int lo, int hi) {
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

static int angleToUs(int angle) {
	angle = clampInt(angle, 0, 180);
	return SERVO_MIN_US + (int)((SERVO_MAX_US - SERVO_MIN_US) * (angle / 180.0f));
}

static void setAngle(int angle) {
	currentAngle = clampInt(angle, 0, 180);
	int pulseUs = angleToUs(currentAngle);
	servo.writeMicroseconds(pulseUs);
}

static void setChannelAngle(int ch, int angle) {
	if (ch < 0 || ch > 15) return;
	chAngle[ch] = clampInt(angle, 0, 180);
	int pulseUs = angleToUs(chAngle[ch]);
	int periodUs = 1000000 / PCA9685_FREQ; // 20,000us
	int offCount = map(pulseUs, 0, periodUs, 0, 4095);
	if (offCount < 0) offCount = 0;
	if (offCount > 4095) offCount = 4095;
	pwm.setPWM(ch, 0, offCount);
}

static void processLine(String line) {
	line.trim();
	if (line.length() == 0) return;
	// Split into tokens
	int s1 = line.indexOf(' ');
	String cmd = (s1 < 0) ? line : line.substring(0, s1);
	String rest = (s1 < 0) ? String("") : line.substring(s1 + 1);
	cmd.toUpperCase();

	bool ok = false;
	if (cmd == "ANGLE" && rest.length() > 0) {
		int v = rest.toInt();
		setAngle(v);
		ok = true;
	} else if ((cmd == "INC" || cmd == "DEC") && rest.length() > 0) {
		int d = rest.toInt();
		if (cmd == "DEC") d = -d;
		setAngle(currentAngle + d);
		ok = true;
	} else if (cmd == "CHANGLE") {
		int s = rest.indexOf(' ');
		if (s > 0) {
			int ch = rest.substring(0, s).toInt();
			int v = rest.substring(s + 1).toInt();
			setChannelAngle(ch, v);
			ok = true;
		}
	} else if (cmd == "CHINC" || cmd == "CHDEC") {
		int s = rest.indexOf(' ');
		if (s > 0) {
			int ch = rest.substring(0, s).toInt();
			int d = rest.substring(s + 1).toInt();
			if (cmd == "CHDEC") d = -d;
			setChannelAngle(ch, chAngle[ch] + d);
			ok = true;
		}
	}

	if (ok) {
		Serial.print("OK ");
		Serial.print(cmd);
		Serial.print(" ");
		Serial.println((cmd.startsWith("CH")) ? "" : String(currentAngle));
	} else {
		Serial.println("ERR UNKNOWN_CMD");
	}
}

void setup() {
	Serial.begin(BAUD);
	while (!Serial) { delay(10); }

	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);

	servo.setPeriodHertz(50);
	servo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
	setAngle(DEFAULT_ANGLE);

	// PCA9685 init
	Wire.begin();
	pwm.begin();
	pwm.setPWMFreq(PCA9685_FREQ);
	for (int i = 0; i < 16; ++i) {
		chAngle[i] = DEFAULT_ANGLE;
		setChannelAngle(i, chAngle[i]);
	}
}

void loop() {
	static String line;
	while (Serial.available() > 0) {
		char c = (char)Serial.read();
		if (c == '\n' || c == '\r') {
			if (line.length() > 0) {
				processLine(line);
				line = "";
			}
		} else {
			line += c;
		}
	}
}
