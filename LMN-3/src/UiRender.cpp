#include "UiRender.h"
#include <Arduino.h>

void renderScreen3_Values(U8G2 &d, const UiState &state) {
    d.clearBuffer();
    d.setFont(u8g2_font_6x12_tr);
    d.drawStr(0, 12, "VALUES");
    char line[24];
    if (state.lastEvent.isNote) {
        snprintf(line, sizeof(line), "Note %u %s", state.lastEvent.code, state.lastEvent.pressed ? "ON" : "OFF");
    } else {
        snprintf(line, sizeof(line), "CC %u val %u", state.lastEvent.code, state.lastEvent.pressed ? 127 : 0);
    }
    d.drawStr(0, 26, line);
    d.sendBuffer();
}

// --- Menu definitions ---
namespace {
void action_placeholder() {
    // placeholder; extend with real actions
}

const MenuItem menuEnc1[] = {
    {"Mute Track", action_placeholder},
    {"Record Arm", action_placeholder},
    {"Quantize", action_placeholder},
    {"Undo", action_placeholder},
};
const MenuItem menuEnc2[] = {
    {"Tempo Tap", action_placeholder},
    {"Swing +", action_placeholder},
    {"Swing -", action_placeholder},
    {"Undo", action_placeholder},
};
const MenuItem menuEnc3[] = {
    {"Copy", action_placeholder},
    {"Paste", action_placeholder},
    {"Slice", action_placeholder},
    {"Quantize", action_placeholder},
};
const MenuItem menuEnc4[] = {
    {"Next Track", action_placeholder},
    {"Prev Track", action_placeholder},
    {"Mute Track", action_placeholder},
    {"Record Arm", action_placeholder},
};
} // namespace

void menu_get(UiFocus focus, const MenuItem *&items, size_t &count) {
    switch (focus) {
        case FOCUS_ENC1: items = menuEnc1; count = sizeof(menuEnc1) / sizeof(menuEnc1[0]); return;
        case FOCUS_ENC2: items = menuEnc2; count = sizeof(menuEnc2) / sizeof(menuEnc2[0]); return;
        case FOCUS_ENC3: items = menuEnc3; count = sizeof(menuEnc3) / sizeof(menuEnc3[0]); return;
        case FOCUS_ENC4: items = menuEnc4; count = sizeof(menuEnc4) / sizeof(menuEnc4[0]); return;
        default: items = nullptr; count = 0; return;
    }
}

void menu_execute(UiFocus focus, int index) {
    const MenuItem *items = nullptr;
    size_t count = 0;
    menu_get(focus, items, count);
    if (!items || count == 0) return;
    if (index < 0 || index >= static_cast<int>(count)) return;
    if (items[index].fn) items[index].fn();
}

void renderScreen4_Control(U8G2 &d, const UiState &state) {
    d.clearBuffer();
    d.setFont(u8g2_font_6x12_tr);
    d.drawStr(0, 12, "CONTROL / MENU");
    d.drawStr(0, 24, build_tag().c_str());

    if (state.focus == FOCUS_NONE) {
        d.drawStr(0, 40, "Click encoder to open");
        d.drawStr(0, 56, "CTRL=Back");
        d.sendBuffer();
        return;
    }

    const MenuItem *items = nullptr;
    size_t count = 0;
    menu_get(state.focus, items, count);
    if (count == 0 || items == nullptr) {
        d.drawStr(0, 40, "No items");
        d.sendBuffer();
        return;
    }

    char title[24];
    switch (state.focus) {
        case FOCUS_ENC1: snprintf(title, sizeof(title), "ENC1 MENU"); break;
        case FOCUS_ENC2: snprintf(title, sizeof(title), "ENC2 MENU"); break;
        case FOCUS_ENC3: snprintf(title, sizeof(title), "ENC3 MENU"); break;
        case FOCUS_ENC4: snprintf(title, sizeof(title), "ENC4 MENU"); break;
        default: snprintf(title, sizeof(title), "MENU"); break;
    }
    d.drawStr(0, 40, title);

    const int visible = 4;
    int idx = state.menuIndex;
    if (idx < 0) idx = 0;
    if (idx >= static_cast<int>(count)) idx = static_cast<int>(count) - 1;
    int start = idx - 1;
    if (start < 0) start = 0;
    if (start > static_cast<int>(count) - visible) start = static_cast<int>(count) - visible;
    if (start < 0) start = 0;

    for (int i = 0; i < visible && (start + i) < static_cast<int>(count); ++i) {
        const int itemIdx = start + i;
        const int y = 54 + i * 10;
        if (itemIdx == idx) {
            d.drawBox(0, y - 8, 128, 10);
            d.setDrawColor(0);
            d.drawStr(2, y, items[itemIdx].label);
            d.setDrawColor(1);
        } else {
            d.drawStr(2, y, items[itemIdx].label);
        }
    }
    d.sendBuffer();
}
