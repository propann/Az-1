#pragma once
#include <Arduino.h>

// ====== PROJECT: LMN3 Front Panel (SLAVE) -> MicroDexed-touch (MASTER) ======
// V1 RULES (GRAVED):
// - MIDI transport: UART Serial (remapped) @ 31250, pins RX=19 / TX=20 (Teensy)
// - NO USB MIDI
// - NO AUDIO on the SLAVE
// - Pin 15 untouched (used by original firmware if needed)
// - A0 legacy (pin 14) is DERIVED to pin 33 in this wiring

// ====== MIDI ======

// MIDI UART pins (wired)
constexpr uint8_t MIDI_RX_PIN = 19;
constexpr uint8_t MIDI_TX_PIN = 20;
constexpr uint32_t MIDI_BAUD = 31250;

// OLED (I2C)
constexpr uint8_t I2C_SDA_PIN = 18;
constexpr uint8_t I2C_SCL_PIN = 19;
constexpr uint8_t OLED_ADDR_A = 0x3C;
constexpr uint8_t OLED_ADDR_B = 0x3D;
// NOTE: You reported MIDI RX/TX are wired to pins 19/20 (remapped).
// WARNING: Pin 19 is also I2C SCL on Teensy. If your OLEDs are on SCL=19, you cannot use UART RX on 19 at the same time.
// If you see I2C or MIDI issues, move either MIDI RX or I2C SCL to a different pin/serial port.
constexpr uint32_t MIDI_BAUD = 31250;
constexpr uint8_t  MIDI_CH  = 1; // Channel 1 (1..16)

// ====== MATRIX (5x14) ======
// Rows from LMN3/Totem wiring (verify with your harness):
constexpr uint8_t ROW_PINS[5] = {24, 23, 34, 35, 28};

// Cols with your constraints:
// - DO NOT use pins 0/1 for matrix (reserved for Serial1 MIDI)
// - legacy COL_6 and COL_7 moved to 20/21
// - legacy "14" derived to 33
constexpr uint8_t COL_PINS[14] = {
  9,  8,  7,  4,  3,  2,
  20, 21,
  25, 33, 13, 41, 40, 36
};

// Base note mapping: 70 keys -> notes 36..105 (inclusive)
constexpr uint8_t BASE_NOTE = 36;

// ====== ENCODERS (optional, if wired) ======
// Quadrature pins (A,B) for 4 encoders (from earlier LMN3 notes):
constexpr uint8_t ENC_PINS_A[4] = {5, 26, 29, 31};
constexpr uint8_t ENC_PINS_B[4] = {6, 27, 30, 32};

// CC numbers (from your config snippet)
constexpr uint8_t ENC_CC[4] = {3, 9, 14, 15};

// ====== JOYSTICK / ANALOG (optional) ======
constexpr uint8_t HORIZONTAL_PB_PIN = A15; // keep as-is (original)
