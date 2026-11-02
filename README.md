# light-triggered-arduino-lamp-alarm
Arduino-based lamp alarm that detects a change in phone screen brightness with a photoresistor and uses an Arduino to signal to a lamp through an IoT relay.

I have to be up before sunrise (around 5 am) for AFROTC PT. Waking up has two parts, sound and light. My phone alarm already handles the sound, so this project handles the light: when my phone's alarm screen lights up, a lamp turns on.

Status: working prototype. TODO (case revision, final lamp timing)

![Finished circuit with the lamp on](docs/images/Finished_Circuit_With_Lamp.jpg)

Demo video 1:
Demo video 2: [docs/videos/arduino_lamp_alarm_demo.mp4](docs/videos/arduino_lamp_alarm_demo.mp4)
All photos: [docs/images](docs/images)

## How it works

```
phone screen -> photoresistor -> voltage divider -> A0
    -> Arduino (threshold + 10 s check) -> pin 7
    -> IoT Power Relay -> lamp
```

The phone sits in a 3D printed case with a photoresistor (LDR) facing the screen. When the screen lights up, the LDR's resistance drops. The Arduino can't measure resistance directly, so the LDR is part of a voltage divider and the Arduino measures the voltage at pin A0 instead. When the voltage stays high enough for 10 seconds straight, the Arduino sends a signal on pin 7 to an IoT Power Relay, which turns the lamp on.

The relay plugs into a normal 120 V wall outlet. The Arduino is powered by a barrel jack adapter plugged into the relay's always-on outlet, and the lamp plugs into one of its normally-off outlets.

The lamp is meant to stay on for 15 minutes, or longer if the screen is still lit. Right now the code is set to 1 minute for testing. Taking the phone out of the case is how I "turn off" the alarm.

## Hardware

### Schematic

![Schematic](hardware/schematic/arduino_lamp_alarm.svg)

KiCad source: [hardware/schematic/](hardware/schematic/). PNG versions: [full schematic](docs/images/KiCAD_Photos/Arduino_Lamp_Alarm_Full_KiCAD_Schematic.png), [close-up](docs/images/KiCAD_Photos/TODO_snippet_file_name.png)

The relay is drawn as a 2-pin connector (J1) since it's an off-the-shelf module, and the 120 V side (relay to lamp) isn't shown. J2 is the barrel jack adapter that powers the Arduino.

### BOM

| Part | Part number | Qty | Source | Notes |
|---|---|---|---|---|
| Elegoo UNO R3 | TODO | 1 | TODO | Select "Arduino Uno" in the IDE |
| Photoresistor (LDR) | TODO | 1 | TODO | Mounted in the phone case |
| 150k resistor | TODO | 1 | TODO | Measured ~150k |
| Digital Loggers IoT Power Relay | TODO | 1 | TODO | Control input 3.3-48 VDC |
| 9V DC barrel jack adapter | TODO | 1 | TODO | Powers the Arduino from the relay's always-on outlet |
| Breadboard | TODO | 1 | TODO | |
| Jumper wires | | ~6 | TODO | |
| USB-B cable | | 1 | Came with the board | For uploading code |
| Lamp | | 1 | | Any normal 120 V lamp |
| Phone case | | 1 | 3D printed | Fusion 360, see below |

### Wiring

| From | To |
|---|---|
| Photoresistor leg 1 | 5V |
| Photoresistor leg 2 | A0 and R1 |
| R1 other leg | GND |
| Relay control + | D7 |
| Relay control - | GND |
| Barrel jack adapter | Arduino power jack (VIN) |

Nothing I wired touches 120 V, only the relay's low voltage control terminal. Unplug the relay before changing any wires.

### Circuit math

A0 reads the voltage across R1. Since the divider and the ADC both run off the same 5 V, the supply cancels out:

```
V_A0    = (5 * (R1 / (R1 + R_LDR)))
reading = (V_A0 * (1024 / 5))
        = ((1024 * R1) / (R1 + R_LDR))

R_LDR   = (R1 * ((1024 - reading) / reading))
```

These are `LDROhmsToADC()` and `ADCToLDROhms()` in the code. My hand derivation: [Arduino_Firmware_Logic_Explanation.jpg](docs/images/Arduino_Firmware_Logic_Explanation.jpg)

### Case

Designed in Fusion 360 and 3D printed. I went through about 10 versions. It has to hold the phone in the same spot every time and keep the photoresistor right over the screen.

| Early prototype | Fusion 360 model | Later iteration |
|---|---|---|
| ![Early prototype](docs/images/Early_Phone_Enclosure_Prototype.jpg) | ![Fusion 360 model](docs/images/Modeling_Prototype_Phone_Enclosure.jpg) | ![Later iteration](docs/images/Phone_Enclosure_Later_Iteration.jpg) |

Files: [TODO folder](TODO) (`external_phone_case_v1.1` as .f3d, .3mf, and .stl)

TODO: a sentence or two on what changed between versions

## Firmware

- Arduino IDE TODO (Help -> About)
- Arduino AVR Boards TODO (Tools -> Board -> Boards Manager)

To flash it:

1. Open `firmware/TODO/TODO.ino`
2. Tools -> Board -> Arduino Uno
3. Tools -> Port -> pick the board's COM port (mine is COM3).
4. Close the Serial Monitor if it's open, then Upload
5. Open the Serial Monitor at 9600 baud. On startup it prints three numbers that should be about/roughly 512, 452, and 301. If they aren't, R1 isn't 150k, the constants were changed, or the ambient lighting is way too bright.

## Calibration / configuration

All of these are at the top of the sketch.

| Constant | Value | What it does |
|---|---|---|
| `LDR_OHMS_SCREEN_ON` | `190000.0` | Screen counts as on below this resistance |
| `LDR_OHMS_SCREEN_OFF` | `360000.0` | Screen counts as off above this resistance |
| `PHONE_SCREEN_DURATION_MS` | `(10UL * 1000UL)` | How long the screen has to stay lit |
| `LAMP_DURATION_MS` | `(15UL * 60UL * 1000UL)` | //need to change this soon
| `RELAY_PIN` | `7` | |
| `LDR_PIN` | `A0` | |

To calibrate for a different phone or room, watch the readings in the Serial Monitor with the screen off and then on, and set the two thresholds in the gap between them. For daylight, try `LDR_OHMS_SCREEN_ON = 315000.0`. That also worked at night for me.

## Design decisions

- I used an IoT Power Relay because it's the safest way to switch 120 V. The 120 V wiring is sealed inside the relay and the control input is isolated, so the Arduino side never touches it.
- R1 is on the ground side so A0 reads the voltage across R1 directly. `analogRead()` measures from the pin to GND.
- Thresholds are in ohms instead of ADC counts since ohms actually mean something. They get converted to counts once in `setup()`.
- There are two thresholds (hysteresis) instead of one so the state doesn't flicker when the reading sits near a single value.
- The screen has to stay lit for 10 seconds so headlights or a notification don't set it off.
- Timing uses `millis()` instead of `delay()` so the loop never blocks.
- Each measurement starts with a throwaway `analogRead()`. Taking a throwaway reading first is a common practice for getting a more accurate value, especially with a high resistance like 150k.

## Results

R1 = 150k. Resistance is calculated from the reading.

| Condition | Reading | R_LDR (approx) | Lamp on? |
|---|---|---|---|
| Dark room, screen off | 3 | 51 M | No |
| Dark room, screen on | 872-875 | 26 k | Yes |
| Daylight, screen off | 251-276 | 406-462 k | No |
| Daylight, screen on | 373-381 | 253-262 k | No |

In a dark room, which is how I actually use it, it triggers reliably. 
In daylight the screen-on and screen-off readings are much closer together, but it can still work. 
The readings also are somewhat inconsistent in the daylight.

## Known issues & future plans
- The lamp duration is still set to 1 minute for testing.

- [ ] Case update: thicker floor, light shield around the sensor

- [ ] Maybe add a sound check along with the light check

## Repo layout

```
firmware/TODO/         Arduino sketch
hardware/schematic/    KiCad project, schematic SVG and PDF
TODO/                  Phone case files (Fusion 360, 3MF, STL)
docs/images/           Photos, schematic PNGs, hand derivation
docs/videos/           Demo video
```

## License

MIT, see [LICENSE](LICENSE).
