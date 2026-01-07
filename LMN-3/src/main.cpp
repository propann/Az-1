#include "config_fixed.h"
#include <Wire.h>
#include <U8g2lib.h>

// Minimal, dependency-light firmware:
// - scans a 5x14 matrix
// - sends MIDI NoteOn/NoteOff via Serial1 (UART) only
// - optional: reads 4 encoders (quadrature) -> sends CC
// - optional: reads joystick/pot -> sends Pitch Bend
//
// If you want to re-integrate Control_Surface later, do it AFTER V1 is stable.


// ---------- OLED (2x SSD1306 over I2C) ----------
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C oledA(U8G2_R0, U8X8_PIN_NONE);
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C oledB(U8G2_R0, U8X8_PIN_NONE);

static void oledInit() {
  // Configure I2C pins (Teensy supports setSDA/setSCL)
  Wire.setSDA(I2C_SDA_PIN);
  Wire.setSCL(I2C_SCL_PIN);
  Wire.begin();

  oledA.setI2CAddress(OLED_ADDR_A << 1);
  oledB.setI2CAddress(OLED_ADDR_B << 1);
  oledA.begin();
  oledB.begin();

  oledA.clearBuffer();
  oledA.setFont(u8g2_font_6x10_tf);
  oledA.drawStr(0, 12, "SLAVE UI A");
  oledA.drawStr(0, 28, "I2C OK");
  oledA.sendBuffer();

  oledB.clearBuffer();
  oledB.setFont(u8g2_font_6x10_tf);
  oledB.drawStr(0, 12, "SLAVE UI B");
  oledB.drawStr(0, 28, "I2C OK");
  oledB.sendBuffer();
}

static uint8_t keyState[5][14] = {};      // 0=up, 1=down
static uint32_t keyLastChange[5][14] = {}; // debounce timer

constexpr uint16_t DEBOUNCE_MS = 12;

// ---------- MIDI helpers (Serial1 raw MIDI) ----------
static inline void midiWrite(uint8_t b) {
  Serial1.write(b);
}

static inline void midiNoteOn(uint8_t ch1_16, uint8_t note, uint8_t vel) {
  midiWrite(0x90 | ((ch1_16 - 1) & 0x0F));
  midiWrite(note & 0x7F);
  midiWrite(vel & 0x7F);
}

static inline void midiNoteOff(uint8_t ch1_16, uint8_t note, uint8_t vel) {
  midiWrite(0x80 | ((ch1_16 - 1) & 0x0F));
  midiWrite(note & 0x7F);
  midiWrite(vel & 0x7F);
}

static inline void midiCC(uint8_t ch1_16, uint8_t cc, uint8_t val) {
  midiWrite(0xB0 | ((ch1_16 - 1) & 0x0F));
  midiWrite(cc & 0x7F);
  midiWrite(val & 0x7F);
}

static inline void midiPitchBend14(uint8_t ch1_16, uint16_t bend14) {
  // 14-bit: 0..16383, center=8192
  bend14 = (bend14 > 16383) ? 16383 : bend14;
  uint8_t lsb = bend14 & 0x7F;
  uint8_t msb = (bend14 >> 7) & 0x7F;
  midiWrite(0xE0 | ((ch1_16 - 1) & 0x0F));
  midiWrite(lsb);
  midiWrite(msb);
}

// ---------- Matrix scanning ----------
static inline uint8_t noteFor(uint8_t r, uint8_t c) {
  // Row-major mapping, 70 notes: 36..105
  return BASE_NOTE + (r * 14) + c;
}

static void matrixInit() {
  for (uint8_t r = 0; r < 5; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], HIGH); // idle high (no active row)
  }
  for (uint8_t c = 0; c < 14; c++) {
    pinMode(COL_PINS[c], INPUT_PULLUP); // columns read with pullups
  }
}

static void scanMatrix() {
  const uint32_t now = millis();

  for (uint8_t r = 0; r < 5; r++) {
    // Activate this row: drive LOW
    digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(5);

    for (uint8_t c = 0; c < 14; c++) {
      const bool pressed = (digitalRead(COL_PINS[c]) == LOW); // active low
      const uint8_t newState = pressed ? 1 : 0;
      const uint8_t oldState = keyState[r][c];

      if (newState != oldState) {
        // debounce
        if (now - keyLastChange[r][c] >= DEBOUNCE_MS) {
          keyState[r][c] = newState;
          keyLastChange[r][c] = now;

          const uint8_t note = noteFor(r, c);
          if (newState) {
            midiNoteOn(MIDI_CH, note, 0x7F);
          } else {
            midiNoteOff(MIDI_CH, note, 0x00);
          }
        }
      }
    }

    // Deactivate row
    digitalWrite(ROW_PINS[r], HIGH);
  }
}

// ---------- Optional encoders (simple polling) ----------
struct Enc {
  uint8_t a, b;
  int8_t  last;
  int32_t value; // unbounded
};

static Enc enc[4];

static inline int8_t readAB(uint8_t a, uint8_t b) {
  return (digitalRead(a) ? 2 : 0) | (digitalRead(b) ? 1 : 0);
}

static void encInit() {
  for (int i = 0; i < 4; i++) {
    enc[i].a = ENC_PINS_A[i];
    enc[i].b = ENC_PINS_B[i];
    pinMode(enc[i].a, INPUT_PULLUP);
    pinMode(enc[i].b, INPUT_PULLUP);
    enc[i].last = readAB(enc[i].a, enc[i].b);
    enc[i].value = 64; // start mid
    midiCC(MIDI_CH, ENC_CC[i], (uint8_t)enc[i].value);
  }
}

// Gray-code transition table for quadrature
static const int8_t QDEC[16] = {
  0, -1, +1, 0,
  +1, 0, 0, -1,
  -1, 0, 0, +1,
  0, +1, -1, 0
};

static void encUpdate() {
  for (int i = 0; i < 4; i++) {
    const int8_t cur = readAB(enc[i].a, enc[i].b);
    const int8_t idx = ((enc[i].last & 3) << 2) | (cur & 3);
    const int8_t step = QDEC[idx & 0x0F];

    if (step != 0) {
      enc[i].last = cur;
      int32_t v = enc[i].value + step;
      if (v < 0) v = 0;
      if (v > 127) v = 127;
      if (v != enc[i].value) {
        enc[i].value = v;
        midiCC(MIDI_CH, ENC_CC[i], (uint8_t)v);
      }
    } else {
      enc[i].last = cur;
    }
  }
}

// ---------- Optional pitch bend (A15) ----------
static uint32_t lastPB = 0;
static uint16_t lastPBVal = 8192;

static void pbUpdate() {
  // limit rate
  if (millis() - lastPB < 10) return;
  lastPB = millis();

  int raw = analogRead(HORIZONTAL_PB_PIN); // Teensy 12-bit default 0..4095
  // invert so right increases (like your earlier note)
  int remapped = map(raw, 0, 4095, 4095, 0);

  // scale 0..4095 -> 0..16383
  uint16_t bend = (uint16_t)((remapped * 16383L) / 4095L);
  if (bend != lastPBVal) {
    lastPBVal = bend;
    midiPitchBend14(MIDI_CH, bend);
  }
}

void setup() {
  // --- MIDI UART (Serial only) ---
  Serial1.setRX(MIDI_RX_PIN);
  Serial1.setTX(MIDI_TX_PIN);
  Serial1.begin(MIDI_BAUD);

  // --- OLEDs ---
  oledInit();

  matrixInit();
  encInit();

  // Stable ADC
  analogReadResolution(12);
}

void loop() {
  scanMatrix();
  encUpdate();
  pbUpdate();
}
