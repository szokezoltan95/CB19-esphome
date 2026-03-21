# CB19 ESPHome Gate Controller

Custom ESPHome component for controlling CB19 gate controllers over UART (RS protocol).

This project replaces the original WiFi module and enables full local control and monitoring of the gate through Home Assistant.

---

## Overview

The CB19 gate controller communicates over a UART-based ASCII protocol.  
This project implements that protocol inside ESPHome, allowing you to:

- fully control the gate (open, close, stop, pedestrian mode)
- read real-time position and movement state
- detect obstruction and photocell events
- integrate everything seamlessly into Home Assistant

---

## Features

- Open / Close / Stop / Pedestrian control
- Real-time position tracking (based on RS frames)
- Motion state detection (opening, closing, idle)
- Obstruction and photocell monitoring
- Adaptive polling to avoid overloading the controller
- Live calibration from Home Assistant
- Fully local operation (no cloud)

---

## Hardware Requirements

To build the system you will need:

- ESP32 (tested on `esp32_devkitc_v4`)
- UART connection to the CB19 controller
- Voltage level adaptation (mandatory)

---

## Wiring

The CB19 controller uses higher voltage logic levels than the ESP32.

Connecting it directly **will damage the ESP32**.

### Voltage divider (required)

Connection:
```
    Gate TX ---[10k]---+--- ESP RX
                       |
                     [18k]
                       |
                      GND
```
Notes:

- Never connect Gate TX directly to ESP RX
- ESP TX → Gate RX is typically safe directly
- Keep wires short and clean to avoid noise

---

## Custom PCB (Fusion 360)

A ready-to-use PCB design is included in the repository:

    docs/cb19-gate-espboard.fbrd

This design already includes:

- correct resistor divider
- proper routing
- ESP integration

Important:

- R1 must be 10kΩ
- R2 must be 18kΩ

---

## ESPHome Configuration

A fully working example configuration is available:

    examples/cb19_example.yaml

To use it:

1. Copy the file
2. Add your WiFi credentials
3. Check and modify RX and TX pins for your board
4. Upload to ESP

No additional changes are required.

---

## How It Works

The ESP communicates with the gate controller using ASCII commands over UART.

Commands are sent in the following format:

    COMMAND;src=P0031DA2\r\n

Examples:

- FULL OPEN → opens the gate
- FULL CLOSE → closes the gate
- STOP → stops movement
- PED OPEN → pedestrian mode
- RS → request status

The controller responds with:

- ACK → command accepted
- NAK → command rejected
- ACK RS → status frame

---

## Position Handling

The controller does not provide normalized position values.

Observed behavior:

- Opening starts around ~56%
- Closing starts around ~44%

To correct this, the component provides two calibration values:

- Opening Start Percent
- Closing Start Percent

These can be adjusted directly from Home Assistant.

This allows:

- correct 0–100% representation
- accurate cover behavior
- no need for reflashing

---

## Polling Strategy

To keep communication safe and stable:

- During movement → polling every 200 ms
- After movement → polling every 500 ms for 10 seconds
- Idle → polling every 60 seconds

Additionally:

- polling pauses briefly after sending a command

This ensures:

- responsive updates
- no overload of the controller

---

## Project Structure

- components/ → ESPHome custom component
- examples/ → working YAML
- docs/ → protocol documentation + PCB design file

---

## Documentation

Detailed protocol description is available here:

    docs/protocol.md

---

## Current Status

- Communication stable
- Commands reliable
- Position tracking usable
- Fully integrated with Home Assistant

---

## Future Plans

- Read and modify full configuration (F-codes)
- Remote management (pairing remotes)
- Full replacement of official mobile app
- Advanced diagnostics

---

## License

MIT License
