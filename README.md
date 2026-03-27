# CB19 ESPHome Gate Controller

Custom ESPHome component for CB19 gate controllers over UART.

This project replaces the original WiFi module with a fully local ESPHome integration and provides full control, diagnostics, and configuration of the gate directly from Home Assistant.



## Experimental branch

This is the experimental branch for a major refactor of the code with the following:
- Removal of the cover entity and switch to custom control
- Corrected gate state handling with all special edge cases
- All gate states in one state machine
- Development of custom HACS card integration



---

## Features

- Open, close, stop, pedestrian open
- Real-time position tracking from RS frames
- Motion detection from RS + $V1PKF events
- Photocell and obstruction detection
- Motor diagnostics:
  - raw position
  - speed
  - load
- Full parameter read/write (RP,1 / WP,1)
- Pending vs current parameter handling
- Remote management:
  - add remote
  - remove remote
- Auto learn with automatic polling
- Factory reset (hidden entity for safety)
- Full Home Assistant integration

---

## Hardware

Required:

- ESP32
- UART connection to CB19 controller
- Level shifting (Gate TX → ESP RX)

Example divider:
```
Gate TX ---[10k]---+--- ESP RX
                  |
                [18k]
                  |
                 GND
```
Notes:

- Do NOT connect Gate TX directly to ESP RX
- ESP TX → Gate RX usually works directly
- Keep wiring short and clean

---

## Installation

### Add external component
```
external_components:
  - source: github://szokezoltan95/CB19-esphome@main
    components: [cb19_gate]
```
### UART setup
```
uart:
  id: gate_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600
```
### Basic config
```
cb19_gate:
  id: gate_controller
  uart_id: gate_uart

  cover:
    name: Gate

  pedestrian_button:
    name: Pedestrian Open
```
Full example:
`examples/cb19_example.yaml`

---

## Home Assistant Entity Model

### Main Control
- Gate (cover)
- Pedestrian open
- Moving / open / closed / obstruction / photocell

### Diagnostics
- Last state
- Last ACK
- Last RS
- Learn status
- Current parameter block
- Pending parameter block
- Motor raw / speed / load

### Configuration

1. Read Parameters
2. Write Parameters
3. Revert Pending Parameters
4. Add Remote
5. Remove Remote
6. Auto Learn
7. Factory Reset

All F-code parameters are exposed as select entities.

---

## Parameter Workflow

1. Change parameters → stored as pending
2. Press Write Parameters
3. WP,1 is sent
4. RP,1 readback
5. System syncs

---

## Learn Workflow

- Auto Learn starts process
- Component polls status ~1s
- Stops on success/fail

---

## Cover Calibration

Controller is not 0–100 normalized.

Observed:
- Opening ~56%
- Closing ~44%

Adjust in HA:
- Opening Start Percent
- Closing Start Percent

---

## Polling

- Fast while moving
- Slower after stop
- Very slow idle
- Periodic RP sync

---

## Protocol

See `docs/protocol.md`
