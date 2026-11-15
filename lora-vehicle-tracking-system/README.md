# Anti-Theft Vehicle Tracking & Remote Actuation System Using LoRa Technology

**B.Sc. Computer Engineering Capstone Thesis (April 2022) — Ashesi University**  
**Author:** Betty Blankson  
**Advisor:** Mr. Kofi Adu Labi  

---

## Overview

This repository reconstructs the hardware architecture, embedded firmware, and gateway telemetry scripts for my 2022 B.Sc. Computer Engineering capstone thesis at Ashesi University. The system provides low-power, wide-area vehicle localization, intrusion/hazard detection, and remote engine cutoff without relying on recurring cellular (GSM/GPRS) data subscriptions.

### Key Capabilities
* **Multi-Sensor Intrusion & Hazard Sensing:** Monitors vehicle proximity/motion (`APDS9960`, distance `< 30 cm`), cabin temperature (`HTS221`, threshold `> 30°C` / `70°C`), and smoke/gas thresholds on an **Arduino Nano 33 BLE Sense**.
* **GPS Localization & Software UART Multiplexing:** Integrates a **u-blox NEO-6M GPS** module alongside a **REYAX RYLR896 LoRa transceiver** (`868–915 MHz`), using software serial multiplexing (`UART gpsSerial` on pins D3/D4) to overcome single-hardware-UART constraints.
* **Headless Raspberry Pi LoRa Gateway:** Receives encrypted (`AES-128`) LoRa packets over serial (`pyserial`) on a headless **Raspberry Pi 3** and forwards telemetry to **The Things Network (TTN)** and **Ubidots**.
* **Closed-Loop Remote Actuation:** Allows the vehicle owner to trigger real-time GPS activation or remote fuel-injector/DC-motor cutoff via transistor/relay switching from the web dashboard.

---

## System Architecture

```text
[NEO-6M GPS] ──(SoftUART D3/D4)──┐
[APDS9960 Proximity] ──(I2C)─────┼──► [Arduino Nano 33 BLE Sense] ──(Hardware UART)──► [REYAX RYLR896 LoRa]
[HTS221 Temperature] ──(I2C)─────┤                                                             │
[5V Relay + DC Motor] ◄─(GPIO)───┘                                                    (LoRa RF 868-915 MHz)
                                                                                               ▼
[Ubidots Web Dashboard] ◄──(MQTT/Webhook)── [The Things Network (TTN)] ◄──(Python)── [Raspberry Pi 3 + RYLR896]
```

---

## Repository Structure

```text
├── firmware/
│   └── sensor_node/
│       └── vehicle_tracker_node.ino    # Arduino Nano 33 BLE Sense sensor, GPS, LoRa & relay firmware
├── gateway/
│   ├── lora_gateway.py                 # Raspberry Pi 3 pyserial LoRa packet receiver & parser
│   └── requirements.txt                # Python gateway dependencies
├── docs/
│   └── THESIS_SUMMARY.md               # Hardware specs, Pugh matrices, thresholds & experimental results
├── .gitignore
└── README.md
```

---

## Hardware Bill of Materials (BOM)

| Subsystem | Component | Key Specifications |
| :--- | :--- | :--- |
| **Sensor Node MCU** | Arduino Nano 33 BLE Sense | 64 MHz, 3.3V logic, 1MB Flash, 256KB SRAM |
| **GPS Module** | u-blox NEO-6M | 50 channels, 2m position accuracy, 5 Hz update rate |
| **LoRa Transceiver (×2)** | REYAX RYLR896 | 868–915 MHz, AES-128 encryption, up to 15 km range |
| **Proximity Sensor** | APDS9960 (onboard) | Gesture/proximity detection (`< 30 cm` intrusion threshold) |
| **Temperature Sensor** | HTS221 (onboard) | ±0.5°C accuracy (`> 30°C` test / `> 70°C` hazard alert) |
| **Actuator & Driver** | 5V Relay + Transistor + TT DC Motor | Simulates remote fuel-injector / engine cutoff |
| **Gateway Compute** | Raspberry Pi 3 Model B | Headless Raspberry Pi OS, Python `pyserial` |

---

## Note on Archived Code
The original 2022 project files were reconstructed from the archived B.Sc. Capstone Thesis document (*Design and Implementation of an Anti-Theft Vehicle Tracking System Using LoRa Technology*, Ashesi University, April 2022), preserving the exact AT-command configurations, pin mappings, sensor thresholds, and serial multiplexing architecture documented in the thesis.
