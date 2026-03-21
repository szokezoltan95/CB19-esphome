# CB19 ESPHome Gate Controller

Custom ESPHome component for controlling CB19 gate controllers over UART (RS protocol).

This project replaces the original WiFi module and provides full integration with Home Assistant.

---

## 🚀 Features

| Feature | Description |
|--------|------------|
| Gate control | Open / Close / Stop / Pedestrian |
| Position tracking | Real-time feedback from RS frames |
| Status detection | Opening, Closing, Stopped, Obstruction |
| Safety | Photocell + obstruction detection |
| Calibration | Adjustable from Home Assistant |
| Integration | Native ESPHome + HA support |

---

## ⚠️ Disclaimer

This is a reverse-engineered protocol. Use at your own risk.

---

## 🔧 Hardware Requirements

| Component | Notes |
|----------|------|
| ESP32 | Tested on esp32_devkitc_v4 |
| UART | Direct connection to gate controller |
| Voltage divider | REQUIRED |

---

## 🔌 Wiring

⚠️ The gate controller uses higher voltage levels than ESP32.

### Voltage Divider (REQUIRED)

| Element | Value |
|--------|------|
| R1 | 10kΩ |
| R2 | 18kΩ |

### Connection

```
Gate TX ---[ R1 10k ]---+--- ESP RX
                        |
                     [ R2 18k ]
                        |
                       GND
```

### Notes

- NEVER connect Gate TX directly to ESP RX
- ESP TX → Gate RX is usually safe directly

---

## 🧩 Custom PCB (Fusion 360)

File location:

```
docs/cb19-gate-espboard.fbrd
```

Important:
- R1 = 10kΩ
- R2 = 18kΩ

---

## ⚙️ ESPHome Configuration

Fully working example:

```
examples/cb19_example.yaml
```

👉 Only WiFi credentials need to be added.

---

## 📡 Communication Protocol

### Command format

```
COMMAND;src=P0031DA2\r\n
```

### Commands

| Command | Description |
|--------|------------|
| FULL OPEN | Open gate |
| FULL CLOSE | Close gate |
| STOP | Stop movement |
| PED OPEN | Pedestrian mode |
| RS | Status request |

---

## 📥 Responses

| Response | Meaning |
|---------|--------|
| ACK | Command accepted |
| NAK | Command rejected |
| ACK RS | Status frame |

---

## 📊 RS Frame Structure

Example:

```
ACK RS:60,64,CC,0D,3E,07,01,40,00
```

### Key bytes

| Byte | Meaning |
|------|--------|
| 0 | Photocell |
| 2 | Motion state |
| 3 | Motor 1 position |
| 6 | Motor 2 position |

### Motion states

| Value | Meaning |
|------|--------|
| CC | Moving |
| AA | Idle |
| EE | Obstruction |

---

## 🎯 Calibration

Observed behavior:

| Direction | Start offset |
|----------|-------------|
| Opening | ~56% |
| Closing | ~44% |

### HA Entities

- Opening Start Percent
- Closing Start Percent

---

## 🔄 Polling Strategy

| State | Interval |
|------|---------|
| Moving | 200 ms |
| After stop | 500 ms (10s) |
| Idle | 60 s |

---

## 📜 License

MIT License
