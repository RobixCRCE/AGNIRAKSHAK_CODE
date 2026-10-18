# AGNIRAKSHAK: Autonomous Firefighting Bot

**Reference code for _AgniRakshak: The Autonomous Firefighting Bot Competition_, presented by Robix CRCE.**

This sketch runs a fully autonomous bot on an **ESP32 DevKit V1**. The bot drives around an obstacle arena, avoids walls with an ultrasonic sensor and two IR sensors, and when its flame sensor spots a candle, it stops and turns on a fan to blow it out.

> This is a **starting point**, not a finished winning bot. Tune the values for your own motors, sensors and arena.

---

## 🏁 The Competition in Short

- Build a bot that fights fire **on its own**. No remote control and no touching it once the run starts.
- The arena is about **8 ft × 8 ft** with walls and obstacles, and **4 lit candles**, each in its own bay.
- Put out every candle, then drive into the **finish zone**, all within **5 minutes**.
- A candle counts only if it **stays out for 5 seconds**. Use a **fan/blower** or **water**.
- The team with the **most candles** wins. Ties go to the **fastest time**.
- Bot limits: **30 × 30 × 30 cm**, **4 kg**, **12 V max**.

---

##  Bill of Materials

| Qty | Part | Notes |
|---|---|---|
| 4 | BO / gear motors (or any) | Wire the 2 left motors together and the 2 right motors together |
| 4 | Wheels | Rubber tyres grip textured floors better |
| 1 | Motor driver: L298N / BTS7960 / any | **Take 2 if you skip the relay**: the 2nd driver runs the fan (this code does that) |
| 1 | 1-channel relay | Switches the fan/pump on and off |
| 1 | IR obstacle sensor | **3×** if using only IR (front, left, right). This code uses 2 (left and right) |
| 1 | Ultrasonic sensor (HC-SR04) | **3×** if using only ultrasonic. This code uses 1 (front) |
| 1 | 12 V DC fan | *Option A* extinguisher |
| 1 | Water pump + reservoir | *Option B* extinguisher (water counts toward 4 kg) |
| 1 | Microcontroller | Arduino Nano or any board with enough pins. **This code is written for an ESP32 DevKit V1** |
| 1 | Chassis | Must fit in 30 × 30 × 30 cm |
| 1 | 3S Li-ion / LiPo battery | A 3S pack is 12.6 V when full, but the limit is 12 V, so **only charge it to 12 V** |
| 1 | Flame sensor | IR flame module with digital output |

**Also handy:** jumper wires, an on/off switch, a 12 V → 5 V buck converter, screws and zip ties, and a multimeter.

---

## 🔌 Pin Map (ESP32 DevKit V1)

| Code name | GPIO | Connects to |
|---|---|---|
| `RPWM1` | 26 | Motor driver IN1 (right motors) |
| `LPWM1` | 25 | Motor driver IN2 (right motors) |
| `RPWM2` | 12 | Motor driver IN3 (left motors) |
| `LPWM2` | 27 | Motor driver IN4 (left motors) |
| `pwmPin1` | 14 | Motor driver ENA (right speed) |
| `pwmPin2` | 13 | Motor driver ENB (left speed) |
| `FANENA` | 33 | Fan driver ENA (or relay IN) |
| `FANIN1` | 32 | Fan driver IN1 |
| `FANIN2` | 23 | Fan driver IN2 |
| `IR_LEFT` | 5 | Left IR sensor OUT |
| `IR_RIGHT` | 35 | Right IR sensor OUT |
| `TRIG` | 15 | Ultrasonic TRIG |
| `ECHO` | 2 | Ultrasonic ECHO (**through a 1 kΩ / 2 kΩ voltage divider**) |
| `FLAME_SENSOR` | 34 | Flame sensor D0 |

**Power:** battery → switch → 12 V line to both drivers. A buck converter turns 12 V into 5 V for the ESP32's VIN pin and the ultrasonic sensor. IR and flame sensors run on 3.3 V. **Connect all GNDs together.**

>  **Wiring notes**
> - The HC-SR04 echo pin outputs 5 V, but ESP32 pins are 3.3 V. Always use a voltage divider.
> - GPIO 34 and 35 are input-only, which is fine for sensors.
> - GPIO 2, 5, 12 and 15 are boot pins. If the board won't boot or upload while wired, unplug those connections, upload, then reconnect (or move them to other pins).
> - On the L298N, remove the ENA/ENB jumpers so speed control works through GPIO 14 and 13.

---

##  How It Works

Every loop, the bot does this:

1. **Fire check (highest priority).** If the flame sensor reads `LOW`, the bot stops, turns the fan on, and skips driving. The fan keeps running as long as the flame is detected.
2. **Read sensors.** It reads the front distance (ultrasonic) and the left and right IR sensors, and prints them to the Serial Monitor.
3. **Decide where to go:**

| Front | Left | Right | Action |
|:---:|:---:|:---:|---|
| ■ | ■ | ■ | Reverse, then U-turn |
| ■ | □ | any | Turn left |
| ■ | ■ | □ | Turn right |
| □ | ■ | ■ | Go straight (narrow corridor) |
| □ | ■ | □ | Steer right |
| □ | □ | ■ | Steer left |
| □ | □ | □ | Forward |

■ = blocked  □ = clear

---

##  Getting Started

1. Install the **Arduino IDE 2.x**.
2. Add ESP32 support: **Boards Manager → "esp32" by Espressif** (v2.0.11 or newer, which `analogWrite()` needs).
3. Select **Tools → Board → ESP32 Dev Module** and the right COM port.
4. Open the `.ino` file from this repo.
5. Match the pin numbers to your wiring, then **Upload**.
6. Open the **Serial Monitor at 115200 baud** to watch the sensor readings.
7. The bot waits **2 seconds** after power-on, so you can place it in the start zone.

---

## Tuning

| Setting | Where | Default | What it does |
|---|---|---|---|
| `SPEED` | `#define` | `220` | Motor speed, 0–255 |
| `STOP_CM` | `#define` | `20` | Front distance (cm) that counts as blocked |
| Echo timeout | `pulseIn(...)` | `30000` µs | No echo in this time = nothing in front |
| Flame level | `if (flameVal == LOW)` | `LOW` | Reading when fire is seen |
| IR level | `(irLeft == 0)` | `0` | Reading when an object is close |
| Reverse time | trapped case | `400` ms | How long to back up |
| U-turn time | trapped case | `1200` ms | How long to spin around |
| Turn time | front-blocked case | `400` ms | Turn toward the clear side |
| Steer time | side-blocked case | `300` ms | Small nudge away from an obstacle |
| Fan pulse | flame block | `100` ms | Fan time per loop while fire is seen |
| Loop delay | end of `loop()` | `30` ms | Pause between sensor reads |
| Start delay | end of `setup()` | `2000` ms | Wait after power-on |

**Tune in this order:** motor directions → sensor readings → flame + fan → `SPEED` → `STOP_CM` → turn timings.

### Example Serial Output
```
Ultrasonic: 54.2 cm  |  IR left: 1  |  IR right: 1
Ultrasonic: 18.7 cm  |  IR left: 1  |  IR right: 0
Front blocked. Left is clear -> turning left
Flame detected! Extinguishing...
```

---

## Troubleshooting

| Problem | Fix |
|---|---|
| Bot drives backwards or spins | Swap the two wires of the motor pair that runs the wrong way |
| Fan blows toward the bot | Swap `FANIN1` / `FANIN2` (HIGH ↔ LOW) |
| Flame never detected / always detected | Adjust the flame sensor's sensitivity knob in arena lighting |
| Ultrasonic always "out of range" | Check the echo divider and that the sensor gets 5 V |
| Board won't upload while wired | Unplug the wires on GPIO 2 / 12 / 15, upload, then reconnect |
| Turns overshoot or undershoot | Adjust the turn delays, and re-check them as the battery drains |

---

##  Ideas to Level Up

- Keep the fan on for 1–2 s **after** the flame goes out, because the rules need it to stay out for 5 s.
- Add left and right flame sensors and steer toward the stronger signal to **find** candles.
- Add a downward IR sensor to detect the taped **finish zone** and stop.
- Use `millis()` to track the 5-minute run time.

---

##  Official Guide

See **AgniRakshak – The Official Guide Part 1: The Code** (PDF) for a block-by-block explanation of this code, the bill of materials and the circuit diagram.

---

## 🙌 All the Best!

Build it, test it, break it, fix it, and test it again. We can't wait to see your bot take on the flames.

**Team Robix · CRCE**
