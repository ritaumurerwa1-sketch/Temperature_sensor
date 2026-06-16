# ============================================================
# Temperature Display and MQTT Monitoring System
# Trade Code: SPE — Embedded Systems Software Integration
# Author: user273
#
# Requirements: pip install pyserial paho-mqtt
# Python 3.7+ required
# Change SERIAL_PORT below to match your system before running:
#   Windows : "COM3", "COM4", etc.
#   Linux   : "/dev/ttyUSB0" or "/dev/ttyACM0"
#   macOS   : "/dev/cu.usbmodem*"
# ============================================================

import sys
import time
from datetime import datetime

import serial
import paho.mqtt.client as mqtt

# ============================================================
# CONFIGURATION — edit only the values in this section
# ============================================================
SERIAL_PORT   = "COM3"                     # Change to match your system
BAUD_RATE     = 9600
SERIAL_TIMEOUT = 5                         # seconds

MQTT_BROKER   = "157.173.101.159"          # VPS IP address
MQTT_PORT     = 1883                       # Standard MQTT port (no TLS)
MQTT_TOPIC    = "SPE/temperature/user273"  # Trade code + username
MQTT_USERNAME = "user273"                  # VPS username
MQTT_PASSWORD = ""                         # Leave empty — broker may not require auth
CLIENT_ID     = "SPE_user273_monitor"


# ============================================================
# MQTT CALLBACKS
# ============================================================
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Connected to MQTT Broker: {MQTT_BROKER}  (code={rc})")
    else:
        print(f"Failed to connect to MQTT Broker — return code: {rc}")


def on_publish(client, userdata, mid):
    print(f"  Message delivered (mid={mid})")


def on_disconnect(client, userdata, rc):
    print(f"Disconnected from broker (code={rc})")


# ============================================================
# MQTT SETUP
# ============================================================
def create_mqtt_client():
    client = mqtt.Client(client_id=CLIENT_ID)
    client.on_connect    = on_connect
    client.on_publish    = on_publish
    client.on_disconnect = on_disconnect

    if MQTT_USERNAME:
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

    client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
    client.loop_start()   # MQTT runs in background thread
    return client


# ============================================================
# SERIAL SETUP
# ============================================================
def open_serial():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=SERIAL_TIMEOUT)
        time.sleep(2)  # Allow Arduino to reset after serial connect
        print(f"Serial port {SERIAL_PORT} opened at {BAUD_RATE} baud")
        return ser
    except serial.SerialException as e:
        print(f"\n[ERROR] Cannot open serial port '{SERIAL_PORT}':")
        print(f"  {e}")
        print("\nTroubleshooting:")
        print("  Windows : Open Device Manager → Ports (COM & LPT) → find your Arduino")
        print("  Linux   : Run 'ls /dev/ttyUSB* /dev/ttyACM*' to list available ports")
        print("  macOS   : Run 'ls /dev/cu.*' to list available ports")
        print(f"\nThen set SERIAL_PORT in this script to the correct value.")
        sys.exit(1)


# ============================================================
# MAIN
# ============================================================
def main():
    # Startup banner
    print("===========================================")
    print(" SPE Temperature MQTT Monitor")
    print(f" Broker    : {MQTT_BROKER}:{MQTT_PORT}")
    print(f" Topic     : {MQTT_TOPIC}")
    print(f" Serial    : {SERIAL_PORT} @ {BAUD_RATE} baud")
    print("  Dashboard: Open dashboard/index.html in your browser")
    print("===========================================\n")

    # Connect MQTT
    mqtt_client = create_mqtt_client()

    # Small pause to allow on_connect callback to fire
    time.sleep(1)

    # Open serial port
    ser = open_serial()

    print("\nListening for temperature data... (Press Ctrl+C to stop)\n")

    try:
        while True:
            try:
                raw = ser.readline()
                if not raw:
                    continue  # Timeout — no data, loop again

                line = raw.decode("utf-8").strip()

                if not line:
                    continue  # Ignore empty lines silently

                # ---- Machine-readable temperature line ----
                if line.startswith("TEMP:"):
                    value_str = line.split(":", 1)[1]

                    if value_str == "ERROR":
                        print(f"[WARNING] DHT11 read error from Arduino — skipping publish")
                        continue

                    # Parse and validate the float
                    try:
                        temperature = float(value_str)
                    except ValueError:
                        print(f"[WARNING] Could not parse temperature value: '{value_str}'")
                        continue

                    # Publish to MQTT broker with QoS=1
                    result = mqtt_client.publish(
                        MQTT_TOPIC,
                        payload=value_str,
                        qos=1
                    )

                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    print(
                        f"[{timestamp}] \U0001f321  Temperature: {temperature:.1f}\u00b0C"
                        f" | Published \u2192 {MQTT_TOPIC}"
                    )

                # ---- Human-readable debug line from Arduino ----
                elif line.startswith("DHT11 Read OK"):
                    print(f"  [Arduino] {line}")

                # ---- All other serial lines (startup messages, etc.) ----
                else:
                    print(f"  [Serial] {line}")

            except serial.SerialException as e:
                print(f"\n[ERROR] Serial communication error: {e}")
                print("Check that the Arduino is still connected.")
                break

    except KeyboardInterrupt:
        print("\n\nShutting down...")

    finally:
        mqtt_client.loop_stop()
        mqtt_client.disconnect()
        if ser.is_open:
            ser.close()
        print("Disconnected. Goodbye.")
        sys.exit(0)


if __name__ == "__main__":
    main()
