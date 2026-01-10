# AZ-1 / LMN-3 — Architecture et règles définitives (V1.1 = 2 écrans)

## 1. Présentation générale
- Projet : AZ-1 / LMN-3, contrôleur MIDI matériel basé sur Teensy 4.1.
- Périphériques : matrice de boutons/notes, 4 encodeurs rotatifs avec clic, 2 écrans OLED actifs (VALUES et CONTROL) en V1.1.
- Objectif : réactivité, stabilité, zéro doublon d’événements.
- Note : OLED1/OLED2 sont retirés en V1.1 pour réduire la charge I2C (seuls OLED3/OLED4 restent actifs).

## 2. Sortie MIDI (son)
- Protocole : MIDI standard uniquement (Serial1).
- Notes : canal 1 (MIDI_CH_NOTES).
- CC : canal 2 (MIDI_CH_CC).
- Status bytes :
  - NoteOn : `0x90 | (chan-1)`
  - NoteOff : `0x80 | (chan-1)` (ou NoteOn vel=0)
  - CC : `0xB0 | (chan-1)`
  - Pitchbend : `0xE0 | (chan-1)` (14 bits LSB/MSB)
- Vitesse/valeur : 100 a l'appui, 0 au relachement.
- Baudrate : 31250.
- Aucune sortie USB MIDI utilisée.

## 3. UART / Liaison MIDI
- Port : `Serial1`.
- Pins : RX = 0, TX = 1.
- Câblage : TX -> RX, RX -> TX, GND commun obligatoire.
- Règle : RX/TX inversés ou baud incorrect = pas de son.

## 4. Matrice (règles figées)
- Scan rapide et prioritaire.
- Détection d’arêtes obligatoire :
  - OFF -> ON : 1 événement (NoteOn/CC 127).
  - ON -> OFF : 1 événement (NoteOff/CC 0).
  - Maintien : aucun renvoi.
- Notes placeholder DUMMY (255) ignorées (aucun envoi).
- Interdiction absolue d’utiliser/configurer les pins 0/1 dans la matrice.
- Pads batterie : notes GM (35..81) envoyées sur canal 1 (NoteOn/NoteOff).
- Boutons controle restants en CC canal 2 : UNDO, RECORD, PLAY, STOP, SETTINGS, TEMPO.
- Anti-rebond : ~25 ms par touche.

## 4.1 Mapping matrice (zones logiques)
- Lignes R0..R2 : pads batterie + controle + fonctions systeme.
- Lignes R3..R4 : zone notes (canal 1, NoteOn/NoteOff).

## 4.2 Mapping matrice (R/C)
- R0: C3 KICK, C4 SNARE, C5 HAT_CLOSED, C6 HAT_OPEN, C7 CLAP.
- R0: C9 ENC1, C10 ENC2, C12 ENC3, C13 ENC4.
- R1: C3 LOW_TOM, C4 MID_TOM, C5 HIGH_TOM, C6 CRASH, C7 RIDE.
- R2: C3 FLOOR_TOM, C4 RIMSHOT, C5 TAMB, C6 NOTE 55 (SPLASH), C7 NOTE 57 (CRASH 2).
- R2: C8 PLUS, C9 MINUS, C10 PLAY, C11 STOP, C12 SUSTAIN, C13 PANIC.

## 5. Colonnes déplacées (conflit UART évité)
- Colonnes historiquement sur 0/1 déplacées sur 20/21.
- Mapping figé :
  - `COL_6 = 20`
  - `COL_7 = 21`
- Règle : si la matrice touche 0/1, l’UART est cassé.

## 6. Encodeurs
- 4 encodeurs rotatifs (A/B) + clic.
- Envois CC uniquement en cas de changement (throttling léger possible).
- Clic encodeur = entree UI (ouverture menu sur ecran gauche).

## 6.1 CC par encodeur (valeurs par defaut)
- Enc 1 : CC74 (CUTOFF)
- Enc 2 : CC73 (ATTACK)
- Enc 3 : CC91 (REVERB)
- Enc 4 : CC73 fixe pour l'instant (SYSTEM)

## 7. Pads batterie (mapping GM)
- KICK_PAD 36, SNARE_PAD 38, HAT_CLOSED_PAD 42, HAT_OPEN_PAD 46, CLAP_PAD 39.
- CRASH_PAD 49, RIDE_PAD 51, LOW_TOM_PAD 45, MID_TOM_PAD 47, HIGH_TOM_PAD 50.
- FLOOR_TOM_PAD 41, TAMB_PAD 54, RIMSHOT_PAD 37.
- Notes additionnelles en matrice : 55 (SPLASH), 57 (CRASH 2).

## 7.1 Pads utilises dans la matrice
- R0: KICK, SNARE, HAT_CLOSED
- R1: HAT_OPEN, CLAP, CRASH, RIDE
- R2: LOW_TOM, MID_TOM, HIGH_TOM, FLOOR_TOM, TAMB, RIMSHOT

## 8. Joystick analogique (axes X/Y)
- Axe X : `A15` (pin 39).
- Axe Y : `A14` (pin 38).
- Bouton SEL : pin 37 (entree digitale avec pullup interne).
- Usage : animation yeux + MIDI (Pitch Bend sur X, CC74 sur Y).
- Regle : zone morte autour du centre (~80).

## 8.1 Joystick et animation
- Animation yeux : pupils suivent le joystick, avec clignements periodiques.
- Reactions aux notes : les yeux regardent la derniere touche jouee ~350 ms.
- Canal MIDI : Pitch Bend (14 bits) + CC74 sur canal 1.

## 9. UI / Écrans (organisation définitive V1.1)
- 2 écrans OLED actifs :
  - Écran gauche : menus + popups encodeurs (mode TEST uniquement).
  - Écran droit : yeux en mode IDLE, debug touches/joystick en mode TEST.
- OLED1/OLED2 sont désactivés (non initialisés) en V1.1.
- Regle : les menus restent sur l'ecran gauche, les yeux sur l'ecran droit.

## 10. Fenêtre CONTROL (écran gauche)
- Ouverture via clic court sur un bouton d’encodeur.
- Affichage d’une liste d’actions par encodeur.
- Navigation :
  - Rotation de l’encodeur concerné : déplacement dans la liste.
  - Clic : validation de l’action sélectionnée.
- Encodeur 4 : entree "MODE TEST".
- Mode TEST : entree via menu, sortie via appui long (>= 1s) sur n'importe quel encodeur.

## 10.1 Menus encodeurs (contenu)
- Enc 1 (Timbre) : CUTOFF, RESONANCE, MOD WHEEL, BREATH.
- Enc 2 (Envelope) : ATTACK, DECAY, SUSTAIN, RELEASE.
- Enc 3 (FX/Mix) : REVERB, CHORUS, VOLUME, PAN.
- Enc 4 (System) : RETOUR, MODE TEST.

## 10.2 Mode TEST (debug)
- Affiche la matrice (row/col/pins, code MIDI, type NOTE/CC, canal).
- Affiche les popups encodeurs (enc 1..4) uniquement en mode test.
- Affiche les valeurs joystick et le bouton SEL sur l'ecran droit.

## 11. Regles de rendu UI (performance)
- Aucun redraw écran ni Serial.print dans la boucle de scan matrice/encodeurs.
- Redraws limités à ~20–30 FPS (cadence UI), via dirty flags.
- Chaque écran a sa fonction de rendu dédiée ; un seul point de vérité.

## 12. Conclusion
- Ce document est la référence officielle actuelle (V1.1, 2 écrans actifs).
- Toute modification future doit être explicitement documentée ici.
- L’architecture (câblage, MIDI, matrice, UI) est figée selon ces règles ; le retour d’écrans supplémentaires devra être spécifié (coprocesseur ou bus séparé).
