#include "LinkUart.h"

namespace {
HardwareSerial &midiSerial = Serial1;
} // namespace

void link_begin() {
    midiSerial.begin(31250);
}

void link_send_note_on(uint8_t note, uint8_t vel) {
    midi_note_on(midiSerial, note, vel);
}

void link_send_note_off(uint8_t note) {
    midi_note_off(midiSerial, note);
}

void link_send_cc(uint8_t cc, uint8_t val) {
    midi_cc(midiSerial, cc, val);
}
