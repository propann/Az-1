#pragma once

#include <Arduino.h>

enum UiPage {
    UI_HOME,
    UI_CONTROL_MENU,
};

enum UiFocus {
    FOCUS_NONE,
    FOCUS_ENC1,
    FOCUS_ENC2,
    FOCUS_ENC3,
    FOCUS_ENC4,
};

struct UiEvent {
    bool isNote = false;
    uint8_t code = 0;
    bool pressed = false;
    uint32_t timestamp = 0;
};

struct UiState {
    UiPage currentPage = UI_HOME;
    UiFocus focus = FOCUS_NONE;
    int menuIndex = 0;
    UiEvent lastEvent;
    // dirtyFlags[0] -> OLED3 (VALUES), dirtyFlags[1] -> OLED4 (CONTROL)
    bool dirtyFlags[2] = {true, true};
    uint32_t lastRenderMs = 0;
};

extern UiState ui;

inline void ui_mark_all_dirty() {
    ui.dirtyFlags[0] = true;
    ui.dirtyFlags[1] = true;
}
