# CB19 Communication Documentation

## Protocol Overview

ASCII-based UART protocol.

---

## Commands

- FULL OPEN
- FULL CLOSE
- STOP
- PED OPEN
- RS

Format:

COMMAND;src=P0031DA2

---

## Responses

- ACK → accepted
- NAK → rejected
- ACK RS → status frame

---

## RS Frame Example

ACK RS:60,64,CC,0D,3E,07,01,40,00

---

## Byte Mapping

| Byte | Meaning |
|------|--------|
| 0    | Photocell |
| 2    | Motion state |
| 3    | Motor 1 position |
| 6    | Motor 2 position |

---

## Motion States

| Value | Meaning |
|------|--------|
| CC   | Moving |
| AA   | Idle |
| EE   | Obstruction |

---

## Notes

- Polling is adaptive
- RS provides position feedback
