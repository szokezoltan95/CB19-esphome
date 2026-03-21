# CB19 ESPHome Gate Controller

Custom ESPHome component for controlling CB19 gate controllers over UART (RS protocol).

This project replaces the original WiFi module and provides full integration with Home Assistant.

---

## 🚀 Features

- Full gate control (open / close / stop / pedestrian mode)
- Real-time position tracking
- RS protocol decoding
- Obstruction and photocell detection
- Adaptive polling (safe for controller)
- Live calibration of gate position report from Home Assistant
- ESPHome native integration

---

## ⚠️ Disclaimer

This is a reverse-engineered protocol implementation.  
Use at your own risk.

---

## 🔧 Hardware Requirements

- ESP32 (tested on esp32_devkitc_v4)
- UART connection to CB19 controller
- Voltage level adaptation (REQUIRED)

---

## 🔌 Wiring (VERY IMPORTANT)

Voltage divider REQUIRED:

Gate TX -- R1 --+-- ESP RX
                |
               R2
                |
               GND

R1 = 10kΩ  
R2 = 18kΩ  

ESP TX may be directly wired to GATE RX

---

## 🧩 Custom ESP PCB (Autodesk Fusion)

docs/cb19-gate-espboard.fbrd

Suitable for esp32_devkitc_v4

---

## ⚙️ ESPHome Configuration

See fully working example:

example/cb19_example.yaml

Only WiFi credentials need to be added.

---

## 📡 Communication Protocol

Command format:

COMMAND;src=P0031DA2\r\n

Examples:

FULL OPEN;src=P0031DA2  
FULL CLOSE;src=P0031DA2  
STOP;src=P0031DA2  
PED OPEN;src=P0031DA2  
RS;src=P0031DA2  

---

## 📜 License

MIT License
