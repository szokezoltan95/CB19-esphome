# CB19 UART Protocol Documentation

This document summarizes the reverse-engineered UART protocol used by the CB19 gate controller
and the behavior implemented in the ESPHome integration.

---

## Transport basics

- ASCII protocol over UART
- line-based communication
- commands end with `\r\n`
- requires `src=` sender identifier

Typical tested sender string:

```
P0031DA2
```

General command format:

```
COMMAND;src=P0031DA2
```

Without a valid `src`, the controller may reply with `NAK`.

---

## Main commands

### Gate movement

```
FULL OPEN;src=P0031DA2
FULL CLOSE;src=P0031DA2
STOP;src=P0031DA2
PED OPEN;src=P0031DA2
```

### Status polling

```
RS;src=P0031DA2
```

Response example:

```
ACK RS:60,64,CC,0D,3E,07,01,40,00
```

---

### Parameter read/write

```
RP,1;src=P0031DA2
WP,1:<20 comma-separated values>;src=P0031DA2
```

`RP,1` returns the full 20-value configuration block:

```
ACK RP,1:0,1,1,2,2,1,3,1,1,0,0,1,0,0,0,0,0,1,0,1
```

`WP,1` writes the full block, not a single parameter.

Recommended handling:

1. Edit pending values locally  
2. Send one `WP,1`  
3. Immediately read back using `RP,1`  

---

### Service commands

```
RESTORE;src=P0031DA2
AUTO LEARN;src=P0031DA2
READ LEARN STATUS;src=P0031DA2
REMOTE LEARN;src=P0031DA2
CLEAR REMOTE LEARN;src=P0031DA2
```

Observed command flow:

- `RESTORE` → `ACK RESTORE`, then `Restored`
- `AUTO LEARN` → `ACK AUTO LEARN`, then learn-status polling
- `READ LEARN STATUS` → `ACK LEARN STATUS:<payload>`
- `REMOTE LEARN` → `ACK REMOTE LEARN`, then `LearnStart` / `RemoteAdd` / `LearnComplete`
- `CLEAR REMOTE LEARN` → `ACK CLEAR REMOTE LEARN`, then `ClearComplete`

---

## RS frame structure

Current RS frame parsing uses 9 bytes:

| Byte | Meaning |
|------|--------|
| 0 | Photocell status |
| 1 | Unknown |
| 2 | Motion/event state |
| 3 | Motor 1 raw position |
| 4 | Motor 1 speed |
| 5 | Motor 1 load |
| 6 | Motor 2 raw position |
| 7 | Motor 2 speed |
| 8 | Motor 2 load |

### Motion state byte values

| Value | Meaning |
|------|--------|
| CC | Moving |
| AA | Idle |
| EE | Obstruction / stop |
| DB / FB | Pedestrian-related |

---

### Additional event lines

The controller also emits `$V1PKF...` messages:

- Opening
- Opened
- Closing
- Closed
- Stopped
- PedOpening
- PedOpened
- AutoClosing
- Restored
- AutoLearn
- LearnStart
- RemoteAdd
- LearnComplete
- ClearComplete

---

## Position handling

The controller does not report normalized 0–100%.

Observed:

- opening starts ~56%
- closing starts ~44%

ESPHome exposes:

- Opening Start Percent
- Closing Start Percent

---

## Learn status handling

`READ LEARN STATUS` responses:

- SYSTEM LEARNING
- SYSTEM LEARN COMPLETE
- SYSTEM LEARN FAIL

Polled approximately once per second during learning.

---

## Full F-Code Table (Complete)

| Slot | F | Name | Options (HA) | Raw | Default |
|------|---|------|--------------|-----|--------|
|1|F1|Motor Type|F1-0 Normal<br>F1-1 Limit switch|0/1|0|
|2|F2|Opening Force|F2-0 2A<br>F2-1 3A<br>F2-2 4A<br>F2-3 5A|0-3|1|
|3|F3|Closing Force|F3-0 2A<br>F3-1 3A<br>F3-2 4A<br>F3-3 5A|0-3|1|
|4|F4|Opening Speed|F4-0 40%<br>F4-1 50%<br>F4-2 75%<br>F4-3 100%|0-3|2|
|5|F5|Closing Speed|F5-0 40%<br>F5-1 50%<br>F5-2 75%<br>F5-3 100%|0-3|2|
|6|F6|Slow Speed|F6-0 40%<br>F6-1 50%<br>F6-2 60%<br>F6-3 70%|0-3|1|
|7|F7|Slowdown Start|F7-0 75%<br>F7-1 80%<br>F7-2 85%<br>F7-3 90%<br>F7-4 95%|0-4|3|
|8|F8|Open Delay|F8-0 0s<br>F8-1 2s<br>F8-2 5s<br>F8-3 10s<br>F8-4 15s<br>F8-5 20s<br>F8-6 25s<br>F8-7 35s<br>F8-8 45s<br>F8-9 55s|0-9|1|
|9|F9|Close Delay|F9-0 0s<br>F9-1 2s<br>F9-2 5s<br>F9-3 10s<br>F9-4 15s<br>F9-5 20s<br>F9-6 25s<br>F9-7 35s<br>F9-8 45s<br>F9-9 55s|0-9|1|
|10|FA|Auto Close|FA-0 Disabled<br>FA-1 3s<br>FA-2 10s<br>FA-3 20s<br>FA-4 40s<br>FA-5 60s<br>FA-6 120s<br>FA-7 180s<br>FA-8 300s|0-8|0|
|11|FB|Safety Mode|FB-0 Mode1<br>FB-1 Mode2<br>FB-2 Mode3<br>FB-3 Mode4<br>FB-4 Mode5<br>FB-5 Mode6<br>FB-6 Mode7|0-6|0|
|12|FC|Pedestrian Mode|FC-0 Disabled<br>FC-1 Enabled|0/1|1|
|13|FD|Flash Lamp|FD-0 Disabled<br>FD-1 Enabled|0/1|0|
|14|FE|Photocell 1|FE-0 Disabled<br>FE-1 Enabled|0/1|0|
|15|FF|Photocell 2|FF-0 Disabled<br>FF-1 Enabled|0/1|0|
|16|FG|Buzzer|FG-0 Disabled<br>FG-1 Enabled|0/1|0|
|17|FH|Electric Lock|FH-0 Normal<br>FH-1 Electric lock|0/1|0|
|18|FI|LED Direction|FI-0 Up<br>FI-1 Down|0/1|1|
|19|FJ|Gate Type|FJ-0 Single wing<br>FJ-1 Double wing|0/1|1|
|20|FK|Back Release|FK-0 Disabled<br>FK-1 0.1s<br>FK-2 0.2s<br>FK-3 0.3s<br>FK-4 0.4s<br>FK-5 0.5s<br>FK-6 0.6s|0-6|0|

---

## Notes

- Raw values are 0-based
- Manual numbering may differ
- Always verify behavior via `RP,1`
