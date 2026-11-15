# Thesis Technical Summary

**Title:** Design and Implementation of an Anti-Theft Vehicle Tracking System Using LoRa Technology  
**Degree:** Bachelor of Science in Computer Engineering, Ashesi University  
**Date:** April 27, 2022  
**Author:** Betty Blankson  
**Supervisor:** Mr. Kofi Adu Labi  

## 1. Engineering Design Decisions (Pugh Matrix Selections)
* **Communication Protocol:** Evaluated Wi-Fi, Bluetooth, LoRa, GSM/GPRS, and ZigBee. Selected **LoRa** (`+3` net score) for low power consumption, zero cellular data cost, and long-range coverage (up to 15 km rural line-of-sight).
* **GPS Receiver:** Evaluated u-blox NEO-6M, u-blox NEO-7, Copernicus II DIP, and SparkFun Venus GPS. Selected **u-blox NEO-6M** (`50 channels`, `2m` positional accuracy, `5 Hz` update rate).
* **Microcontroller:** Selected **Arduino Nano 33 BLE Sense** (`64 MHz`, `3.3V` logic, built-in `APDS9960` proximity/gesture sensor and `HTS221` temperature/humidity sensor) paired with a **Raspberry Pi 3** headless Linux gateway.

## 2. Threshold & Actuation Matrix
| Sensor / Module | Trigger Condition | Dashboard Alert | Remote Actuator Response |
| :--- | :--- | :--- | :--- |
| **APDS9960 Proximity** | Object distance `< 30 cm` (`r,g,b,a < 5`) | `Motion detected` | Enable GPS tracking; prompt user to cut off DC motor |
| **HTS221 Temperature** | Cabin temp `> 30°C` (test) / `> 70°C` (hazard) | `Vehicle temperature high` | Turn OFF DC motor (simulated fuel injector) via relay |
| **Gas / Smoke Sensor** | Analog sensor reading `> 300` | `Smoke Detected` | Turn OFF DC motor via relay |

## 3. Hardware Integration Challenges Solved
* **UART Pin Multiplexing:** Because the Arduino Nano 33 BLE Sense exposes only one hardware UART (`Serial1`), digital pins `D3` and `D4` were configured as a dedicated software UART (`UART gpsSerial`) to support simultaneous NEO-6M GPS NMEA parsing and REYAX RYLR896 LoRa AT-command transmission.
* **AES-128 Encrypted LoRa Link:** Configured matching `AT+ADDRESS`, `AT+NETWORKID`, and `AT+CPIN` AES-128 keys across both the Arduino vehicle node and the Python `pyserial` Raspberry Pi gateway before bridging payloads to The Things Network (TTN) and Ubidots.
