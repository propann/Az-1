#pragma once

#include <U8g2lib.h>
#include "UiState.h"
#include "build_info.h"

struct MenuItem {
    const char *label;
    void (*fn)();
};

void renderScreen3_Values(U8G2 &d, const UiState &state);
void renderScreen4_Control(U8G2 &d, const UiState &state);

void menu_get(UiFocus focus, const MenuItem *&items, size_t &count);
void menu_execute(UiFocus focus, int index);
