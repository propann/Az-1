# LMN-3 Serial Link

- Port: Hardware UART @ 31_250 baud (MIDI DIN rate).
- Pins: TX=1, RX=0 (Serial1 on Teensy 4.1), plus common GND. Pins 17/18 are reserved for the display and must stay unused here.
- USB MIDI is disabled; all events go out on this link as standard 3-byte MIDI messages (NoteOn/NoteOff/CC, channel 1).

## Frame format

Each event is a standard 3-byte MIDI message on channel 1:

| Byte | Meaning | Notes |
| --- | --- | --- |
| 0 | Status | `0x90` note on, `0x80` note off, `0xB0` CC |
| 1 | `d1` | Note or CC number |
| 2 | `d2` | Velocity or CC value (note off uses `0x00`) |

- Buttons/encoders send CC 127 on press/increment and 0 on release/decrement.
- Pitch-bend stick is encoded as CC `HORIZONTAL_PB_CC` (default 120), 0–127 range.

## Edge detection

- Matrices send events only on transitions (press → NOTE_ON/CC 127, release → NOTE_OFF/CC 0).
- Plus/Minus buttons adjust transposition when the CONTROL button (shift) is held; otherwise they emit their own CC.
