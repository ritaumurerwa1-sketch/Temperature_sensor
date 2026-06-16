# Temperature Display and MQTT Monitoring System

**Trade Code: SPE — Embedded Systems Software Integration**
**Author: user273**

This project reads temperature data from a DHT11 sensor connected to an Arduino Uno, displays it on a 16×2 LCD screen, and transmits the values over USB serial to a Python client on a PC. The Python client then publishes each reading to an MQTT broker running on a Linux VPS, making the data available to any MQTT subscriber in real time.

---

## System Architecture

```
┌─────────────┐   DATA pin 7    ┌──────────────────┐
│   DHT11     │────────────────▶│                  │
│  Sensor     │                 │   Arduino Uno    │──────▶  16x2 LCD Display
└─────────────┘                 │                  │         (Name + Temp)
                                └────────┬─────────┘
                                         │
                                    USB Serial
                                    9600 baud
                                 Format: TEMP:XX.X
                                         │
                                         ▼
                                ┌──────────────────┐
                                │   PC Python      │
                                │   Client         │
                                │   mqtt_client.py │
                                └────────┬─────────┘
                                         │
                                   MQTT publish
                                   TCP port 1883
                                         │
                                         ▼
                         ┌──────────────────────────────┐
                         │   Mosquitto MQTT Broker      │
                         │   VPS: 157.173.101.159       │
                         │   Topic: SPE/temperature/    │
                         │          user273             │
                         └──────┬───────────────────────┘
                                │
                          WebSocket port 9001
                                │
                                ▼
                         ┌──────────────────┐
                         │  Web Dashboard   │
                         │  index.html      │
                         │  (browser)       │
                         │                  │
                         │  • Live chart    │
                         │  • Current temp  │
                         │  • Min/Max/Avg   │
                         └──────────────────┘
```

---

## Hardware Requirements

| Component       | Specification                    | Quantity |
|-----------------|----------------------------------|----------|
| Arduino Uno     | R3 or compatible                 | 1        |
| DHT11           | Temperature & Humidity Sensor    | 1        |
| I2C 16×2 LCD    | HD44780 + I2C backpack (PCF8574) | 1        |
| Resistor        | 10 kΩ pull-up for DHT11 DATA pin | 1        |
| Breadboard      | Standard 830 tie-point           | 1        |
| USB Cable       | Type-A to Type-B (Arduino)       | 1        |
| Jumper Wires    | Male-to-Male                     | ~10      |

---

## Wiring Diagram

### DHT11 → Arduino

| DHT11 Pin | Arduino Pin    | Note                             |
|-----------|----------------|----------------------------------|
| VCC       | 5V             | Power                            |
| GND       | GND            | Ground                           |
| DATA      | Digital Pin 7  | 10 kΩ pull-up resistor to 5V     |

### I2C 16×2 LCD → Arduino

The LCD uses an I2C backpack module — only 4 wires needed (power + I2C bus).

| LCD I2C Pin | Arduino Pin | Note                                          |
|-------------|-------------|-----------------------------------------------|
| VCC         | 5V          | Power                                         |
| GND         | GND         | Ground                                        |
| SDA         | A4          | I2C data line                                 |
| SCL         | A5          | I2C clock line                                |

> **I2C Address:** Default is `0x27`. If the display stays blank after uploading,
> change the address to `0x3F` in the `.ino` file:
> `LiquidCrystal_I2C lcd(0x3F, 16, 2);`
>
> **Contrast** is adjusted via the small trimmer potentiometer on the I2C backpack itself — no external potentiometer needed.

---

## Software Requirements

| Software / Library                   | Version       | Notes                                          |
|--------------------------------------|---------------|------------------------------------------------|
| Arduino IDE                          | 1.8.x or 2.x  | Download from arduino.cc                       |
| DHT sensor library (Adafruit)        | Latest        | Install via Arduino Library Manager            |
| Adafruit Unified Sensor              | Latest        | Dependency for DHT library                     |
| LiquidCrystal_I2C (Frank de Brabander) | Latest      | Install via Arduino Library Manager            |
| Python                               | 3.7+          | Must be installed on PC                        |
| pyserial                             | Latest        | `pip install pyserial`                         |
| paho-mqtt                            | Latest        | `pip install paho-mqtt`                        |

---

## Step-by-Step Setup and Installation

### Step 1 — Install Arduino Libraries

1. Open Arduino IDE.
2. Go to **Sketch → Include Library → Manage Libraries**.
3. Search for **"DHT sensor library"** by Adafruit → click **Install**. When prompted, also install **Adafruit Unified Sensor** (dependency).
4. Search for **"LiquidCrystal I2C"** by Frank de Brabander → click **Install**.

### Step 2 — Upload Arduino Sketch

1. Open `arduino/temperature_lcd.ino` in Arduino IDE.
2. Connect your Arduino Uno via USB.
3. Go to **Tools → Board → Arduino Uno**.
4. Go to **Tools → Port** and select the correct COM port.
5. Click **Upload** (→ arrow button).
6. After upload, open **Serial Monitor** (Tools → Serial Monitor) at **9600 baud** to verify output.

### Step 3 — Identify Your Serial Port

| OS      | How to find the port                                              |
|---------|-------------------------------------------------------------------|
| Windows | Device Manager → Ports (COM & LPT) → look for "Arduino Uno"      |
| Linux   | Run: `ls /dev/ttyUSB* /dev/ttyACM*`                              |
| macOS   | Run: `ls /dev/cu.*` — look for `cu.usbmodem*`                    |

### Step 4 — Install Python Dependencies

```bash
pip install pyserial paho-mqtt
```

### Step 5 — Configure Serial Port

Open `pc_client/mqtt_client.py` and change line:

```python
SERIAL_PORT = "COM3"   # Windows example
```

To your actual port:

```python
# Windows
SERIAL_PORT = "COM4"

# Linux
SERIAL_PORT = "/dev/ttyUSB0"

# macOS
SERIAL_PORT = "/dev/cu.usbmodem14101"
```

### Step 6 — Run the Python Client

```bash
cd temperature-mqtt-system
python pc_client/mqtt_client.py
```

### Step 7 — Verify MQTT Messages on the Broker

From any machine with `mosquitto-clients` installed, subscribe to the topic:

```bash
mosquitto_sub -h 157.173.101.159 -p 1883 -t "SPE/temperature/user273" -v
```

You should see messages arriving every 2 seconds:

```
SPE/temperature/user273 24.5
SPE/temperature/user273 24.6
```

---

## Communication Protocol Details

### Serial (Arduino → PC)

| Parameter   | Value                       |
|-------------|-----------------------------|
| Interface   | USB Serial (UART over USB)  |
| Baud Rate   | 9600                        |
| Data Format | `TEMP:XX.X\n`               |
| Interval    | Every 2 seconds             |
| Error Line  | `TEMP:ERROR\n`              |

Example serial output:
```
TEMP:24.5
DHT11 Read OK - Temp: 24.5C
TEMP:24.6
DHT11 Read OK - Temp: 24.6C
```

### MQTT (PC → Broker)

| Parameter   | Value                          |
|-------------|--------------------------------|
| Broker IP   | 157.173.101.159                |
| Port        | 1883                           |
| Topic       | `SPE/temperature/user273`      |
| QoS         | 1 (at least once delivery)     |
| Payload     | Temperature string e.g. `24.5` |
| Client ID   | `SPE_user273_monitor`          |

---

## Expected Console Output

```
===========================================
 SPE Temperature MQTT Monitor
 Broker : 157.173.101.159:1883
 Topic  : SPE/temperature/user273
 Serial : COM3 @ 9600 baud
===========================================

Connected to MQTT Broker: 157.173.101.159  (code=0)
Serial port COM3 opened at 9600 baud

Listening for temperature data... (Press Ctrl+C to stop)

  [Serial] SPE Temperature Monitor — user273
  [Serial] Initialising DHT11 and LCD...
  [Serial] Ready. Sending readings every 2s.
  [Arduino] DHT11 Read OK - Temp: 24.5C
[2024-01-15 10:23:45] 🌡  Temperature: 24.5°C | Published → SPE/temperature/user273
  Message delivered (mid=1)
  [Arduino] DHT11 Read OK - Temp: 24.6C
[2024-01-15 10:23:47] 🌡  Temperature: 24.6°C | Published → SPE/temperature/user273
  Message delivered (mid=2)
```

Screenshots of the running system are stored in the `screenshots/` folder.

---

## LCD Display

```
┌────────────────┐
│user273         │  ← Row 0: Candidate name (static, 7 chars)
│Temp: 24.5 °C   │  ← Row 1: Live temperature reading
└────────────────┘
```

> **Note on scrolling:** The scroll logic is implemented in `handleScroll()`. It activates automatically if the candidate name exceeds 16 characters. For `user273` (7 chars) it remains static. To test scrolling, replace `candidateName` in the `.ino` with a string longer than 16 characters.

---

## Troubleshooting

| Problem                      | Likely Cause              | Solution                                                       |
|------------------------------|---------------------------|----------------------------------------------------------------|
| Serial port not found        | Wrong COM port            | Check Device Manager (Windows) or `ls /dev/tty*` (Linux/Mac)  |
| DHT11 read fails repeatedly  | Loose wiring / no pull-up | Check 10 kΩ pull-up resistor on DATA pin; reseat wires        |
| LCD shows nothing (backlit)  | Wrong I2C address         | Change `0x27` to `0x3F` in the `.ino` and re-upload           |
| LCD backlight off / blank    | Bad SDA/SCL wiring        | Confirm SDA→A4 and SCL→A5; check I2C backpack power           |
| LCD shows garbage characters | Wrong I2C address         | Try `0x3F`; or run an I2C scanner sketch to detect address     |
| MQTT not connecting          | Broker unreachable        | Ping `157.173.101.159`; verify port 1883 is open              |
| No scroll on LCD             | Name ≤ 16 characters      | Expected behaviour — scroll only triggers when name > 16 chars |
| Python import error          | Missing libraries         | Run `pip install pyserial paho-mqtt`                           |

---

## Dashboard

A self-contained web dashboard is included in the `dashboard/` folder.

- Open `dashboard/index.html` directly in any browser — no web server needed
- Requires WebSocket support enabled on the VPS (run `setup_websocket.sh` once — see `dashboard/README_DASHBOARD.md`)
- Connects to the broker at `ws://157.173.101.159:9001` via MQTT over WebSocket
- Shows live updating line chart of the last 20 readings
- Displays current temperature, session minimum, maximum, and average
- Connection status indicator shows green when broker is reachable
- Auto-reconnects if the connection drops

---

## Repository Structure

```
temperature-mqtt-system/
│
├── arduino/
│   └── temperature_lcd.ino       # Arduino sketch: DHT11 + I2C LCD + serial output
│
├── pc_client/
│   └── mqtt_client.py            # Python: reads serial, publishes to MQTT broker
│
├── dashboard/
│   ├── index.html                # Browser dashboard: live chart, temp, min/max/avg
│   ├── setup_websocket.sh        # Run once on VPS to enable Mosquitto WebSocket
│   └── README_DASHBOARD.md       # Dashboard-specific setup instructions
│
├── screenshots/
│   └── .gitkeep                  # Placeholder — add runtime screenshots here
│
└── README.md                     # This file
```

---

## Author

**User:** user273
**Trade Code:** SPE — Embedded Systems Software Integration
**MQTT Broker:** 157.173.101.159:1883
**MQTT Topic:** `SPE/temperature/user273`
