# 🚲 Bike Theft Detection Alarm System

An ESP32-based bike theft alarm that uses ultrasonic proximity sensing and MPU6050 motion/tilt detection to trigger an alarm when tampering is detected. The system is armed/disarmed via a 4×4 matrix keypad with a password.

## 📽️ Demo

[▶️ Watch the project demo](https://drive.google.com/file/d/1ZMGFibhWqV4UxkrIJQ5h9d2j1P_xW9__/view?usp=sharing)

---

## Hardware Used

| Component | Purpose |
|---|---|
| ESP32 | Main microcontroller |
| HC-SR04 Ultrasonic Sensor | Proximity detection (2–5 cm range) |
| MPU6050 IMU | Tilt / motion detection |
| 4×4 Matrix Keypad | Password input for arm/disarm |
| Active Buzzer | Alarm output |
| LED | Visual alarm indicator |

---

## How It Works

### Detection
The system runs two independent sensors in parallel:

- **Ultrasonic sensor** — triggers if an object stays within 2–5 cm for 3 consecutive readings (`closeCount >= 3`), filtering out brief noise.
- **MPU6050 accelerometer** — records a baseline tilt angle at startup. If the bike tilts more than 5° from baseline for 2 consecutive readings (`motionCount >= 2`), the alarm triggers.

### Alarm
When a threat is detected, the buzzer activates (active-low logic) and the LED turns on. Both sensors must be clear for the alarm to stop.

### Arming / Disarming
- **Default state:** System is locked and monitoring.
- **Disarm:** Enter the password followed by `#` on the keypad (default: `1234#`).
- **Re-arm:** Press any key while in disarmed mode.

---

## 📌 Pin Configuration

| Pin | GPIO | Description |
|---|---|---|
| Keypad rows | 23, 26, 13, 14 | Row scan outputs |
| Keypad cols | 32, 4, 15, 19 | Column read inputs |
| Ultrasonic TRIG | 5 | Trigger pulse output |
| Ultrasonic ECHO | 18 | Echo pulse input |
| LED | 25 | Visual alarm indicator |
| Buzzer | 27 | Audible alarm (active-low) |
| MPU6050 SDA | 21 | I2C data |
| MPU6050 SCL | 22 | I2C clock |

---

## 📚 Libraries Required

Install these via Arduino Library Manager:

- [`Keypad`](https://github.com/Chris--A/Keypad) by Mark Stanley
- [`MPU6050`](https://github.com/ElectronicCats/mpu6050) by Electronic Cats

---

## 🚀 Setup & Upload

1. Install [Arduino IDE](https://www.arduino.cc/en/software) and add ESP32 board support.
2. Install the required libraries listed above.
3. Open `bike_alarm.ino` in Arduino IDE.
4. Select board: **ESP32 Dev Module**
5. Select the correct COM port.
6. Upload the sketch.
7. Open Serial Monitor at **115200 baud** to view sensor readings and system state.

---

## 🔐 Changing the Password

The default password is `1234#`. To change it, edit this line in the code:

```cpp
String correctPassword = "1234#";
```

> ⚠️ The password is stored in plaintext in the source code. For a production system, consider storing it in ESP32 NVS (non-volatile storage).

---

## 📁 File Structure

```
bike-theft-alarm/
├── bike_alarm.ino     # Main Arduino sketch
├── demo.mp4           # Project demo video
└── README.md          # This file
```

---

## 🛠️ Known Limitations & Future Improvements

- Password is hardcoded in source (not secure for production)
- No remote alert — a GSM/Wi-Fi push notification module could be added
- Baseline tilt is recorded once at startup; parking on uneven ground may cause false positives
- No battery backup (would need a power management module)

---

## 👤 Author

Made as part of an embedded systems project.
