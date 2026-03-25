# CB19 UART Protocol Documentation

This document summarizes the reverse-engineered UART protocol used by the CB19 gate controller and the current ESPHome integration behavior.

## Transport basics

- ASCII protocol over UART
- line based
- commands end with `\r\n`
- the controller expects a `src=` sender identifier

Typical tested sender string:

```text
P0031DA2
```

General command format:

```text
COMMAND;src=P0031DA2
```

Without a valid `src`, the controller may reply with `NAK`.

## Main commands

### Gate movement

```text
FULL OPEN;src=P0031DA2
FULL CLOSE;src=P0031DA2
STOP;src=P0031DA2
PED OPEN;src=P0031DA2
```

### Status polling

```text
RS;src=P0031DA2
```

Response example:

```text
ACK RS:60,64,CC,0D,3E,07,01,40,00
```

### Parameter read/write

```text
RP,1;src=P0031DA2
WP,1:<20 comma-separated values>;src=P0031DA2
```

`RP,1` returns the full 20-value configuration block:

```text
ACK RP,1:0,1,1,2,2,1,3,1,1,0,0,1,0,0,0,0,0,1,0,1
```

`WP,1` writes the full block, not a single parameter.

Recommended handling:

1. edit pending values locally
2. send one `WP,1`
3. immediately read back using `RP,1`

### Service commands

```text
RESTORE;src=P0031DA2
AUTO LEARN;src=P0031DA2
READ LEARN STATUS;src=P0031DA2
REMOTE LEARN;src=P0031DA2
CLEAR REMOTE LEARN;src=P0031DA2
```

Observed command flow:

- `RESTORE` -> `ACK RESTORE`, then `Restored`
- `AUTO LEARN` -> `ACK AUTO LEARN`, then learn-status polling
- `READ LEARN STATUS` -> `ACK LEARN STATUS:<payload>`
- `REMOTE LEARN` -> `ACK REMOTE LEARN`, then `LearnStart` / `RemoteAdd` / `LearnComplete`
- `CLEAR REMOTE LEARN` -> `ACK CLEAR REMOTE LEARN`, then `ClearComplete`

## RS frame structure

Current RS frame parsing in the project uses 9 bytes:

| Byte | Meaning |
|---|---|
| 0 | Photocell status |
| 1 | Unknown / not yet used |
| 2 | Motion/event state |
| 3 | Motor 1 raw position |
| 4 | Motor 1 speed |
| 5 | Motor 1 load |
| 6 | Motor 2 raw position |
| 7 | Motor 2 speed |
| 8 | Motor 2 load |

### Motion state byte values

| Value | Meaning |
|---|---|
| `CC` | Moving |
| `AA` | Idle |
| `EE` | Obstruction / stop |
| `DB` / `FB` | Pedestrian mode related |

### Additional event lines

The controller also emits `$V1PKF...` textual state/event messages. These are important because they add context to the RS frames.

Examples:

- `Opening`
- `Opened`
- `Closing`
- `Closed`
- `Stopped`
- `PedOpening`
- `PedOpened`
- `AutoClosing`
- `Restored`
- `AutoLearn`
- `LearnStart`
- `RemoteAdd`
- `LearnComplete`
- `ClearComplete`

## Position handling

The controller does not report normalized 0 to 100% cover position directly.

Observed behavior from testing:

- opening begins around ~56%
- closing begins around ~44%

Because of that, the ESPHome component exposes two calibration numbers:

- Opening Start Percent
- Closing Start Percent

## Learn status handling

`READ LEARN STATUS` responses observed during testing:

- `SYSTEM LEARNING`
- `SYSTEM LEARN COMPLETE`
- `SYSTEM LEARN FAIL`

The integration polls this automatically roughly once per second after `AUTO LEARN` starts.

## Full F-code table

The 20-value `WP,1` / `RP,1` block maps to F-codes in this order:

| Slot in RP/WP block | F-code |
|---|---|
| 1 | F1 |
| 2 | F2 |
| 3 | F3 |
| 4 | F4 |
| 5 | F5 |
| 6 | F6 |
| 7 | F7 |
| 8 | F8 |
| 9 | F9 |
| 10 | FA |
| 11 | FB |
| 12 | FC |
| 13 | FD |
| 14 | FE |
| 15 | FF |
| 16 | FG |
| 17 | FH |
| 18 | FI |
| 19 | FJ |
| 20 | FK |

## F-code values currently exposed in Home Assistant

Note: Some of the parameter value mappings differ from the user manual. The mappings below are based on the actual logic of the gate controller.

### F1
- `F1-0 | Normal (default)`
- `F1-1 | Limit switch`

### F2
- `F2-0 | 2A`
- `F2-1 | 3A (default)`
- `F2-2 | 4A`
- `F2-3 | 5A`

### F3
- `F3-0 | 2A`
- `F3-1 | 3A (default)`
- `F3-2 | 4A`
- `F3-3 | 5A`

### F4
- `F4-0 | 40%`
- `F4-1 | 50%`
- `F4-2 | 75% (default)`
- `F4-3 | 100%`

### F5
- `F5-0 | 40%`
- `F5-1 | 50%`
- `F5-2 | 75% (default)`
- `F5-3 | 100%`

### F6
- `F6-0 | 40%`
- `F6-1 | 50% (default)`
- `F6-2 | 60%`
- `F6-3 | 70%`

### F7
- `F7-0 | 75%`
- `F7-1 | 80%`
- `F7-2 | 85%`
- `F7-3 | 90% (default)`
- `F7-4 | 95%`

### F8
- `F8-0 | 0 s`
- `F8-1 | 2 s (default)`
- `F8-2 | 5 s`
- `F8-3 | 10 s`
- `F8-4 | 15 s`
- `F8-5 | 20 s`
- `F8-6 | 25 s`
- `F8-7 | 35 s`
- `F8-8 | 45 s`
- `F8-9 | 55 s`

### F9
- `F9-0 | 0 s`
- `F9-1 | 2 s (default)`
- `F9-2 | 5 s`
- `F9-3 | 10 s`
- `F9-4 | 15 s`
- `F9-5 | 20 s`
- `F9-6 | 25 s`
- `F9-7 | 35 s`
- `F9-8 | 45 s`
- `F9-9 | 55 s`

### FA
- `FA-0 | Disabled (default)`
- `FA-1 | 3 s`
- `FA-2 | 10 s`
- `FA-3 | 20 s`
- `FA-4 | 40 s`
- `FA-5 | 60 s`
- `FA-6 | 120 s`
- `FA-7 | 180 s`
- `FA-8 | 300 s`

### FB
- `FB-0 | Mode 1 (default)`
- `FB-1 | Mode 2`
- `FB-2 | Mode 3`
- `FB-3 | Mode 4`
- `FB-4 | Mode 5`
- `FB-5 | Mode 6`
- `FB-6 | Mode 7`

### FC
- `FC-0 | Disabled`
- `FC-1 | Enabled (default)`

Entity name in Home Assistant should be presented as **Pedestrian Mode**.

### FD
- `FD-0 | Disabled (default)`
- `FD-1 | Enabled`

### FE
- `FE-0 | Disabled (default)`
- `FE-1 | Enabled`

### FF
- `FF-0 | Disabled (default)`
- `FF-1 | Enabled`

### FG
- `FG-0 | Disabled (default)`
- `FG-1 | Enabled`

### FH
- `FH-0 | Normal (default)`
- `FH-1 | Electric lock`

### FI
- `FI-0 | Up`
- `FI-1 | Down (default)`

### FJ
- `FJ-0 | Single wing`
- `FJ-1 | Double wing (default)`


### FK
- `FK-0 | Disabled (default)`
- `FK-1 | 0.1 s`
- `FK-2 | 0.2 s`
- `FK-3 | 0.3 s`
- `FK-4 | 0.4 s`
- `FK-5 | 0.5 s`
- `FK-6 | 0.6 s`

## Home Assistant configuration entity order

To keep the configuration page tidy, the integration uses numbered button names in this order:

1. Read Parameters
2. Write Parameters
3. Revert Pending Parameters
4. Add Remote
5. Remove Remote
6. Auto Learn
7. Factory Reset

These belong in the configuration category, not in the primary controls list.
