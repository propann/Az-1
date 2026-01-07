# AZ-1 / LMN-3 — Architecture et règles définitives (V1.1 = 2 écrans)

## 1. Présentation générale
- Projet : AZ-1 / LMN-3, contrôleur MIDI matériel basé sur Teensy 4.1.
- Périphériques : matrice de boutons/notes, 4 encodeurs rotatifs avec clic, 2 écrans OLED actifs (VALUES et CONTROL) en V1.1.
- Objectif : réactivité, stabilité, zéro doublon d’événements.
- Note : OLED1/OLED2 sont retirés en V1.1 pour réduire la charge I2C (seuls OLED3/OLED4 restent actifs).

## 2. Sortie MIDI (son)
- Protocole : MIDI standard uniquement.
- Canal MIDI unique : canal 1 (status bytes OR 0).
- Status bytes :
  - NoteOn : `0x90 | 0`
  - NoteOff : `0x80 | 0` (ou NoteOn vel=0)
  - CC : `0xB0 | 0`
  - Pitchbend : `0xE0 | 0` (14 bits LSB/MSB)
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
- Notes placeholder (valeur 1) ignorées (aucun envoi).
- Interdiction absolue d’utiliser/configurer les pins 0/1 dans la matrice.

## 5. Colonnes déplacées (conflit UART évité)
- Colonnes historiquement sur 0/1 déplacées sur 20/21.
- Mapping figé :
  - `COL_6 = 20`
  - `COL_7 = 21`
- Règle : si la matrice touche 0/1, l’UART est cassé.

## 6. Encodeurs
- 4 encodeurs rotatifs (A/B) + clic.
- Envois CC uniquement en cas de changement (throttling léger possible).
- Clic encodeur = entrée UI (ouverture menu CONTROL sur écran 4).

## 7. UI / Écrans (organisation définitive V1.1)
- 2 écrans OLED actifs :
  - Écran 3 : VALUES (feedback immédiat : dernière note/CC, transposition)
  - Écran 4 : CONTROL (fenêtre principale, menus uniquement)
- OLED1/OLED2 sont désactivés (non initialisés) en V1.1.
- Interdiction d’afficher des menus sur OLED3 (VALUES) ; CONTROL est le seul écran de menus.

## 8. Fenêtre CONTROL (écran 4)
- Ouverture via clic sur un bouton d’encodeur.
- Affichage d’une liste d’actions par encodeur (structures de menu prêtes).
- Navigation :
  - Rotation de l’encodeur concerné : déplacement dans la liste.
  - Clic : validation de l’action sélectionnée.
  - Bouton CONTROL : retour / fermeture du menu.
- Structure prête pour extensions futures (nouvelles actions).

## 9. Règles de rendu UI (performance)
- Aucun redraw écran ni Serial.print dans la boucle de scan matrice/encodeurs.
- Redraws limités à ~20–30 FPS (cadence UI), via dirty flags.
- Chaque écran a sa fonction de rendu dédiée ; un seul point de vérité.

## 10. Conclusion
- Ce document est la référence officielle actuelle (V1.1, 2 écrans actifs).
- Toute modification future doit être explicitement documentée ici.
- L’architecture (câblage, MIDI, matrice, UI) est figée selon ces règles ; le retour d’écrans supplémentaires devra être spécifié (coprocesseur ou bus séparé).
