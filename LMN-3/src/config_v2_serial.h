#pragma once

#include <Arduino.h>

// Analog inputs
constexpr int HORIZONTAL_PB_PIN = A15;

// CC values
constexpr uint8_t ENCODER_1 = 3;
constexpr uint8_t ENCODER_2 = 9;
constexpr uint8_t ENCODER_3 = 14;
constexpr uint8_t ENCODER_4 = 15;
constexpr uint8_t ENCODER_1_BUTTON = 20;
constexpr uint8_t ENCODER_2_BUTTON = 21;
constexpr uint8_t ENCODER_3_BUTTON = 22;
constexpr uint8_t ENCODER_4_BUTTON = 23;
constexpr uint8_t UNDO_BUTTON = 24;
constexpr uint8_t TEMPO_BUTTON = 25;
constexpr uint8_t SAVE_BUTTON = 26;
constexpr uint8_t SETTINGS_BUTTON = 85;
constexpr uint8_t TRACKS_BUTTON = 86;
constexpr uint8_t MIXER_BUTTON = 88;
constexpr uint8_t PLUGINS_BUTTON = 89;
constexpr uint8_t MODIFIERS_BUTTON = 90;
constexpr uint8_t SEQUENCERS_BUTTON = 102;
constexpr uint8_t LOOP_IN_BUTTON = 103;
constexpr uint8_t LOOP_OUT_BUTTON = 104;
constexpr uint8_t LOOP_BUTTON = 105;
constexpr uint8_t CUT_BUTTON = 106;
constexpr uint8_t PASTE_BUTTON = 107;
constexpr uint8_t SLICE_BUTTON = 108;
constexpr uint8_t RECORD_BUTTON = 109;
constexpr uint8_t PLAY_BUTTON = 110;
constexpr uint8_t STOP_BUTTON = 111;
constexpr uint8_t CONTROL_BUTTON = 112;
constexpr uint8_t OCTAVE_CHANGE = 117;
constexpr uint8_t PLUS_BUTTON = 118;
constexpr uint8_t MINUS_BUTTON = 119;
constexpr uint8_t DUMMY = 31;
constexpr uint8_t HORIZONTAL_PB_CC = 120;

// Row Pins
constexpr uint8_t ROW_0 = 24;
constexpr uint8_t ROW_1 = 23;
constexpr uint8_t ROW_2 = 34;
constexpr uint8_t ROW_3 = 35;
constexpr uint8_t ROW_4 = 28;

// Col Pins (pins 0/1 rerouted to 20/21; COL_9 rerouted to 33)
constexpr uint8_t COL_0 = 9;
constexpr uint8_t COL_1 = 8;
constexpr uint8_t COL_2 = 7;
constexpr uint8_t COL_3 = 4;
constexpr uint8_t COL_4 = 3;
constexpr uint8_t COL_5 = 2;
constexpr uint8_t COL_6 = 20;
constexpr uint8_t COL_7 = 21;
constexpr uint8_t COL_8 = 25;
constexpr uint8_t COL_9 = 33;
constexpr uint8_t COL_10 = 13;
constexpr uint8_t COL_11 = 41;
constexpr uint8_t COL_12 = 40;
constexpr uint8_t COL_13 = 36;
