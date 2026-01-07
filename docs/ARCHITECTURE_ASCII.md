# AZ-1 / LMN-3 — Schémas ASCII officiels

## 1) Vue globale du système

```
 [ MATRICE ]      [ ENCODEURS ]      [ ANALOG (PB) ]
     |                 |                 |
     +--------+--------+--------+--------+
              |      TEENSY 4.1         |
              | (scan + MIDI + UI loop)  |
              +--------+--------+--------+
                       |
                       +--> [ OLED3 (VALUES) ]
                       +--> [ OLED4 (CONTROL) ]
                       |
                  [ UART MIDI ]
                       |
                  [ SYNTH / DAW ]
```

## 2) Schéma détaillé — Matrice

```
Rows:    ROW_0 ROW_1 ROW_2 ROW_3 ROW_4
Cols:    COL_0 ... COL_5  COL_6  COL_7  COL_8 ... COL_13
Pins:    9..2            20     21     25..36 (cf. config_v2_serial.h)

Notes zone    : rows 3-4 x cols 0-13 (placeholders=1 ignorés)
CC/boutons    : rows 0-2 x cols 3-13

       COL_0 COL_1 COL_2 COL_3 ... COL_6 COL_7 ... COL_13
ROW_0   .     .     .    CC ...   CC    CC     CC
ROW_1   .     .     .    CC ...   CC    CC     CC
ROW_2   .     .     .    CC ...   CC    CC     CC
ROW_3   note  note  note note ... note  note   note
ROW_4   note  note  note note ... note  note   note

Règles :
- Edge detection uniquement (OFF->ON = NoteOn/CC127, ON->OFF = NoteOff/CC0).
- Placeholders valeur 1 ignorés (aucun envoi).
- Interdiction absolue d’utiliser/configurer les pins 0/1 dans la matrice.
- Colonnes déplacées : COL_6 = 20, COL_7 = 21 (pins UART préservées).
```

## 3) Schéma UART / MIDI

```
[ TEENSY TX1 (pin 1) ] ---> [ RX MIDI IN (synth/DAW) ]
[ TEENSY RX0 (pin 0) ] <--- [ TX MIDI OUT (optionnel) ]
[ GND ] ------------------- [ GND ]

Port : Serial1
Baud : 31250 (MIDI)
Canal : 1 (channel = 0)
Note : inversion TX/RX obligatoire, mauvais baud = pas de son.
```

## 4) UI / Écrans (V1.1 = 2 actifs)

```
[ OLED 3 ] VALUES   <= dernière note/CC, transpo (feedback)
[ OLED 4 ] CONTROL  <= menus, actions (seul écran à afficher les menus)

Entrées -> affichage :
- Matrice/Encodeurs/Analog -> VALUES (feedback)
- Clics encodeurs -> CONTROL (menus sur écran 4 uniquement)
```

## 5) Flux UI (HOME → MENU)

```
HOME
 |
 | (clic encodeur)
 v
MENU (SCREEN 4)
 |
 | rotate (encodeur associé)
 v
SELECT ITEM
 |
 | click (encodeur)
 v
ACTION
 |
 | CONTROL button (BACK/CLOSE)
 v
HOME
```

## 6) Priorités temps réel

```
1) Scan matrice (notes/boutons)
2) Émission MIDI
3) Encodeurs
4) Analog (pitchbend)
5) UI / écrans

- UI ne doit jamais bloquer MIDI.
- Aucun redraw écran dans le scan.
- Rendu UI limité (~20–30 FPS) via dirty flags.
```

## 7) Légende et règles

```
[ BLOC ]  : module matériel ou fonctionnel
-->       : flux unidirectionnel
<-->      : flux bidirectionnel (optionnel)
. / note / CC : zones de matrice
```

Règles figées :
- Ce schéma est la référence. Toute modification matérielle ou protocolaire doit mettre à jour ce document.
- Conformité avec ARCHITECTURE.md, UI_MENUS.md, MIDI_PROTOCOL_TIMING.md.
