#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Encoder.h>
#include "config.h"

// --- 1. CONFIGURATION MIDI ---
HardwareSerial &midiSerial = Serial1; // Pins 0 et 1
const int MIDI_BAUD = 31250;
const uint8_t MIDI_CH_NOTES = 1;
const uint8_t MIDI_CH_CC = 2;
const uint8_t MIDI_CH_DRUMS = 10; // Canal standard pour les percussions

// --- 2. ECRANS (U8g2 - Wire1 - Pins 17/18) ---
U8G2_SSD1306_128X64_NONAME_F_HW_I2C leftEye(U8G2_R0, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightEye(U8G2_R0, U8X8_PIN_NONE);

// --- 3. ENCODEURS (Pins uniques) ---
Encoder enc1(5, 6);
Encoder enc2(26, 27);
Encoder enc3(29, 30);
Encoder enc4(31, 32);

long pos1=-999, pos2=-999, pos3=-999, pos4=-999;
unsigned long popupTimer = 0;
int activeEncID = -1;
int lastJoyX = -1;
int lastJoyY = -1;
int lastMidiJoyX = -1;
int lastMidiJoyY = -1;
bool lastJoySel = false;
unsigned long joyTimer = 0;
bool testMode = false;

enum AppState { STATE_IDLE, STATE_MENU, STATE_TEST };
AppState currentState = STATE_IDLE;

int globalOctave = 0; // Transposition (-3 à +3)
bool encBtnPressed[4] = {false, false, false, false};
uint32_t encBtnStart[4] = {0, 0, 0, 0};
uint32_t nextBlinkMs = 0;
uint32_t menuOpenTime = 0; // Timer pour éviter le rebond à l'ouverture
uint32_t blinkStartMs = 0;
bool blinking = false;
int eyeDx = 0;
int eyeDy = 0;
uint32_t lastNoteMs = 0;

void drawEyes(int dx, int dy);
void drawPopup(int id, long val);

// --- STRUCTURE MENUS ---
struct MenuOption { const char* name; int cc; };

// Définition des menus par encodeur
// Configuration optimisée pour Dexed / Synthés
// CC Standards : 74=Cutoff, 71=Res, 73=Attack, 72=Release, 91=Reverb
const MenuOption menuEnc1[] = { 
    {"CUTOFF", 74}, {"RESONANCE", 71}, {"MOD WHEEL", 1}, {"BREATH", 2},
    {"FOOT CTRL", 4}, {"PORTA TIME", 5}, {"PORTA SW", 65} 
};
const MenuOption menuEnc2[] = { 
    {"ATTACK", 73}, {"DECAY", 75}, {"SUSTAIN LVL", 79}, {"RELEASE", 72},
    {"SUSTAIN PED", 64}, {"SOSTENUTO", 66}, {"SOFT PEDAL", 67}
};
const MenuOption menuEnc3[] = { {"VOLUME", 7}, {"PAN", 10}, {"REVERB", 91}, {"CHORUS", 93} };
const MenuOption menuEnc4[] = { {"RETOUR", -1}, {"MODE TEST", -2} };

// État des menus et assignations
const MenuOption* activeMenu = nullptr;
int activeMenuSize = 0;
int activeMenuEncoder = -1; // Quel encodeur a ouvert le menu (0-3)
int menuIndex = 0;

// Assignations actuelles (CC) et Noms pour l'affichage
int encoderCCs[4] = { 74, 73, 91, -1 }; 
const char* encoderParamNames[4] = { "CUTOFF", "ATTACK", "REVERB", "SYSTEM" };

void drawMenu() {
    rightEye.clearBuffer(); // Correction : Menu sur l'écran de droite (CONTROL)
    rightEye.setFont(u8g2_font_ncenB10_tr);
    rightEye.setCursor(20, 12);
    rightEye.print("MENU ENC "); 
    rightEye.print(activeMenuEncoder + 1);
    
    rightEye.setFont(u8g2_font_6x12_tr);
    
    // Gestion du défilement (Scrolling) : on affiche 3 lignes max
    int visibleLines = 3;
    int startIdx = 0;
    if (menuIndex >= visibleLines) {
        startIdx = menuIndex - (visibleLines - 1);
    }

    for(int i=0; i<visibleLines; i++) {
        int idx = startIdx + i;
        if(idx >= activeMenuSize) break;
        // Espacement réduit (12px) et départ plus haut (26px) pour tout faire rentrer (64px max)
        int y = 26 + i * 12;
        if(idx == menuIndex) rightEye.drawStr(5, y, ">");
        rightEye.drawStr(20, y, activeMenu[idx].name);
    }
    rightEye.sendBuffer();
}

void exitTestMode() {
    currentState = STATE_IDLE;
    // On redessine les réglages pour qu'ils persistent
    drawPopup(0, pos1);
    drawPopup(1, pos2);
}

void initTestMode() {
    currentState = STATE_TEST;
    rightEye.clearBuffer(); // Correction : Test Mode sur l'écran de droite
    rightEye.setFont(u8g2_font_ncenB10_tr);
    rightEye.drawStr(15, 25, "TEST MODE");
    rightEye.setFont(u8g2_font_6x12_tr);
    rightEye.drawStr(10, 45, "PRESS ANY KEY");
    rightEye.drawStr(10, 58, "HOLD ENC4 > EXIT");
    rightEye.sendBuffer();
}

// --- 4. MATRICE ---
constexpr uint8_t ROW_COUNT = 5;
constexpr uint8_t COL_COUNT = 14;
const int rowPins[ROW_COUNT] = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};
const int colPins[COL_COUNT] = {COL_0, COL_1, COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8, COL_9, COL_10, COL_11, COL_12, COL_13};
bool prevState[ROW_COUNT][COL_COUNT];
uint32_t lastChangeMs[ROW_COUNT][COL_COUNT];

const int midiMap[ROW_COUNT][COL_COUNT] = {
    {DUMMY, DUMMY, DUMMY, KICK_PAD, SNARE_PAD, HAT_CLOSED_PAD, HAT_OPEN_PAD, CLAP_PAD, DUMMY, ENCODER_1_BUTTON, ENCODER_2_BUTTON, DUMMY, ENCODER_3_BUTTON, ENCODER_4_BUTTON},
    {DUMMY, DUMMY, DUMMY, LOW_TOM_PAD, MID_TOM_PAD, HIGH_TOM_PAD, CRASH_PAD, RIDE_PAD, DUMMY, DUMMY, DUMMY, DUMMY, DUMMY, DUMMY},
    {DUMMY, DUMMY, DUMMY, FLOOR_TOM_PAD, RIMSHOT_PAD, TAMB_PAD, 55, 57, PLUS_BUTTON, MINUS_BUTTON, PLAY_BUTTON, STOP_BUTTON, SUSTAIN_BUTTON, PANIC_BUTTON},
    {52, 54, 56, 58, 59, 61, 63, 64, 66, 68, 70, 71, 73, 75},
    {53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76}
};

// --- 5. FONCTIONS ENVOI MIDI (SON) ---
void sendMidi(uint8_t status, uint8_t data1, uint8_t data2) {
    midiSerial.write(status);
    midiSerial.write(data1);
    midiSerial.write(data2);
}

// --- 6. GRAPHISMES ---
void drawPopup(int id, long val) {
    activeEncID = id; popupTimer = millis();
    
    // Cible : Encodeur 1 -> Gauche, Encodeur 2 -> Droite
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C *target = &leftEye;
    if (id == 1) target = &rightEye;

    // Si le menu est ouvert sur l'écran de droite, on n'affiche pas le popup par-dessus
    if (id == 1 && currentState == STATE_MENU) return;

    target->clearBuffer();
    target->drawFrame(0,0,128,64);
    target->setFont(u8g2_font_6x12_tr);
    target->setCursor(10, 20); target->print(encoderParamNames[id]); // Affiche le nom du paramètre
    target->setFont(u8g2_font_ncenB14_tr);
    target->setCursor(40, 45); target->print(abs(val % 128));
    target->sendBuffer();
}

void drawOctave() {
    leftEye.clearBuffer();
    leftEye.drawFrame(0,0,128,64);
    leftEye.setFont(u8g2_font_ncenB14_tr);
    leftEye.setCursor(20, 40); 
    leftEye.print("OCTAVE "); leftEye.print(globalOctave > 0 ? "+" : ""); leftEye.print(globalOctave);
    leftEye.sendBuffer();
}

void cleanLeft() {
    leftEye.clearBuffer();
    leftEye.setFont(u8g2_font_ncenB14_tr);
    leftEye.drawStr(25, 40, "LMN-3");
    leftEye.sendBuffer();
}

void cleanRight() {
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_ncenB14_tr);
    rightEye.drawStr(25, 40, "AZ-1");
    rightEye.sendBuffer();
}

void drawBootLogo() {
    leftEye.clearBuffer();
    rightEye.clearBuffer();
    leftEye.setFont(u8g2_font_ncenB14_tr);
    rightEye.setFont(u8g2_font_ncenB14_tr);
    leftEye.drawStr(6, 36, "AZOTH-1");
    rightEye.drawStr(6, 36, "AZOTH-1");
    leftEye.sendBuffer();
    rightEye.sendBuffer();
    delay(800);
}

static inline bool isDrumNote(int code) { return code >= 35 && code <= 81; }

void drawEyes(int dx, int dy) {
    const int cx = 64;
    const int cy = 32;
    const int eyeR = 20;
    uint32_t now = millis();
    if (!blinking && now > nextBlinkMs) {
        blinking = true;
        blinkStartMs = now;
    }
    uint32_t blinkElapsed = now - blinkStartMs;
    int blinkCover = 0;
    if (blinking) {
        if (blinkElapsed < 120) {
            blinkCover = map((int)blinkElapsed, 0, 120, 0, eyeR * 2);
        } else if (blinkElapsed < 200) {
            blinkCover = map((int)blinkElapsed, 120, 200, eyeR * 2, 0);
        } else {
            blinking = false;
            blinkCover = 0;
            nextBlinkMs = now + 2500 + (now % 1500);
        }
    }

    rightEye.clearBuffer();
    rightEye.setDrawColor(1);
    rightEye.drawDisc(cx, cy, eyeR);
    rightEye.setDrawColor(0);
    rightEye.drawDisc(cx - dx, cy - dy, 8);
    if (blinkCover > 0) {
        rightEye.drawBox(cx - eyeR - 1, cy - eyeR - 1, eyeR * 2 + 2, blinkCover);
    }
    rightEye.setDrawColor(1);
    rightEye.sendBuffer();
}

const char* getButtonName(uint8_t row, uint8_t col, int code) {
    switch (code) {
        case UNDO_BUTTON: return "UNDO";
        case RECORD_BUTTON: return "RECORD";
        case PLAY_BUTTON: return "PLAY";
        case STOP_BUTTON: return "STOP";
        case SETTINGS_BUTTON: return "SETTINGS";
        case TEMPO_BUTTON: return "TEMPO";
        case ENCODER_1_BUTTON: return "ENC 1";
        case ENCODER_2_BUTTON: return "ENC 2";
        case ENCODER_3_BUTTON: return "ENC 3";
        case ENCODER_4_BUTTON: return "ENC 4";
        case PLUS_BUTTON: return "PLUS";
        case MINUS_BUTTON: return "MINUS";
        case SUSTAIN_BUTTON: return "SUSTAIN";
        case PANIC_BUTTON: return "PANIC";
    }

    if (isDrumNote(code)) {
        switch (code) {
            case 35: return "KICK 2";
            case 36: return "KICK";
            case 37: return "RIM";
            case 38: return "SNARE";
            case 39: return "CLAP";
            case 40: return "SNARE 2";
            case 41: return "FLOOR T";
            case 42: return "HAT C";
            case 43: return "FLOOR T2";
            case 45: return "LOW TOM";
            case 46: return "HAT O";
            case 47: return "MID TOM";
            case 49: return "CRASH";
            case 50: return "HI TOM";
            case 51: return "RIDE";
            case 54: return "TAMB";
            case 55: return "SPLASH";
            case 57: return "CRASH 2";
            default: return "DRUM";
        }
    }
    if (code == DUMMY) return "NA";
    return (row >= 3) ? "NOTE" : "CC";
}

// Variables pour éviter le redessin inutile en mode test (anti-flicker)
uint8_t lastTestRow=255, lastTestCol=255;
int lastTestCode=-1;
bool lastTestPressed=false;

void drawRightStatus(uint8_t row, uint8_t col, int code, bool pressed, uint8_t chan, uint8_t type) {
    rightEye.clearBuffer(); // Correction : Debug sur l'écran de droite
    if (currentState == STATE_TEST) {
        rightEye.setDrawColor(1);
        rightEye.drawBox(0, 0, 128, 10);
        rightEye.setDrawColor(0);
        rightEye.setFont(u8g2_font_5x8_tr);
        rightEye.setCursor(4, 8);
        rightEye.print("TEST");
        rightEye.setDrawColor(1);
    }
    rightEye.setFont(u8g2_font_ncenB14_tr);
    rightEye.drawStr(25, 22, "AZ-1");
    rightEye.setFont(u8g2_font_5x8_tr);
    rightEye.setCursor(6, 34);
    rightEye.print("R"); rightEye.print(row);
    rightEye.print(" P"); rightEye.print(rowPins[row]);
    rightEye.print(" C"); rightEye.print(col);
    rightEye.print(" P"); rightEye.print(colPins[col]);
    rightEye.setCursor(6, 46);
    if (code == DUMMY) {
        rightEye.print("EMPTY");
    } else {
        rightEye.print("CH");
        rightEye.print(chan);
        rightEye.print(" ");
        if (type == 0xB0) rightEye.print("CC ");
        else rightEye.print("NOTE ");
        rightEye.print(code);
        rightEye.print(pressed ? " ON" : " OFF");
    }
    rightEye.setCursor(6, 58);
    rightEye.print(getButtonName(row, col, code));
    
    // Anti-spam : on n'envoie au contrôleur que si c'est une nouvelle info
    if (row == lastTestRow && col == lastTestCol && code == lastTestCode && pressed == lastTestPressed) return;
    lastTestRow = row; lastTestCol = col; lastTestCode = code; lastTestPressed = pressed;
    
    rightEye.sendBuffer();
}

void drawRightJoystick(int x, int y, bool selPressed) {
    rightEye.clearBuffer(); // Correction : Debug Joystick sur l'écran de droite
    rightEye.setFont(u8g2_font_ncenB14_tr);
    rightEye.drawStr(25, 22, "AZ-1");
    rightEye.setFont(u8g2_font_5x8_tr);
    rightEye.setCursor(6, 10);
    rightEye.print("SEL ");
    rightEye.print(selPressed ? "ON" : "OFF");
    rightEye.setFont(u8g2_font_6x12_tr);
    rightEye.setCursor(6, 42);
    rightEye.print("JOY X ");
    rightEye.print(x);
    rightEye.setCursor(6, 56);
    rightEye.print("JOY Y ");
    rightEye.print(y);
    rightEye.sendBuffer();
}

// --- 7. SETUP & LOOP ---
void setup() {
    midiSerial.begin(MIDI_BAUD);
    
    // Matrice: colonnes en entrée pullup, lignes en sortie.
    for (int c=0; c<COL_COUNT; c++) pinMode(colPins[c], INPUT_PULLUP);
    for (int r=0; r<ROW_COUNT; r++) { pinMode(rowPins[r], OUTPUT); digitalWrite(rowPins[r], HIGH); }
    for (int r=0; r<ROW_COUNT; r++) {
        for (int c=0; c<COL_COUNT; c++) prevState[r][c] = false;
    }
    for (int r=0; r<ROW_COUNT; r++) {
        for (int c=0; c<COL_COUNT; c++) lastChangeMs[r][c] = 0;
    }
    lastJoyX = analogRead(JOYSTICK_X_PIN);
    lastJoyY = analogRead(JOYSTICK_Y_PIN);
    pinMode(JOYSTICK_SEL_PIN, INPUT_PULLUP);
    // Initialisation des valeurs MIDI pour éviter un envoi massif au démarrage
    lastMidiJoyX = lastJoyX;
    lastMidiJoyY = lastJoyY;
    
    lastJoySel = (digitalRead(JOYSTICK_SEL_PIN) == LOW);

    Wire1.setSDA(18); Wire1.setSCL(17);
    Wire1.begin();
    leftEye.setI2CAddress(0x3D * 2); leftEye.begin(); // Inversion A/B (Swap)
    rightEye.setI2CAddress(0x3C * 2); rightEye.begin(); // Inversion A/B (Swap)
    
    drawBootLogo();
    cleanLeft();
    cleanRight();
    nextBlinkMs = millis() + 1500;
}

void loop() {
    // A. SCAN MATRICE (Version Securisee)
    const uint32_t debounceMs = 50; // Filtre anti-rebond très fort (50ms)
    for (uint8_t r = 0; r < ROW_COUNT; r++) {
        digitalWrite(rowPins[r], LOW);
        delayMicroseconds(1000); // Stabilisation MAXIMALE (1ms) pour éviter le ghosting entre lignes
        for (uint8_t c = 0; c < COL_COUNT; c++) {
            // Lecture double pour filtrer les parasites (Glitch Filter)
            bool reading = (digitalRead(colPins[c]) == LOW);
            if (reading != prevState[r][c]) {
                delayMicroseconds(500); // On attend 0.5ms pour confirmer
                if ((digitalRead(colPins[c]) == LOW) != reading) continue; // Si ça a changé, c'était du bruit -> on ignore

                bool pressed = reading;
                uint32_t now = millis();
                if (now - lastChangeMs[r][c] < debounceMs) continue;
                lastChangeMs[r][c] = now;
                prevState[r][c] = pressed;

                int code = midiMap[r][c];
                uint8_t chan = 1;
                uint8_t type = 0;

                if (code != DUMMY) {
                    // Séparation stricte : Clavier (Rangs 3-4) vs Pads/Boutons (Rangs 0-2)
                    if (r >= 3) {
                        // CLAVIER : Toujours des notes mélodiques sur Canal 1
                        chan = MIDI_CH_NOTES;
                        // Application de l'octave
                        int note = code + (globalOctave * 12);
                        // Sécurité pour rester dans la plage MIDI (0-127)
                        if (note < 0) note = 0;
                        if (note > 127) note = 127;
                        code = note; // On remplace le code par la note transposée
                        type = pressed ? 0x90 : 0x80;
                    } else {
                        // PADS & BOUTONS
                        // Interception des boutons Octave
                        if (code == PLUS_BUTTON && pressed) {
                            if (globalOctave < 3) globalOctave++;
                            drawOctave();
                            continue; // On n'envoie pas de MIDI pour ce bouton interne
                        } else if (code == MINUS_BUTTON && pressed) {
                            if (globalOctave > -3) globalOctave--;
                            drawOctave();
                            continue;
                        }
                        // Exception pour SUSTAIN (64) qui est dans la plage des notes de batterie
                        else if (code == SUSTAIN_BUTTON || code == PANIC_BUTTON) {
                            chan = MIDI_CH_CC;
                            type = 0xB0;
                        }
                        else if (isDrumNote(code)) {
                            chan = MIDI_CH_DRUMS; // Pads -> Canal 10
                            type = pressed ? 0x90 : 0x80;
                        } else {
                            chan = MIDI_CH_CC;    // Boutons -> Canal 2 (CC)
                            type = 0xB0;
                        }
                    }
                    if (pressed) {
                        eyeDx = map(c, 0, COL_COUNT - 1, -8, 8);
                        eyeDy = map(r, 0, ROW_COUNT - 1, -6, 6);
                        lastNoteMs = millis();
                    }
                    if (currentState == STATE_TEST) drawRightStatus(r, c, code, pressed, chan, type);
                    sendMidi(type | (chan - 1), code, pressed ? 100 : 0);
                }

                // GESTION DES BOUTONS ENCODEURS (1, 2, 3, 4)
                int encId = -1;
                if (code == ENCODER_1_BUTTON) encId = 0;
                else if (code == ENCODER_2_BUTTON) encId = 1;
                else if (code == ENCODER_3_BUTTON) encId = 2;
                else if (code == ENCODER_4_BUTTON) encId = 3;

                if (encId != -1) {
                    if (pressed && !encBtnPressed[encId]) {
                        encBtnPressed[encId] = true;
                        encBtnStart[encId] = millis();
                    } else if (!pressed && encBtnPressed[encId]) {
                        uint32_t held = millis() - encBtnStart[encId];
                        encBtnPressed[encId] = false;
                        
                        // --- LOGIQUE MENU ---
                        if (currentState == STATE_IDLE) {
                            // Ouverture du menu correspondant à l'encodeur
                            if (held < 800) {
                                currentState = STATE_MENU;
                                activeMenuEncoder = encId;
                                menuIndex = 0;
                                menuOpenTime = millis(); // Sécurité anti-rebond à l'ouverture
                                
                                // Reset des positions pour éviter un saut immédiat
                                pos1 = enc1.read();
                                pos2 = enc2.read();
                                pos3 = enc3.read();
                                pos4 = enc4.read();

                                // Sélection du bon menu
                                if (encId == 0) { activeMenu = menuEnc1; activeMenuSize = 7; }
                                else if (encId == 1) { activeMenu = menuEnc2; activeMenuSize = 7; }
                                else if (encId == 2) { activeMenu = menuEnc3; activeMenuSize = 4; }
                                else { activeMenu = menuEnc4; activeMenuSize = 2; }
                                drawMenu();
                            }
                        } else if (currentState == STATE_MENU) {
                            // Appui long (n'importe quel encodeur) -> sortie menu
                            if (held >= 600) {
                                exitTestMode();
                            } else if (encId == activeMenuEncoder) {
                                // Appui court : validation (avec sécurité anti-rebond)
                                if (held < 600 && (millis() - menuOpenTime > 250)) {
                                    int selectedCC = activeMenu[menuIndex].cc;
                                    
                                    if (selectedCC == -1) { // RETOUR
                                        exitTestMode();
                                    } else if (selectedCC == -2) { // MODE TEST
                                        initTestMode();
                                    } else {
                                        // Assignation du paramètre à l'encodeur
                                        encoderCCs[encId] = selectedCC;
                                        encoderParamNames[encId] = activeMenu[menuIndex].name;
                                        exitTestMode(); // Retour aux yeux après sélection
                                    }
                                }
                            }
                        } else if (currentState == STATE_TEST) {
                            // Sortie du mode test par appui long
                            if (held >= 600) {
                                exitTestMode();
                            }
                        }
                    }
                }
            }
        }
        digitalWrite(rowPins[r], HIGH);
    }

    // B. SCAN ENCODEURS + POPUPS
    // Gestion unifiée : Navigation Menu OU Envoi MIDI
    long n1 = enc1.read(); 
    if (currentState == STATE_MENU && activeMenuEncoder == 0) {
        if (!encBtnPressed[0] && abs(n1 - pos1) >= 4) { // 4 crans pour stabiliser
            if (n1 > pos1) { if (menuIndex < activeMenuSize - 1) menuIndex++; }
            else { if (menuIndex > 0) menuIndex--; }
            drawMenu();
            pos1 = n1;
        }
    } else if (n1 != pos1) {
        if (currentState == STATE_TEST || currentState == STATE_IDLE) {
            drawPopup(0, n1);
            sendMidi(0xB0 | (MIDI_CH_CC-1), encoderCCs[0], abs(n1%128)); 
        }
        pos1 = n1;
    }

    long n2 = enc2.read(); 
    if (currentState == STATE_MENU && activeMenuEncoder == 1) {
        if (!encBtnPressed[1] && abs(n2 - pos2) >= 4) {
            if (n2 > pos2) { if (menuIndex < activeMenuSize - 1) menuIndex++; }
            else { if (menuIndex > 0) menuIndex--; }
            drawMenu();
            pos2 = n2;
        }
    } else if (n2 != pos2) {
        if (currentState == STATE_TEST || currentState == STATE_IDLE) {
            drawPopup(1, n2);
            sendMidi(0xB0 | (MIDI_CH_CC-1), encoderCCs[1], abs(n2%128)); 
        }
        pos2 = n2;
    }

    long n3 = enc3.read(); 
    if (currentState == STATE_MENU && activeMenuEncoder == 2) {
        if (!encBtnPressed[2] && abs(n3 - pos3) >= 4) {
            if (n3 > pos3) { if (menuIndex < activeMenuSize - 1) menuIndex++; }
            else { if (menuIndex > 0) menuIndex--; }
            drawMenu();
            pos3 = n3;
        }
    } else if (n3 != pos3) {
        if (currentState == STATE_TEST || currentState == STATE_IDLE) {
            drawPopup(2, n3);
            sendMidi(0xB0 | (MIDI_CH_CC-1), encoderCCs[2], abs(n3%128)); 
        }
        pos3 = n3;
    }
    
    long n4 = enc4.read(); 
    if (currentState == STATE_MENU && activeMenuEncoder == 3) {
        if (!encBtnPressed[3] && abs(n4 - pos4) >= 4) {
            if (n4 > pos4) { if (menuIndex < activeMenuSize - 1) menuIndex++; }
            else { if (menuIndex > 0) menuIndex--; }
            drawMenu();
            pos4 = n4;
        }
    } else if (n4 != pos4) {
        if (currentState == STATE_TEST || currentState == STATE_IDLE) {
            drawPopup(3, n4);
            sendMidi(0xB0 | (MIDI_CH_CC-1), 73, abs(n4%128)); // Enc 4 reste fixe pour l'instant
        }
        pos4 = n4;
    }

    // C. JOYSTICK
    int rawX = analogRead(JOYSTICK_X_PIN);
    int rawY = analogRead(JOYSTICK_Y_PIN);
    // Zone morte (Deadzone) : on force à 512 si on est proche du centre (+/- 60)
    int joyX = (abs(rawX - 512) < 80) ? 512 : rawX; // Zone morte augmentée
    int joyY = (abs(rawY - 512) < 80) ? 512 : rawY;

    if (currentState == STATE_IDLE) {
        // --- MODE NORMAL : ANIMATION YEUX + MIDI ---
        

        // 2. Envoi MIDI (Pitch Bend sur X, Modulation sur Y)
        // Pitch Bend (14 bits) : Axe X
        if (abs(joyX - lastMidiJoyX) > 8) { // Seuil pour éviter le spam
            int bend = map(joyX, 0, 1023, 0, 16383);
            // Pitch Bend message: Status 0xE0 | Channel, LSB, MSB
            sendMidi(0xE0 | (MIDI_CH_NOTES - 1), bend & 0x7F, (bend >> 7) & 0x7F);
            lastMidiJoyX = joyX;
        }

        // Filter Cutoff (CC 74) : Axe Y
        if (abs(joyY - lastMidiJoyY) > 8) {
            int val = map(joyY, 0, 1023, 0, 127); // 0 en bas, 127 en haut
            sendMidi(0xB0 | (MIDI_CH_NOTES - 1), 74, val);
            lastMidiJoyY = joyY;
        }

    } else if (currentState == STATE_TEST) {
        // --- MODE TEST : DEBUG AFFICHAGE ---
        bool joySel = (digitalRead(JOYSTICK_SEL_PIN) == LOW);
        // On affiche seulement si le joystick bouge significativement (> 15) pour ne pas cacher les boutons
        if (abs(joyX - lastJoyX) > 15 || abs(joyY - lastJoyY) > 15 || joySel != lastJoySel) {
            drawRightJoystick(joyX, joyY, joySel);
            lastJoyX = joyX;
            lastJoyY = joyY;
            lastJoySel = joySel;
        }
    }
}
