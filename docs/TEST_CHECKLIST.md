# AZ-1 / LMN-3 — Checklist officielle de tests

## 1) Pré-vol (avant flash)
- [ ] Carte utilisée : Teensy 4.1 confirmée
- [ ] Firmware compilé sans warning critique
- [ ] Pins UART confirmées : RX = pin 0, TX = pin 1
- [ ] Colonnes matrice déplacées : COL_6 = 20, COL_7 = 21
- [ ] Pins 17 / 18 réservées écran (non utilisées ailleurs)
- [ ] GND commun présent entre Teensy et récepteur MIDI

## 2) Boot & démarrage
- [ ] Le firmware démarre sans freeze
- [ ] Aucun reset en boucle
- [ ] Les écrans actifs s’allument (OLED3 = VALUES, OLED4 = CONTROL)
- [ ] L’écran 4 affiche la page CONTROL / HOME
- [ ] Aucun écran ne spamme de redraw visible

## 3) Test matrice — Notes
- [ ] Appui sur une touche valide → 1 NOTE_ON
- [ ] Maintien de la touche → aucun message supplémentaire
- [ ] Relâchement → 1 NOTE_OFF
- [ ] Aucune note ne reste bloquée
- [ ] Les touches placeholder (note = 1) n’envoient rien
- [ ] Multi-appuis rapides → comportement stable

## 4) Test matrice — Boutons CC
- [ ] Appui bouton CC → 1 CC envoyé
- [ ] Relâchement → comportement attendu (selon mapping)
- [ ] Aucun CC en boucle pendant maintien
- [ ] Boutons spéciaux (UNDO, SAVE, etc.) fonctionnent

## 5) Test encodeurs
- [ ] Rotation encodeur → CC envoyé uniquement si changement
- [ ] Pas de spam excessif de CC
- [ ] Sens de rotation correct
- [ ] Clic encodeur détecté une seule fois par appui

## 6) Test Pitch Bend
- [ ] Mouvement lent → variation fluide
- [ ] Centre stable (≈ 8192)
- [ ] Aucun jitter excessif
- [ ] Aucun envoi hors plage MIDI

## 7) Test MIDI (son)
- [ ] Synthé / DAW reçoit sur CANAL 1
- [ ] NoteOn / NoteOff audibles
- [ ] CC agissent sur les paramètres attendus
- [ ] Aucun message MIDI parasite (SysEx, Clock, etc.)
- [ ] Baudrate confirmé à 31250

## 8) Test UI — écrans
- [ ] Écran 3 affiche VALUES uniquement
- [ ] Écran 4 affiche CONTROL uniquement
- [ ] Aucun menu n’apparaît sur l’écran 3

## 9) Test UI — menus (écran 4)
- [ ] Clic encodeur ouvre un menu sur écran 4
- [ ] Le menu correspond à l’encodeur cliqué
- [ ] Rotation encodeur navigue dans la liste
- [ ] Clic valide l’action
- [ ] Bouton CONTROL ferme le menu et revient à HOME

## 10) Test performance / réactivité
- [ ] Latence appui → son < 5 ms (ressenti)
- [ ] La matrice réagit instantanément
- [ ] Aucun lag causé par les écrans
- [ ] Aucun freeze lors d’appuis rapides multiples

## 11) Test régression
- [ ] Aucune fonctionnalité validée précédemment cassée
- [ ] Le comportement correspond à la documentation figée
- [ ] Aucun comportement non documenté observé

## 12) Validation finale
- [ ] Tous les tests sont cochés
- [ ] Firmware validé
- [ ] Documentation à jour si changement observé

### Règle finale
- Si une case ne peut pas être cochée : firmware NON validé. Corriger la cause AVANT toute évolution.
