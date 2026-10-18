// Robix autonomous obstacle-avoiding bot
// Ultrasonic: TRIG D15, ECHO D2
// IR left  (D5)  : 0 = object detected, 1 = clear
// IR right (D35) : 0 = object detected, 1 = clear

#include <Arduino.h>

#define RPWM1 26
#define LPWM1 25
#define RPWM2 12
#define LPWM2 27
#define pwmPin1 14
#define pwmPin2 13
#define FANENA 33
#define FANIN1 32
#define FANIN2 23

#define IR_RIGHT 35
#define IR_LEFT 5

#define TRIG 15
#define ECHO 2
#define FLAME_SENSOR 34

// ---- Tunable settings ----
#define SPEED 220  // motor speed 0-255
#define STOP_CM 20 // stop and check IRs when closer than this

// ---------------- Motor helpers ----------------
void setMotors(bool r1, bool l1, bool r2, bool l2, int spd)
{
  digitalWrite(RPWM1, r1);
  digitalWrite(LPWM1, l1);
  digitalWrite(RPWM2, r2);
  digitalWrite(LPWM2, l2);
  analogWrite(pwmPin1, spd);
  analogWrite(pwmPin2, spd);
}

void stopBot() { setMotors(0, 0, 0, 0, 0); }
void moveForward() { setMotors(0, 1, 0, 1, SPEED); }
void moveBackward() { setMotors(1, 0, 1, 0, SPEED); }
void turnLeft() { setMotors(0, 1, 1, 0, SPEED); }
void turnRight() { setMotors(1, 0, 0, 1, SPEED); }

// ---------------- Ultrasonic ----------------
// Distance in cm, or -1 if no echo (nothing in range)
float readDistanceCm()
{
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  if (duration == 0)
    return -1;

  return duration * 0.0343 / 2.0;
}

bool isBlocked(float d)
{
  return (d > 0 && d < STOP_CM);
}

void setup()
{
  Serial.begin(115200);

  pinMode(RPWM1, OUTPUT);
  pinMode(LPWM1, OUTPUT);
  pinMode(RPWM2, OUTPUT);
  pinMode(LPWM2, OUTPUT);
  pinMode(pwmPin1, OUTPUT);
  pinMode(pwmPin2, OUTPUT);

  // Fan kept off
  pinMode(FANENA, OUTPUT);
  pinMode(FANIN1, OUTPUT);
  pinMode(FANIN2, OUTPUT);
  digitalWrite(FANIN1, LOW);
  digitalWrite(FANIN2, LOW);
  digitalWrite(FANENA, LOW);

  pinMode(IR_RIGHT, INPUT);
  pinMode(IR_LEFT, INPUT);
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  stopBot();
  Serial.println("Robix auto bot starting in 2 seconds...");
  delay(2000);
}

void loop()
{
  // ==========================================
  // Flame detection override logic
  // ==========================================
  int flameVal = digitalRead(FLAME_SENSOR);

  if (flameVal == LOW)
  { // Assuming LOW means flame detected
    stopBot();
    Serial.println("Flame detected! Extinguishing...");

    // Turn on fan
    digitalWrite(FANIN1, HIGH); // Test 1 polarity
    digitalWrite(FANIN2, LOW);
    digitalWrite(FANENA, HIGH);

    delay(100);
    return; // Skip normal movement to stay stopped and extinguish
  }
  else
  {
    // Ensure fan is off when no flame is detected
    digitalWrite(FANIN1, LOW);
    digitalWrite(FANIN2, LOW);
    digitalWrite(FANENA, LOW);
  }
  // ==========================================

  float dist = readDistanceCm();
  int irLeft = digitalRead(IR_LEFT);
  int irRight = digitalRead(IR_RIGHT);

  Serial.print("Ultrasonic: ");
  if (dist < 0)
    Serial.print("out of range");
  else
  {
    Serial.print(dist, 1);
    Serial.print(" cm");
  }
  Serial.print("  |  IR left: ");
  Serial.print(irLeft);
  Serial.print("  |  IR right: ");
  Serial.println(irRight);

  bool blockedFront = isBlocked(dist);
  bool blockedLeft = (irLeft == 0);
  bool blockedRight = (irRight == 0);

  // 1. Blocked on all sides (or front and both sides) - back up and turn around
  if (blockedFront && blockedLeft && blockedRight)
  {
    Serial.println("Trapped! Reversing and turning around...");
    moveBackward();
    delay(400);
    turnRight();
    delay(1200); // Increased to execute a full U-turn
  }
  // 2. Front is blocked, check sides to choose turn direction
  else if (blockedFront)
  {
    if (!blockedLeft)
    {
      Serial.println("Front blocked. Left is clear -> turning left");
      turnLeft();
      delay(400); // Increased from 200
    }
    else
    {
      Serial.println("Front blocked. Right is clear -> turning right");
      turnRight();
      delay(400); // Increased from 200
    }
  }
  // 3. Narrow corridor: blocked on both sides but front is clear
  else if (blockedLeft && blockedRight)
  {
    Serial.println("Blocked on both sides -> moving straight to escape");
    moveForward();
  }
  // 4. Diagonal obstacles detected by IR sensors while moving
  else if (blockedLeft)
  {
    Serial.println("Object on left -> steering right");
    turnRight();
    delay(300); // Increased from 150
  }
  else if (blockedRight)
  {
    Serial.println("Object on right -> steering left");
    turnLeft();
    delay(300); // Increased from 150
  }
  // 4. Path is completely clear
  else
  {
    moveForward();
  }

  delay(30); // Small delay for stability before next sensor reading
}
