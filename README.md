# CB19 ESPHome Gate Controller

Custom ESPHome component for CB19 gate controllers over UART.

This project replaces the original WiFi module with a fully local ESPHome integration. In the current state it provides gate control, state feedback, protocol-level diagnostics, parameter read/write support, remote-management commands, learn-status polling, and Home Assistant friendly configuration entities.

## Current capabilities

- Open, close, stop, and pedestrian open
- Real-time position tracking from `ACK RS` frames
- Motion state detection from both `RS` frames and `$V1PKF...` event lines
- Photocell and obstruction indication
- Motor raw position, speed, and load diagnostics for both motors
- Parameter read/write using `RP,1` and `WP,1`
- Pending vs current parameter tracking
- Manual actions for:
  - read parameters
  - write parameters
  - revert pending parameters
  - add remote
  - remove remote
  - auto learn
  - factory reset
- Learn-status polling via `READ LEARN STATUS`
- Home Assistant configuration entities for all F-code parameters
- Local calibration for cover percentage handling

## Hardware

Required:

- ESP32
- UART connection to the CB19 controller
- level shifting on Gate TX -> ESP RX

Typical tested divider:

```text
Gate TX ---[10k]---+--- ESP RX
                   |
                 [18k]
                   |
                  GND
```

Important:

- Do not connect Gate TX directly to ESP RX
- ESP TX -> Gate RX is typically usable directly
- Keep UART wiring short and clean

## ESPHome setup

The component is loaded through `external_components`.

Example:

```yaml
external_components:
  - source: github://szokezoltan95/CB19-esphome@full-feature
    components: [cb19_gate]
```

A complete example configuration is available in `examples/cb19_example.yaml`.

## Home Assistant entity model

The integration exposes three practical groups of entities.

### Main control

- gate cover
- pedestrian open button
- motion/open/closed/photocell/obstruction states

### Diagnostics

- last state
- last ACK
- last RS
- learn status
- config warning
- current parameter block
- pending parameter block
- motor raw values
- motor speed/load values

### Configuration

Buttons are intentionally placed in the configuration category and named in a fixed order:

1. Read Parameters
2. Write Parameters
3. Revert Pending Parameters
4. Add Remote
5. Remove Remote
6. Auto Learn
7. Factory Reset

All F-code parameters are exposed as `select` entities.

## Parameter workflow

The intended workflow is:

1. Change one or more F-code select entities in Home Assistant
2. The new values are stored as pending values only
3. Press **Write Parameters**
4. The component sends a full `WP,1:` block
5. It then performs a readback with `RP,1`
6. Current and pending values are resynchronized

This prevents accidental writes for each individual change and mirrors the safer workflow we wanted for Home Assistant.

## Learn workflow

`READ LEARN STATUS` is not exposed as a manual button. Instead:

- **Auto Learn** starts the learn procedure
- the component polls learn status automatically about once per second
- polling continues until success, failure, or timeout

This mirrors the behavior of the original WiFi module more closely.

## Cover calibration

The controller does not report a directly normalized 0 to 100% position.

Observed behavior from testing:

- opening begins around ~56%
- closing begins around ~44%

The component therefore keeps two Home Assistant-adjustable calibration values:

- Opening Start Percent
- Closing Start Percent

These values correct the reported cover percentage without reflashing firmware.

## Polling

Current strategy:

- fast polling while moving
- slower polling after motion stops
- very slow polling when idle
- periodic parameter resync with `RP,1`

Polling is briefly suppressed after sending commands to avoid stepping on protocol traffic.

## Supported protocol features

Documented in more detail in `docs/protocol.md`:

- `FULL OPEN`
- `FULL CLOSE`
- `STOP`
- `PED OPEN`
- `RS`
- `RP,1`
- `WP,1`
- `RESTORE`
- `AUTO LEARN`
- `READ LEARN STATUS`
- `REMOTE LEARN`
- `CLEAR REMOTE LEARN`

## Notes on F1

For Home Assistant exposure, the Hall sensor mode is intentionally not presented as a selectable option. This keeps the configuration model simpler and avoids dynamic remapping of F2 and F3 option labels.

## Repository structure

- `components/` custom ESPHome component
- `examples/` example ESPHome YAML
- `docs/` documentation and protocol notes

## Documentation

- `README.md` project overview
- `docs/protocol.md` protocol details, command list, status handling, and full F-code table
