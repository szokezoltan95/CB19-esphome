# 🏗️ CB19 Gate ESPHome Component

Custom **ESPHome component** for integrating **CB19 gate controllers** via UART, with full support for:

- remote control (open / close / stop / pedestrian)
- real-time state feedback
- dual-motor position tracking
- obstruction & photocell detection

---

## 📡 Overview

This project reverse-engineers the CB19 communication protocol and provides a clean ESPHome integration.

The controller communicates via a **text-based serial protocol** containing:

- command acknowledgements
- human-readable state messages
- binary-like status frames (`ACK RS`)

Example:
```
ACK FULL OPEN
Opening
ACK RS: 60,64,CC,86,3E,00,84,1F,0C
```

---

## ⚙️ Features

### ✅ Control
- Full open
- Full close
- Stop
- Pedestrian open

### ✅ Feedback
- Opening / Closing / Opened / Closed / Stopped
- Pedestrian states
- Obstruction detection
- Photocell detection

### ✅ Position tracking
- Independent tracking of both gate wings
- Calculated percentage position
- Suitable for Home Assistant `cover` entity

---

## 🔌 Hardware assumptions

- UART connection to CB19 controller
- Typical baud rate: `9600` (verify on your device)
- Logic-level serial (use level shifting if needed)

---

## 🧠 Protocol understanding

### RS frame format

```
ACK RS: b0,b1,b2,b3,b4,b5,b6,b7,b8
```

| Byte | Meaning |
|------|--------|
| b0 | status flag (60 = normal, 62 = photocell active) |
| b1 | constant (64) |
| b2 | state type |
| b3 | **motor1 position** |
| b4 | dynamic data (unknown) |
| b5 | unknown |
| b6 | **motor2 position** |
| b7 | dynamic data (unknown) |
| b8 | unknown |

---

## 🚪 Motor model (important!)

We intentionally avoid "master/slave" terminology.

### 🔹 motor1
- Primary motor
- Works in:
  - single-wing setups
  - pedestrian mode
- **Opens first**
- Position stored in **byte 3**

### 🔹 motor2
- Secondary motor (second wing)
- Only active in dual-wing mode
- **Closes first**
- Position stored in **byte 6**

---

## 🚶 Pedestrian mode

- Only **motor1 moves**
- motor2 position remains constant
- Easily detectable from RS frames

---

## 📊 Position mapping

Typical observed values:

| State | Raw value |
|------|----------|
| Closed | ~1–4 |
| Open | ~220–226 |

Linear scaling is sufficient:

```
percent = (value - min) / (max - min) * 100
```

---

## 🚦 State byte (b2)

| Value | Meaning |
|------|--------|
| CC | Moving |
| AA | Idle (end position) |
| EE | Stopped / error / obstruction |
| DB / FB | Pedestrian mode |

---

## 🚨 Safety detection

### Photocell
- `b0 = 62` instead of `60`

### Obstruction
- State transitions to `Stopped`
- RS frame switches to `EE`

### Closing obstruction behavior
- Gate reverses (Closing → Opening)

---

## 🧱 Architecture

```
UART → Parser → State Machine → ESPHome Entities
```

---

## 📦 Entities exposed

### Cover
- Main gate entity
- open / close / stop
- optional position reporting

### Button
- Pedestrian open

### Sensors
- motor1 raw position
- motor2 raw position
- motor1 %
- motor2 %
- overall %

### Binary sensors
- moving
- fully open
- fully closed
- photocell active
- obstruction detected

### Text sensors
- last state
- last RS frame
- debug info

---

## 🔧 Example ESPHome config

```yaml
external_components:
  - source: https://github.com/szokezoltan95/CB19-esphome
    components: [cb19_gate]

uart:
  id: gate_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

cb19_gate:
  id: gate1
  uart_id: gate_uart

cover:
  - platform: cb19_gate
    name: "Gate"

button:
  - platform: cb19_gate
    pedestrian_open:
      name: "Pedestrian Open"
```

---

## 🧪 Development status

### ✔ Planned baseline
- UART communication
- command sending
- RS parsing
- position decoding
- basic state machine

### 🔄 In progress
- ESPHome entity integration
- cover position sync
- stability improvements

## 🔮 Future plans

This project is not limited to basic gate control. The long-term goal is to reproduce as much of the original TMT app functionality as possible through ESPHome and Home Assistant.

Planned areas of development:

- full decoding of configuration-related protocol messages
- reading all controller configuration values (`F` codes)
- exposing controller settings as Home Assistant entities
- changing gate controller settings directly from Home Assistant
- implementing the full TMT app configuration workflow
- support for timing, delay, and behavior parameters
- support for obstruction, photocell, and safety-related settings
- remote management of paired hardware remotes
- reading and editing remote-control related data if supported by the protocol
- improved protocol coverage beyond basic command/status handling
- better distinction between user stop, safety stop, and obstruction events
- optional advanced diagnostics for installation and servicing
- documenting all known protocol messages and field meanings

The ultimate target is not just a UART bridge, but a full-featured CB19 integration layer that allows both everyday control and deep controller configuration.

---

## ⚠️ Disclaimer

This project is based on **reverse engineering** of CB19 protocol.

- No official documentation was used
- Behavior may vary between firmware versions
- Use at your own risk

---

## 🤝 Contributing

PRs, logs, and testing feedback are welcome.

---

## 📜 License

MIT (recommended)
