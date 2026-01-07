# AZ-1 / LMN-3 — Roadmap V2 (officielle)

## 1) État actuel figé (V1.1 — référence)
- MIDI standard sur UART (Serial1), canal unique : CANAL 1.
- Matrice rapide avec edge detection (OFF->ON/ON->OFF, aucune répétition, placeholders ignorés).
- 4 encodeurs + clic encodeur.
- 2 écrans OLED actifs (VALUES = OLED3, CONTROL = OLED4) ; fenêtre CONTROL unique sur OLED4. OLED1/OLED2 sont désactivés en V1.1.
- UI non bloquante (priorité matrice > MIDI > UI).
- Règle : toute évolution V2 ne doit pas casser ces points.

## 2) Objectifs généraux V2
- Étendre les fonctionnalités et l’expressivité.
- Enrichir l’UI sans sacrifier la réactivité.
- Rester compatible MIDI standard.

## 3) Évolutions MIDI prévues (V2)
### 3.1 Multi-canal MIDI (optionnel)
- Possibilité future de gérer plusieurs canaux.
- Sélection du canal via menu CONTROL.
- Canal par défaut au boot : canal 1.
- Contraintes : pas de latence ajoutée, aucun message non documenté.

### 3.2 Messages MIDI supplémentaires (optionnels)
- Potentiels : Program Change, Pitch Bend par encodeur, CC avancés.
- Explicitement hors V2 immédiate : SysEx, MIDI Clock, NRPN complexes.

## 4) Évolutions UI prévues (V2)
### 4.1 Sous-menus
- SUBMENUS dans la CONTROL WINDOW (écran 4 uniquement), hiérarchie max 2 niveaux.

### 4.2 Pages UI supplémentaires
- Pages possibles : CONFIG (réglages globaux), INFO (version/build/debug), MAP (visualisation mapping).
- Règle : seul l’écran CONTROL (OLED4) affiche des menus/pages ; OLED3 reste VALUES.

## 5) Évolutions encodeurs (V2)
- Modes par encodeur (normal / shift via CONTROL).
- Affectation dynamique des CC via menu.
- Feedback visuel sur écran 4 et écran VALUES.

## 6) Évolutions matrice (V2)
- Banks de notes (changement d’octave / banque).
- Layers (note vs fonction).
- Règle absolue : edge detection conservée, aucune répétition automatique.

## 7) Évolutions écran / affichage (V2)
- Icônes simples, bargraphs CC, indicateurs de focus encodeur.
- Possibilité de réactiver des écrans additionnels via coprocesseur (ex : ESP32) ou bus séparé, sans pénaliser la matrice/MIDI.
- Aucun écran animé en continu.

## 8) Évolutions performance / debug (V2)
- Page PERFORMANCE enrichie (scan rate, charge UI).
- Mode debug activable/désactivable.

## 9) Hors-scope explicite
- OS embarqué, UI tactile.
- USB MIDI principal.
- MIDI over WiFi/Bluetooth.
- Animations lourdes, plugins dynamiques.

## 10) Règles de gouvernance V2
- Toute fonctionnalité V2 : documentée avant implémentation, respecte les priorités temps réel, passe la TEST_CHECKLIST.
- Toute rupture de compatibilité : signalée et optionnelle.

## 11) Conclusion
- V1 est stable et figée.
- V2 est évolutive mais contrôlée.
- La documentation fait foi ; le code suit la documentation.
