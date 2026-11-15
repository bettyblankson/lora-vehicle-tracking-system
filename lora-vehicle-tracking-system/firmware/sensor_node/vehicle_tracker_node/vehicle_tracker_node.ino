/**
 * Anti-Theft Vehicle Tracking & Remote Actuation Node
 * Reconstructed from B.Sc. Computer Engineering Capstone Thesis (April 2022)
 * Author: Betty Blankson | Ashesi University
 *
 * Hardware:
 *  - Arduino Nano 33 BLE Sense (onboard APDS9960 proximity & HTS221 temp/humidity)
 *  - u-blox NEO-6M GPS Module (Software UART on D3=TX, D4=RX)
 *  - REYAX RYLR896 LoRa Transceiver (Hardware Serial1)
 *  - 5V Relay / Transistor switching circuit driving DC Motor (simulated fuel cutoff)
 */

#include <Arduino_APDS9960.h>
#include <Arduino_HTS221.h>
#include <TinyGPSPlus.h>

// Software UART pins for NEO-6M GPS (since Serial1 is dedicated to LoRa)
#define GPS_RX_PIN 4
#define GPS_TX_PIN 3

// Actuator & Threshold Definitions (from Thesis Table 6 & Section 5.2)
#define RELAY_MOTOR_PIN   7
#define TEMP_THRESHOLD_C  30.0f   // 30C test threshold (70C operational hazard threshold)
#define PROX_COLOR_THRESH 5       // RGB/Ambient threshold indicating < 30cm proximity

// LoRa Network Configuration (Placeholder credentials)
const String LORA_ADDRESS    = "1";
const String GATEWAY_ADDRESS = "2";
const String LORA_NETWORK_ID = "6";
const String LORA_AES_PASS   = "YOUR_AES128_HEX_KEY";

UART gpsSerial(digitalPinToPinName(GPS_TX_PIN), digitalPinToPinName(GPS_RX_PIN), NC, NC);
TinyGPSPlus gps;

bool motorEnabled = true;
bool gpsEnabled   = true;

void configureLoRa() {
  Serial1.print("AT+ADDRESS=" + LORA_ADDRESS + "\r\n");
  delay(200);
  Serial1.print("AT+NETWORKID=" + LORA_NETWORK_ID + "\r\n");
  delay(200);
  Serial1.print("AT+CPIN=" + LORA_AES_PASS + "\r\n");
  delay(200);
  Serial1.print("AT+CPIN?\r\n");
  delay(200);
}

void sendLoRaPacket(const String &payload) {
  String cmd = "AT+SEND=" + GATEWAY_ADDRESS + "," + String(payload.length()) + "," + payload + "\r\n";
  Serial1.print(cmd);
  Serial.println("[LoRa TX] " + payload);
}

void handleDownlinkCommand(const String &rx) {
  if (rx.indexOf("MOTOR_OFF") >= 0) {
    motorEnabled = false;
    digitalWrite(RELAY_MOTOR_PIN, LOW);
    Serial.println("[ACTUATOR] Remote engine cutoff executed (DC Motor OFF)");
  } else if (rx.indexOf("MOTOR_ON") >= 0) {
    motorEnabled = true;
    digitalWrite(RELAY_MOTOR_PIN, HIGH);
    Serial.println("[ACTUATOR] Engine relay restored (DC Motor ON)");
  } else if (rx.indexOf("GPS_ON") >= 0) {
    gpsEnabled = true;
  } else if (rx.indexOf("GPS_OFF") >= 0) {
    gpsEnabled = false;
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);   // REYAX RYLR896 hardware UART
  gpsSerial.begin(9600);   // NEO-6M GPS software UART

  pinMode(RELAY_MOTOR_PIN, OUTPUT);
  digitalWrite(RELAY_MOTOR_PIN, HIGH); // Default: vehicle operational

  if (!APDS.begin()) {
    Serial.println("[ERROR] Failed to initialize APDS9960 proximity sensor");
  }
  if (!HTS.begin()) {
    Serial.println("[ERROR] Failed to initialize HTS221 temperature sensor");
  }

  configureLoRa();
  Serial.println("[SYSTEM] Anti-Theft Vehicle Tracker Initialized");
}

void loop() {
  // 1. Poll GPS stream
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // 2. Check incoming LoRa commands from Raspberry Pi Gateway
  if (Serial1.available()) {
    String incoming = Serial1.readStringUntil('\n');
    handleDownlinkCommand(incoming);
  }

  // 3. Read environmental & proximity sensors
  float temperature = HTS.readTemperature();
  float humidity    = HTS.readHumidity();

  int r = 255, g = 255, b = 255, a = 255;
  bool motionDetected = false;
  if (APDS.colorAvailable()) {
    APDS.readColor(r, g, b, a);
    if (r < PROX_COLOR_THRESH && g < PROX_COLOR_THRESH && b < PROX_COLOR_THRESH) {
      motionDetected = true;
    }
  }

  bool highTemp = (temperature > TEMP_THRESHOLD_C);

  double lat = gps.location.isValid() ? gps.location.lat() : 0.0;
  double lng = gps.location.isValid() ? gps.location.lng() : 0.0;

  // 4. Format and transmit telemetry packet
  String telemetry = "TEMP:" + String(temperature, 2) +
                     ",HUM:" + String(humidity, 2) +
                     ",MOTION:" + String(motionDetected ? 1 : 0) +
                     ",ALERT_TEMP:" + String(highTemp ? 1 : 0) +
                     ",LAT:" + String(lat, 6) +
                     ",LNG:" + String(lng, 6);

  sendLoRaPacket(telemetry);
  delay(5000);
}
