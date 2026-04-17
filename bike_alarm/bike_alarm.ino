#include <Keypad.h>
#include <Wire.h>
#include <MPU6050.h>

// ---------- KEYPAD ----------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {23, 26, 13, 14};
byte colPins[COLS] = {32, 4, 15, 19};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------- ULTRASONIC ----------
const int trigPin = 5;
const int echoPin = 18;

// ---------- OUTPUT ----------
const int ledPin = 25;
const int buzzerPin = 27;

// ---------- MPU ----------
MPU6050 mpu;
bool mpuOK = false;

float baseAngleX = 0, baseAngleY = 0;
int motionCount = 0;
const float ANGLE_THRESHOLD = 30;

// ---------- PASSWORD ----------
String correctPassword = "1234#";
String input = "";

// ---------- STATE ----------
bool locked = true;
bool disarmedMode = false;
bool alarmActive = false;

// ---------- ALARM ----------
void alarmON() {
  if (!alarmActive) Serial.println("🚨 ALARM ON");
  alarmActive = true;
  digitalWrite(ledPin, HIGH);
  digitalWrite(buzzerPin, LOW);
}

void alarmOFF() {
  if (alarmActive) Serial.println("✅ ALARM OFF");
  alarmActive = false;
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, HIGH);
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  // I2C
  Wire.begin(21, 22);
  Wire.setClock(100000); // 🔥 stability fix

  // MPU
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU CONNECTED");
    mpuOK = true;

    delay(1000);
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    baseAngleX = atan2((float)ay, (float)az) * 180.0 / PI;
    baseAngleY = atan2(-(float)ax, (float)az) * 180.0 / PI;

  } else {
    Serial.println("MPU NOT DETECTED (will ignore)");
    mpuOK = false;
  }

  // Pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(buzzerPin, HIGH);
  digitalWrite(ledPin, LOW);

  Serial.println("SYSTEM LOCKED");
}

// ---------- DISTANCE ----------
float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 200;

  return duration * 0.034 / 2;
}

// ---------- MOTION ----------
bool isMotionDetected(float &dX_out, float &dY_out) {

  if (!mpuOK) return false;

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // ❌ if dead → ignore safely
  if (ax == 0 && ay == 0 && az == 0) {
    Serial.println("⚠️ MPU DISCONNECTED");
    motionCount = 0;
    return false;
  }

  float angleX = atan2((float)ay, (float)az) * 180.0 / PI;
  float angleY = atan2(-(float)ax, (float)az) * 180.0 / PI;

  float dX = abs(angleX - baseAngleX);
  float dY = abs(angleY - baseAngleY);

  dX_out = dX;
  dY_out = dY;

  if (dX > ANGLE_THRESHOLD || dY > ANGLE_THRESHOLD) {
    motionCount++;
  } else {
    motionCount = 0;
  }

  return (motionCount >= 2); // small filtering
}

// ---------- LOOP ----------
void loop() {

  char key = keypad.getKey();

  // RE-ARM
  if (disarmedMode && key) {
    Serial.println("RE-ARMING...");
    delay(500);

    locked = true;
    disarmedMode = false;
    alarmOFF();

    Serial.println("SYSTEM LOCKED");
    input = "";
    return;
  }

  // KEYPAD
  if (key) {
    Serial.print("Key: ");
    Serial.println(key);

    input += key;
    if (input.length() > 6) input = "";

    if (key == '#') {
      if (input == correctPassword) {
        Serial.println("SYSTEM DISARMED");
        locked = false;
        disarmedMode = true;
        alarmOFF();
      } else {
        Serial.println("WRONG PASSWORD");
      }
      input = "";
    }
  }

  if (disarmedMode) {
    delay(200);
    return;
  }

  // SENSOR LOGIC
  if (locked) {

    float distance = getDistance();

    float dX = 0, dY = 0;
    bool motion = isMotionDetected(dX, dY);

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" | Motion: ");
    Serial.print(motion ? "YES" : "NO");
    Serial.print(" | dX: ");
    Serial.print(dX);
    Serial.print(" | dY: ");
    Serial.println(dY);

    // 🔥 STABLE ULTRASONIC (no flicker)
    static int closeCount = 0;

    if (distance > 2 && distance < 5) {
      closeCount++;
    } else {
      closeCount = 0;
    }

    bool proximityAlert = (closeCount >= 3);

    // 🔥 FINAL LOGIC
    if (proximityAlert || motion) {
      alarmON();
    } else {
      alarmOFF();
    }
  }

  delay(100);
}
