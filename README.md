# CB19 ESPHome Gate Controller

Custom ESPHome component for CB19 gate controllers over UART.

## Features

- Open / Close / Stop / Pedestrian
- Real-time position tracking
- Photocell & obstruction detection
- Motor diagnostics (position, speed, load)
- Full parameter read/write (RP,1 / WP,1)
- Pending vs current parameter handling
- Remote management (add/remove)
- Auto learn with polling
- Factory reset
- Full Home Assistant integration

## Installation

```yaml
external_components:
  - source: github://szokezoltan95/CB19-esphome@main
    components: [cb19_gate]
```

## Notes

- Uses raw parameter values (0-based)
- Manual numbering may differ
