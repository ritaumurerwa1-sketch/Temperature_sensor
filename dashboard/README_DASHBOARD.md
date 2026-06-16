# Dashboard Setup and Usage

Live web dashboard for the SPE Temperature Monitoring System.
Connects directly to the MQTT broker via WebSocket and displays temperature readings in real time — no web server, no Node.js, no install required.

---

## Step 1 — Enable WebSocket on VPS (one time only)

SSH into your VPS and run the setup script:

```bash
ssh user273@157.173.101.159
bash setup_websocket.sh
```

The script will:
- Back up the existing Mosquitto config
- Add a WebSocket listener on port 9001
- Open port 9001 in the firewall (if `ufw` is active)
- Restart Mosquitto

You only need to do this once. After that, the broker keeps WebSocket support across reboots.

---

## Step 2 — Open the Dashboard

Simply double-click `dashboard/index.html` in File Explorer (Windows) or Finder (Mac), or drag it into any browser.

No web server needed — it runs as a local HTML file.

---

## Step 3 — Start the Full System

Run all three components in order:

1. **Upload Arduino sketch** — `arduino/temperature_lcd.ino` → Arduino Uno
2. **Start Python client** — in a terminal:
   ```bash
   python pc_client/mqtt_client.py
   ```
3. **Watch the dashboard** — temperature updates every 2 seconds automatically

---

## How It Works

```
Arduino DHT11
     │
     │  USB Serial (9600 baud, "TEMP:XX.X")
     ▼
PC Python Client  (mqtt_client.py)
     │
     │  MQTT publish → port 1883
     ▼
Mosquitto Broker  (157.173.101.159)
     │
     │  WebSocket → port 9001
     ▼
Browser Dashboard (index.html)
     │
     └─ Chart.js line chart (last 20 readings)
     └─ Current temperature (large display)
     └─ Min / Max / Average stats
```

- Dashboard subscribes to topic `SPE/temperature/user273`
- Each published message triggers an instant UI update
- Chart auto-scrolls, keeping the last 20 readings in view
- No page refresh needed

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| Dashboard shows "Disconnected" | Run `setup_websocket.sh` on the VPS first |
| Dashboard connects but no data | Make sure `mqtt_client.py` is running |
| Chart not showing | Open browser console (F12) and check for errors |
| Port 9001 refused | Run on VPS: `sudo ufw allow 9001` then `sudo systemctl restart mosquitto` |
| Mosquitto not restarting | Check config: `sudo mosquitto -c /etc/mosquitto/mosquitto.conf -v` |
| Mixed content error in browser | Use `ws://` not `wss://` — broker has no TLS configured |

---

## Files in this Folder

| File | Purpose |
|------|---------|
| `index.html` | Self-contained dashboard — open directly in browser |
| `setup_websocket.sh` | One-time VPS script to enable Mosquitto WebSocket support |
| `README_DASHBOARD.md` | This file |
