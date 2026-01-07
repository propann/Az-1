# AZ-1 / LMN-3 — Protocole MIDI et Timing (référence)

## 1. Vue d’ensemble MIDI
- Firmware AZ-1 / LMN-3 : MIDI STANDARD uniquement, aucune extension propriétaire.
- Compatibilité attendue : synthés matériels, DAW, modules MIDI classiques.
- Objectif : latence minimale, comportement déterministe, aucun message parasite.

## 2. Transport MIDI
- Transport : UART matériel.
- Port : Serial1.
- Pins : RX = 0, TX = 1 (TX -> RX, RX -> TX, GND commun obligatoire).
- Baudrate : 31250 bauds (standard MIDI). Toute autre vitesse invalide la compatibilité.

## 3. Canal MIDI
- Canal utilisé : CANAL 1 (humain).
- Représentation brute : channel = 0 (0x0).
- Tous les messages “sonores” utilisent ce canal. Pas de multi-canal implémenté.

## 4. Messages MIDI émis (format figé)

### 4.1 Notes
- Note On : Status `0x90 | channel`, Data1 = note (0–127), Data2 = velocity (1–127).
- Note Off : Status `0x80 | channel`, Data1 = note (0–127), Data2 = 0.
- Alternative acceptée : Note On avec velocity = 0.

### 4.2 Control Change (CC)
- Status : `0xB0 | channel`.
- Data1 : CC number (0–127).
- Data2 : CC value (0–127).
- CC envoyés uniquement si la valeur change.

### 4.3 Pitch Bend
- Status : `0xE0 | channel`.
- Data1 : LSB (7 bits).
- Data2 : MSB (7 bits).
- Résolution : 14 bits (0–16383).
- Centre : 8192 (0x2000).

## 5. Messages explicitement interdits
- SysEx, Active Sensing, MIDI Clock, MIDI Start/Stop/Continue.
- Aftertouch (Channel/Poly), NRPN/RPN.
- Ces messages ne doivent pas être émis sans mise à jour explicite de cette documentation.

## 6. Règles de timing — Notes
- Par touche : exactement 1 Note On à l’appui, 1 Note Off au relâchement.
- Aucune répétition pendant maintien.
- Aucun double envoi autorisé.
- Notes placeholder (valeur = 1) ignorées (aucun envoi).

## 7. Règles de timing — CC / Encodeurs
- CC envoyés uniquement si la valeur change.
- Throttling autorisé pour limiter le spam : intervalle minimal recommandé 2 à 5 ms.
- Aucun envoi périodique inutile.

## 8. Latence et réactivité (objectifs)
- Latence cible appui → MIDI : < 5 ms.
- Scan matrice recommandé : ~1 kHz.
- Rendu UI ne doit jamais bloquer l’émission MIDI.

## 9. Priorités temps réel (ordre strict)
1) Scan matrice (notes / boutons)
2) Émission MIDI
3) Scan encodeurs
4) Scan analog (pitchbend)
5) UI / écrans

Toute inversion de priorité est interdite.

## 10. Gestion des erreurs
- Si l’UART est indisponible : ne pas buffer indéfiniment, ne pas bloquer la boucle principale.
- Aucun retry actif. Le firmware reste silencieux plutôt que d’envoyer des messages corrompus.

## 11. Règles d’évolution du protocole
- Toute modification MIDI doit être documentée ici et préciser l’impact compatibilité.
- Évolutions possibles (multi-canal, messages avancés) uniquement après mise à jour de ce document.
