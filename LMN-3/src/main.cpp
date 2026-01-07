#include "LinkUart.h"
#include "UiRender.h"
#include "UiState.h"
#include "build_info.h"
#include "config_v2_serial.h"
#include <cstdio>
#include <Wire.h>
#include <U8g2lib.h>
#include <ResponsiveAnalogRead.h>

constexpr size_t NOTE_ROWS = 2;
constexpr size_t NOTE_COLS = 14;
constexpr size_t CC_ROWS = 3;
constexpr size_t CC_COLS = 11;
constexpr uint8_t NOTE_VELOCITY = 127;

constexpr int MAX_TRANSPOSITION = 4;
constexpr int MIN_TRANSPOSITION = -MAX_TRANSPOSITION;
constexpr int TRANSPOSE_SEMITONES = 12;

// Rows/columns for the two matrices
constexpr uint8_t ROW_PINS[] = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};
constexpr uint8_t NOTE_ROW_PINS[NOTE_ROWS] = {ROW_3, ROW_4};
constexpr uint8_t NOTE_COL_PINS[NOTE_COLS] = {COL_0, COL_1, COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8, COL_9, COL_10, COL_11, COL_12, COL_13};
constexpr uint8_t CC_ROW_PINS[CC_ROWS] = {ROW_0, ROW_1, ROW_2};
constexpr uint8_t CC_COL_PINS[CC_COLS] = {COL_3, COL_4, COL_5, COL_6, COL_7, COL_8, COL_9, COL_10, COL_11, COL_12, COL_13};
constexpr uint8_t ALL_COL_PINS[NOTE_COLS] = {COL_0, COL_1, COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8, COL_9, COL_10, COL_11, COL_12, COL_13};

static const uint8_t NOTE_ADDRESSES[NOTE_ROWS][NOTE_COLS] = {
    {1, 54, 56, 58, 1, 61, 63, 1, 66, 68, 70, 1, 73, 75},
    {53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76},
};

static const uint8_t CC_ADDRESSES[CC_ROWS][CC_COLS] = {
    {LOOP_BUTTON, LOOP_IN_BUTTON, LOOP_OUT_BUTTON, DUMMY, DUMMY, DUMMY, ENCODER_1_BUTTON, ENCODER_2_BUTTON, DUMMY, ENCODER_3_BUTTON, ENCODER_4_BUTTON},
    {CUT_BUTTON, PASTE_BUTTON, SLICE_BUTTON, SAVE_BUTTON, UNDO_BUTTON, DUMMY, DUMMY, DUMMY, DUMMY, DUMMY, DUMMY},
    {CONTROL_BUTTON, RECORD_BUTTON, PLAY_BUTTON, STOP_BUTTON, SETTINGS_BUTTON, TEMPO_BUTTON, MIXER_BUTTON, TRACKS_BUTTON, PLUGINS_BUTTON, MODIFIERS_BUTTON, SEQUENCERS_BUTTON},
};

bool notePrev[NOTE_ROWS][NOTE_COLS] = {};
uint8_t noteOnValue[NOTE_ROWS][NOTE_COLS] = {};
bool ccPrev[CC_ROWS][CC_COLS] = {};

bool plusHeld = false;
bool minusHeld = false;
int transposition = 0;

ResponsiveAnalogRead analog(HORIZONTAL_PB_PIN, true);
uint8_t lastPitchValue = 0;

// Displays conservés :
// - OLED VALUES (bus matériel Wire par défaut, adresse auto 0x3C/0x3D)
// - OLED CONTROL (I2C logiciel SDA=11, SCL=12, adresse 0x7A)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightDisplay(U8G2_R2, U8X8_PIN_NONE); // VALUES (Wire bus par défaut)
constexpr uint8_t AUX_SCL = 12;
constexpr uint8_t AUX_SDA = 11;
U8G2_SSD1306_128X64_NONAME_F_SW_I2C auxLeft2(U8G2_R2, AUX_SCL, AUX_SDA, U8X8_PIN_NONE); // CONTROL

enum ScreenRole {
    SCREEN_VALUES = 3,
    SCREEN_CONTROL = 4
};

struct Screen {
    U8G2 *display;
    ScreenRole role;
};

Screen screens[] = {
    {&rightDisplay, SCREEN_VALUES},
    {&auxLeft2, SCREEN_CONTROL},
};

// Forward declarations for control menu handling
UiFocus cc_to_focus(uint8_t cc);
void handle_control_menu_click(UiFocus f);
void handle_control_back();

void show_event_on_left(bool isNote, bool pressed, size_t row, size_t col, uint8_t code, uint8_t pin, uint8_t value) {
    ui.lastEvent.isNote = isNote;
    ui.lastEvent.code = code;
    ui.lastEvent.pressed = pressed;
    ui.lastEvent.timestamp = millis();
    // Pour limiter la charge I2C, ne rafraîchir que l'écran VALUES (+ CONTROL si menu ouvert)
    ui.dirtyFlags[0] = true; // VALUES screen
    if (ui.currentPage == UI_CONTROL_MENU) {
        ui.dirtyFlags[1] = true; // CONTROL menu visible
    }
    (void)row;
    (void)col;
    (void)pin;
    (void)value;
}

struct EncoderState {
    uint8_t pinA;
    uint8_t pinB;
    uint8_t cc;
    int8_t accumulator;
    uint8_t last;
};

EncoderState encoders[] = {
    {5, 6, ENCODER_1, 0, 0},
    {26, 27, ENCODER_2, 0, 0},
    {29, 30, ENCODER_3, 0, 0},
    {31, 32, ENCODER_4, 0, 0},
};

constexpr int8_t TRANSITION_TABLE[16] = {0, -1, +1, 0, +1, 0, 0, -1, -1, 0, 0, +1, 0, +1, -1, 0};

template <size_t N>
void configure_rows(const uint8_t (&rows)[N]) {
    for (uint8_t pin : rows) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
}

template <size_t N>
void configure_columns(const uint8_t (&cols)[N]) {
    for (uint8_t pin : cols) {
        pinMode(pin, INPUT_PULLUP);
    }
}

template <size_t N>
void set_active_row(const uint8_t (&rows)[N], uint8_t activePin) {
    for (uint8_t pin : rows) {
        digitalWrite(pin, pin == activePin ? LOW : HIGH);
    }
}

uint8_t apply_transposition(uint8_t note) {
    const int shifted = static_cast<int>(note) + (transposition * TRANSPOSE_SEMITONES);
    return static_cast<uint8_t>(constrain(shifted, 0, 127));
}

void send_octave_change() {
    const uint8_t value = static_cast<uint8_t>(transposition + MAX_TRANSPOSITION);
    link_send_cc(OCTAVE_CHANGE, value);
}

void handle_note_cell(size_t row, size_t col, bool pressed) {
    bool &prev = notePrev[row][col];
    if (pressed == prev) {
        return;
    }

    const uint8_t raw = NOTE_ADDRESSES[row][col];
    if (raw == 1) {
        prev = pressed;
        return;
    }

    if (pressed) {
        const uint8_t note = apply_transposition(raw);
        noteOnValue[row][col] = note;
        link_send_note_on(note, NOTE_VELOCITY);
        show_event_on_left(true, true, row, col, note, NOTE_COL_PINS[col], NOTE_VELOCITY);
    } else {
        const uint8_t note = noteOnValue[row][col];
        link_send_note_off(note);
        show_event_on_left(true, false, row, col, note, NOTE_COL_PINS[col], 0);
    }

    prev = pressed;
}

void scan_note_matrix() {
    for (uint8_t rowIdx = 0; rowIdx < NOTE_ROWS; ++rowIdx) {
        const uint8_t rowPin = NOTE_ROW_PINS[rowIdx];
        set_active_row(ROW_PINS, rowPin);

        for (uint8_t colIdx = 0; colIdx < NOTE_COLS; ++colIdx) {
            const bool pressed = digitalRead(NOTE_COL_PINS[colIdx]) == LOW;
            handle_note_cell(rowIdx, colIdx, pressed);
        }
    }
}

void process_plus_minus(bool pressed, bool isPlus, bool shiftActive, bool &heldFlag) {
    if (shiftActive) {
        if (pressed) {
            heldFlag = true;
            return;
        }

        if (heldFlag) {
            const int delta = isPlus ? 1 : -1;
            transposition = constrain(transposition + delta, MIN_TRANSPOSITION, MAX_TRANSPOSITION);
            send_octave_change();
            heldFlag = false;
        }
    } else {
        link_send_cc(isPlus ? PLUS_BUTTON : MINUS_BUTTON, pressed ? 127 : 0);
        heldFlag = pressed;
    }
}

void process_cc_cell(size_t row, size_t col, bool pressed, bool shiftActive) {
    bool &prev = ccPrev[row][col];
    if (pressed == prev) {
        return;
    }

    const bool isPlus = row == 0 && col == 3;
    const bool isMinus = row == 0 && col == 4;

    if (isPlus || isMinus) {
        process_plus_minus(pressed, isPlus, shiftActive, isPlus ? plusHeld : minusHeld);
        prev = pressed;
        return;
    }

    const uint8_t cc = CC_ADDRESSES[row][col];
    const UiFocus encFocus = cc_to_focus(cc);
    if (encFocus != FOCUS_NONE && pressed) {
        handle_control_menu_click(encFocus);
        prev = pressed;
        return;
    }

    if (cc == CONTROL_BUTTON && pressed) {
        handle_control_back();
        prev = pressed;
        return;
    }

    if (cc != DUMMY) {
        link_send_cc(cc, pressed ? 127 : 0);
        show_event_on_left(false, pressed, row, col, cc, CC_COL_PINS[col], pressed ? 127 : 0);
    }

    prev = pressed;
}

void scan_cc_matrix() {
    bool current[CC_ROWS][CC_COLS] = {};

    for (uint8_t rowIdx = 0; rowIdx < CC_ROWS; ++rowIdx) {
        const uint8_t rowPin = CC_ROW_PINS[rowIdx];
        set_active_row(ROW_PINS, rowPin);
        for (uint8_t colIdx = 0; colIdx < CC_COLS; ++colIdx) {
            current[rowIdx][colIdx] = (digitalRead(CC_COL_PINS[colIdx]) == LOW);
        }
    }

    const bool shiftActive = current[2][0];

    for (uint8_t rowIdx = 0; rowIdx < CC_ROWS; ++rowIdx) {
        for (uint8_t colIdx = 0; colIdx < CC_COLS; ++colIdx) {
            process_cc_cell(rowIdx, colIdx, current[rowIdx][colIdx], shiftActive);
        }
    }
}

uint8_t read_encoder_state(const EncoderState &enc) {
    return static_cast<uint8_t>((digitalRead(enc.pinA) << 1) | digitalRead(enc.pinB));
}

void init_encoders() {
    for (auto &enc : encoders) {
        pinMode(enc.pinA, INPUT_PULLUP);
        pinMode(enc.pinB, INPUT_PULLUP);
        enc.last = read_encoder_state(enc);
        enc.accumulator = 0;
    }
}

uint8_t detect_hw_oled_address() {
    // Detecte 0x3D (7-bit) puis 0x3C sur le bus matériel par défaut (Wire : SCL=19, SDA=18 sur Teensy 4.1).
    Wire.begin();
    const uint8_t candidates[] = {0x3D, 0x3C};
    for (uint8_t addr : candidates) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            return addr;
        }
    }
    return 0x3D; // défaut
}

void update_encoders() {
    for (auto &enc : encoders) {
        const uint8_t state = read_encoder_state(enc);
        const uint8_t index = static_cast<uint8_t>((enc.last << 2) | state);
        enc.accumulator += TRANSITION_TABLE[index];
        enc.last = state;

        if (enc.accumulator >= 4) {
            enc.accumulator = 0;
            if (ui.currentPage == UI_CONTROL_MENU && ui.focus == cc_to_focus(enc.cc)) {
                const MenuItem *items = nullptr;
                size_t count = 0;
                menu_get(ui.focus, items, count);
                if (count > 0) {
                    ui.menuIndex = (ui.menuIndex + 1) % static_cast<int>(count);
                    ui.dirtyFlags[1] = true;
                }
            } else {
                link_send_cc(enc.cc, 1);
                show_event_on_left(false, true, 0, 0, enc.cc, enc.pinA, 1);
            }
        } else if (enc.accumulator <= -4) {
            enc.accumulator = 0;
            if (ui.currentPage == UI_CONTROL_MENU && ui.focus == cc_to_focus(enc.cc)) {
                const MenuItem *items = nullptr;
                size_t count = 0;
                menu_get(ui.focus, items, count);
                if (count > 0) {
                    ui.menuIndex = (ui.menuIndex - 1);
                    if (ui.menuIndex < 0) ui.menuIndex = static_cast<int>(count) - 1;
                    ui.dirtyFlags[1] = true;
                }
            } else {
                link_send_cc(enc.cc, 127);
                show_event_on_left(false, true, 0, 0, enc.cc, enc.pinA, 127);
            }
        }
    }
}

void update_pitch_bend() {
    analog.update();
    if (!analog.hasChanged()) {
        return;
    }

    const int remapped = map(analog.getValue(), 0, 4095, 4095, 0);
    const uint8_t value = static_cast<uint8_t>(constrain(remapped >> 5, 0, 127));

    if (value != lastPitchValue) {
        lastPitchValue = value;
        link_send_cc(HORIZONTAL_PB_CC, value);
        show_event_on_left(false, true, 0, 0, HORIZONTAL_PB_CC, HORIZONTAL_PB_PIN, value);
    }
}

void print_build_banner() {
    Serial.println(build_tag());
}

void init_displays() {
    // OLED VALUES sur bus matériel (Wire)
    const uint8_t hwAddr7 = detect_hw_oled_address(); // 0x3D ou 0x3C (7-bit)
    rightDisplay.setI2CAddress(static_cast<uint8_t>(hwAddr7 << 1)); // 8-bit attendu par u8g2
    rightDisplay.setBusClock(100000);
    rightDisplay.begin();

    // OLED CONTROL sur bus logiciel (SCL=12, SDA=11). Adresse 0x7A = 0x3D décalé (8-bit attendu par U8g2).
    auxLeft2.setI2CAddress(0x7A);
    auxLeft2.begin();
}

void render_dirty_screens() {
    const uint32_t now = millis();
    if (now - ui.lastRenderMs < 33) {
        return;
    }
    ui.lastRenderMs = now;

    // Rendre au plus un écran par tick pour réduire le temps bloquant
    for (size_t i = 0; i < sizeof(screens) / sizeof(screens[0]); ++i) {
        if (!ui.dirtyFlags[i]) continue;
        switch (screens[i].role) {
            case SCREEN_VALUES:
                renderScreen3_Values(*screens[i].display, ui);
                break;
            case SCREEN_CONTROL:
                renderScreen4_Control(*screens[i].display, ui);
                break;
            default:
                ui.dirtyFlags[i] = false;
                continue;
        }
        ui.dirtyFlags[i] = false;
        break; // stop after one render to keep latency low
    }
}

void setup() {
    Serial.begin(115200);
    link_begin();
    init_displays();

    configure_rows(ROW_PINS);
    configure_columns(ALL_COL_PINS);
    init_encoders();

    analog.setAnalogResolution(4096);
    analog.setActivityThreshold(10.0f);

    print_build_banner();
    ui_mark_all_dirty();
    render_dirty_screens();
}

// --- CONTROL MENU HELPERS ---
constexpr int MENU_ITEMS = 3;

UiFocus cc_to_focus(uint8_t cc) {
    switch (cc) {
        case ENCODER_1_BUTTON: return FOCUS_ENC1;
        case ENCODER_2_BUTTON: return FOCUS_ENC2;
        case ENCODER_3_BUTTON: return FOCUS_ENC3;
        case ENCODER_4_BUTTON: return FOCUS_ENC4;
        default: return FOCUS_NONE;
    }
}

void handle_control_menu_click(UiFocus f) {
    if (f == FOCUS_NONE) return;
    if (ui.currentPage != UI_CONTROL_MENU) {
        ui.currentPage = UI_CONTROL_MENU;
        ui.focus = f;
        ui.menuIndex = 0;
        ui.dirtyFlags[1] = true;
        return;
    }
    if (ui.focus != f) {
        ui.focus = f;
        ui.menuIndex = 0;
        ui.dirtyFlags[1] = true;
        return;
    }
    // Second click on same encoder -> validate action (placeholder)
    menu_execute(f, ui.menuIndex);
    ui.currentPage = UI_HOME;
    ui.focus = FOCUS_NONE;
    ui.menuIndex = 0;
    ui.dirtyFlags[1] = true;
}

void handle_control_back() {
    if (ui.currentPage == UI_CONTROL_MENU) {
        ui.currentPage = UI_HOME;
        ui.focus = FOCUS_NONE;
        ui.menuIndex = 0;
        ui.dirtyFlags[1] = true;
    }
}

void loop() {
    // Scheduler temps réel strict pour la matrice (1 kHz)
    static uint32_t lastMatrixUs = 0;
    const uint32_t nowUs = micros();
    if (lastMatrixUs == 0) {
        lastMatrixUs = nowUs;
    }
    while ((uint32_t)(nowUs - lastMatrixUs) >= 1000) {
        lastMatrixUs += 1000;
        scan_note_matrix();
        scan_cc_matrix();
    }

    update_encoders();
    update_pitch_bend();
    render_dirty_screens();
}
