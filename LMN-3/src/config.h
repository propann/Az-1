#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- PINS ANALOGIQUES ---
const int JOYSTICK_X_PIN = A15; // Pin 39
const int JOYSTICK_Y_PIN = A14; // Pin 38
const int JOYSTICK_SEL_PIN = 37; // Pin 37 (bouton SEL)

// --- CODES MIDI (Messages) ---
#define DUMMY 255
const int KICK_PAD = 36;
const int SNARE_PAD = 38;
const int HAT_CLOSED_PAD = 42;
const int HAT_OPEN_PAD = 46;
const int CLAP_PAD = 39;
const int CRASH_PAD = 49;
const int RIDE_PAD = 51;
const int LOW_TOM_PAD = 45;
const int MID_TOM_PAD = 47;
const int HIGH_TOM_PAD = 50;
const int FLOOR_TOM_PAD = 41;
const int TAMB_PAD = 54;
const int RIMSHOT_PAD = 37;
const int UNDO_BUTTON = 24;
const int RECORD_BUTTON = 109;
const int PLAY_BUTTON = 110;
const int STOP_BUTTON = 111;
const int SETTINGS_BUTTON = 85;
const int TEMPO_BUTTON = 25;
const int ENCODER_1_BUTTON = 21;
const int ENCODER_2_BUTTON = 20;
const int ENCODER_3_BUTTON = 22;
const int ENCODER_4_BUTTON = 23;
const int OCTAVE_CHANGE = 117;
const int PLUS_BUTTON = 118;
const int MINUS_BUTTON = 119;
const int SUSTAIN_BUTTON = 64;
const int PANIC_BUTTON = 123;

// --- MATRICE : PINS DES LIGNES (ROWS) ---
const int ROW_0 = 24;
const int ROW_1 = 23;
const int ROW_2 = 34;
const int ROW_3 = 35;
const int ROW_4 = 28;

// --- MATRICE : PINS DES COLONNES (COLS) ---
const int COL_0 = 9;
const int COL_1 = 8;
const int COL_2 = 7;
const int COL_3 = 4;
const int COL_4 = 3;
const int COL_5 = 2;
const int COL_6 = 21;
const int COL_7 = 20;
const int COL_8 = 25;
const int COL_9 = 33; // LE HACK : Remplace la Pin 14 morte
const int COL_10 = 13;
const int COL_11 = 41;
const int COL_12 = 40;
const int COL_13 = 36;

#endif
