# CB19 UART Protocol Documentation

This document describes the communication protocol used by the CB19 gate controller.

The protocol was reverse-engineered from real device communication.

---

## General Description

- Communication is ASCII-based
- Uses UART (typically 9600 baud)
- Messages are line-based
- Each command ends with CRLF (\r\n)

---

## Command Format

All commands follow the same structure:

    COMMAND;src=DEVICE_ID

Example:

    FULL OPEN;src=P0031DA2

### Notes

- `src` identifies the sender
- The original WiFi module uses `P0031DA2`
- Using a different ID may result in NAK responses

---

## Supported Commands

- FULL OPEN
- FULL CLOSE
- STOP
- PED OPEN
- RS (status request)

---

## Responses

### ACK

Command accepted:

    ACK FULL OPEN

### NAK

Command rejected:

    NAK FULL OPEN

Common reasons:

- invalid format
- missing src
- invalid device ID

---

## RS (Status) Frame

The most important part of the protocol.

Example:

    ACK RS:60,64,CC,0D,3E,07,01,40,00

This is a comma-separated byte stream in hexadecimal format.

---

## Frame Structure

| Byte | Meaning |
|------|--------|
| 0 | Photocell status |
| 1 | Unknown |
| 2 | Motion state |
| 3 | Motor 1 position |
| 4 | Unknown |
| 5 | Unknown |
| 6 | Motor 2 position |
| 7 | Flags |
| 8 | Unknown |

---

## Motion States

| Value | Meaning |
|------|--------|
| CC | Moving |
| AA | Idle |
| EE | Obstruction |
| DB/FB | Pedestrian mode |

---

## Position Interpretation

- Motor values are raw positions (0–255 range)
- Not normalized
- Not perfectly linear

Typical behavior:

- Opening does not start from 0
- Closing does not start from 100

This is why calibration is required.

---

## Photocell

- Byte 0 indicates photocell state
- Value changes when beam is interrupted

---

## Obstruction Detection

- Motion state EE indicates obstruction
- Gate typically stops immediately

---

## Polling Behavior

The controller supports status polling via:

    RS;src=P0031DA2

Recommended strategy:

- Fast polling during movement
- Slow polling when idle

Avoid excessive polling to prevent instability.

---

## Important Observations

- Commands without `src` are rejected
- Timing matters (too frequent commands → NAK)
- RS frames are the only reliable source of position data

---

## Conclusion

The CB19 protocol is simple but requires:

- correct formatting
- proper timing
- calibration for accurate position

Once implemented correctly, it provides reliable full control over the gate.
