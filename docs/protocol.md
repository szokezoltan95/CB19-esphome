# CB19 UART Protocol

## Commands

FULL OPEN  
FULL CLOSE  
STOP  
PED OPEN  

RS  
RP,1  
WP,1  

RESTORE  
AUTO LEARN  
READ LEARN STATUS  
REMOTE LEARN  
CLEAR REMOTE LEARN  

## F-code Table

| Slot | Code | Name | Values |
|------|------|------|--------|
| 1 | F1 | Motor Type | 0–1 |
| 2 | F2 | Opening Force | 0–3 |
| 3 | F3 | Closing Force | 0–3 |
| 4 | F4 | Opening Speed | 0–3 |
| 5 | F5 | Closing Speed | 0–3 |
| 6 | F6 | Slow Speed | 0–3 |
| 7 | F7 | Slowdown | 0–4 |
| 8 | F8 | Open Delay | 0–9 |
| 9 | F9 | Close Delay | 0–9 |
| 10 | FA | Auto Close | 0–8 |
| 11 | FB | Safety Mode | 0–6 |
| 12 | FC | Pedestrian Mode | 0–1 |
| 13 | FD | Flash Lamp | 0–1 |
| 14 | FE | Photocell 1 | 0–1 |
| 15 | FF | Photocell 2 | 0–1 |
| 16 | FG | Buzzer | 0–1 |
| 17 | FH | Electric Lock | 0–1 |
| 18 | FI | LED Direction | 0–1 |
| 19 | FJ | Gate Type | 0–1 |
| 20 | FK | Back Release | 0–6 |
