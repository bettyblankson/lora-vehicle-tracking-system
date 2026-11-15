"""
Raspberry Pi 3 LoRa Gateway Receiver
Reconstructed from B.Sc. Computer Engineering Capstone Thesis (April 2022)
Author: Betty Blankson | Ashesi University

Configures the REYAX RYLR896 LoRa transceiver over UART (/dev/serial0),
receives encrypted sensor packets from the vehicle node, and forwards parsed
telemetry to The Things Network (TTN) / Ubidots cloud dashboard.
"""

import os
import time
import serial
import requests

SERIAL_PORT = os.getenv("LORA_SERIAL_PORT", "/dev/serial0")
BAUD_RATE   = int(os.getenv("LORA_BAUD_RATE", "115200"))

ADDRESS     = os.getenv("LORA_ADDRESS", "2")
NODE_ADDR   = os.getenv("LORA_NODE_ADDRESS", "1")
NETWORK_ID  = os.getenv("LORA_NETWORK_ID", "6")
PASSWORD    = os.getenv("LORA_AES_PASS", "YOUR_AES128_HEX_KEY")

UBIDOTS_TOKEN  = os.getenv("UBIDOTS_TOKEN", "YOUR_UBIDOTS_TOKEN")
UBIDOTS_DEVICE = os.getenv("UBIDOTS_DEVICE", "lora-vehicle-tracker")


def set_lora_config(serial_conn: serial.Serial) -> None:
    """Configures the REYAX RYLR896 transceiver module (Thesis Fig. 11)."""
    serial_conn.write(str.encode("AT+ADDRESS=" + str(ADDRESS) + "\r\n"))
    resp = serial_conn.readline().decode("utf-8", errors="ignore").strip()
    print("Address set?", resp)

    serial_conn.write(str.encode("AT+NETWORKID=" + str(NETWORK_ID) + "\r\n"))
    resp = serial_conn.readline().decode("utf-8", errors="ignore").strip()
    print("Network ID set?", resp)

    serial_conn.write(str.encode("AT+CPIN=" + str(PASSWORD) + "\r\n"))
    time.sleep(1)
    resp = serial_conn.readline().decode("utf-8", errors="ignore").strip()
    print("AES-128 password set?", resp)


def parse_telemetry(raw_line: str) -> dict:
    """Parses +RCV=<addr>,<len>,<data>,<RSSI>,<SNR> frames from RYLR896."""
    if not raw_line.startswith("+RCV="):
        return {}
    parts = raw_line.split(",")
    if len(parts) < 3:
        return {}
    # Extract key:value pairs inside payload
    payload_str = ",".join(parts[2:-2]) if len(parts) >= 5 else parts[2]
    data = {}
    for item in payload_str.split(","):
        if ":" in item:
            k, v = item.split(":", 1)
            try:
                data[k.lower()] = float(v)
            except ValueError:
                pass
    return data


def forward_to_cloud(telemetry: dict) -> None:
    """Publishes parsed vehicle telemetry to Ubidots HTTP API."""
    if not telemetry or UBIDOTS_TOKEN == "YOUR_UBIDOTS_TOKEN":
        return
    url = f"https://industrial.api.ubidots.com/api/v1.6/devices/{UBIDOTS_DEVICE}/"
    headers = {"X-Auth-Token": UBIDOTS_TOKEN, "Content-Type": "application/json"}
    try:
        r = requests.post(url, json=telemetry, headers=headers, timeout=5)
        print(f"[CLOUD] Uploaded telemetry -> HTTP {r.status_code}")
    except requests.RequestException as exc:
        print(f"[CLOUD] Upload failed: {exc}")


def main() -> None:
    print(f"[GATEWAY] Opening serial port {SERIAL_PORT} @ {BAUD_RATE} baud...")
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2) as ser:
        time.sleep(1)
        set_lora_config(ser)
        print("[GATEWAY] Listening for LoRa packets from vehicle node...")
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if line:
                print(f"[RX] {line}")
                telemetry = parse_telemetry(line)
                if telemetry:
                    forward_to_cloud(telemetry)


if __name__ == "__main__":
    main()
