#pragma once

#include <Arduino.h>

void link_begin();
void link_send_note_on(uint8_t note, uint8_t vel);
void link_send_note_off(uint8_t note);
void link_send_cc(uint8_t cc, uint8_t val);

static constexpr uint8_t MIDI_CH = 0; // canal 1 = 0

inline void midi_write3(HardwareSerial &s, uint8_t a, uint8_t b, uint8_t c) {
    s.write(a);
    s.write(b);
    s.write(c);
}

inline void midi_note_on(HardwareSerial &s, uint8_t note, uint8_t vel) {
    midi_write3(s, static_cast<uint8_t>(0x90 | MIDI_CH), note & 0x7F, vel & 0x7F);
}

inline void midi_note_off(HardwareSerial &s, uint8_t note) {
    // vrai NoteOff
    midi_write3(s, static_cast<uint8_t>(0x80 | MIDI_CH), note & 0x7F, 0);
    // alternative: NoteOn vel=0
    // midi_write3(s, static_cast<uint8_t>(0x90 | MIDI_CH), note & 0x7F, 0);
}

inline void midi_cc(HardwareSerial &s, uint8_t cc, uint8_t val) {
    midi_write3(s, static_cast<uint8_t>(0xB0 | MIDI_CH), cc & 0x7F, val & 0x7F);
}

inline void midi_pitchbend14(HardwareSerial &s, uint16_t v14) {
    v14 = (v14 > 16383) ? 16383 : v14;
    const uint8_t lsb = v14 & 0x7F;
    const uint8_t msb = (v14 >> 7) & 0x7F;
    midi_write3(s, static_cast<uint8_t>(0xE0 | MIDI_CH), lsb, msb);
}
