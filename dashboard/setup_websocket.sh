#!/bin/bash
# =============================================================
# setup_websocket.sh
# Enables WebSocket support on the VPS Mosquitto MQTT broker.
#
# Run once on the VPS:
#   ssh user273@157.173.101.159
#   bash setup_websocket.sh
# =============================================================

set -e   # Exit immediately on any error

echo "============================================="
echo " SPE — Mosquitto WebSocket Setup"
echo " VPS: 157.173.101.159"
echo "============================================="

# ── 1. Backup existing main config ──────────────────────────
echo ""
echo "[1/4] Backing up existing mosquitto.conf ..."
sudo cp /etc/mosquitto/mosquitto.conf /etc/mosquitto/mosquitto.conf.backup
echo "      Backup saved to /etc/mosquitto/mosquitto.conf.backup"

# ── 2. Write WebSocket config into conf.d ───────────────────
echo ""
echo "[2/4] Writing WebSocket listener config ..."
sudo tee /etc/mosquitto/conf.d/websocket.conf > /dev/null <<EOF
# Standard MQTT listener
listener 1883
protocol mqtt

# WebSocket listener (used by browser dashboard)
listener 9001
protocol websockets

# Allow unauthenticated connections (required for browser clients)
allow_anonymous true
EOF
echo "      Written to /etc/mosquitto/conf.d/websocket.conf"

# ── 3. Open port 9001 in firewall (if ufw is active) ────────
echo ""
echo "[3/4] Opening port 9001 in firewall (ufw) if active ..."
if sudo ufw status | grep -q "Status: active"; then
  sudo ufw allow 9001/tcp
  echo "      Port 9001/tcp allowed."
else
  echo "      ufw not active — skipping firewall rule."
  echo "      If using another firewall, open TCP port 9001 manually."
fi

# ── 4. Restart Mosquitto and show status ────────────────────
echo ""
echo "[4/4] Restarting Mosquitto ..."
sudo systemctl restart mosquitto
sleep 2
sudo systemctl status mosquitto --no-pager

echo ""
echo "============================================="
echo " Done! Mosquitto is now listening on:"
echo "   MQTT      → port 1883  (Arduino / Python client)"
echo "   WebSocket → port 9001  (Browser dashboard)"
echo ""
echo " Next step:"
echo "   Open  dashboard/index.html  in your browser"
echo "   while  pc_client/mqtt_client.py  is running."
echo "============================================="
