# AZ-1 / LMN-3 — Guide officiel de debug

## 1) Symptôme : PAS DE SON
Causes possibles (ordre) :
- [ ] Le récepteur MIDI écoute sur CANAL 1
- [ ] Le baudrate UART est 31250
- [ ] Le port utilisé est Serial1
- [ ] TX/RX bien inversés (TX->RX, RX->TX)
- [ ] GND commun présent
- [ ] Aucun message USB MIDI actif
- [ ] Messages émis = MIDI standard (0x90/0x80/0xB0/0xE0)
- [ ] La matrice ne configure PAS les pins 0/1
- [ ] Notes placeholder (valeur=1) ignorées

Actions correctives :
- Vérifier câblage TX/RX
- Vérifier colonnes (COL_6=20, COL_7=21)
- Vérifier canal MIDI côté synthé/DAW

## 2) Symptôme : NOTES BLOQUÉES
Causes possibles :
- [ ] Absence de NoteOff
- [ ] Edge detection mal implémentée
- [ ] NoteOn vel=0 mal interprété
- [ ] Double envoi NoteOn sans NoteOff
- [ ] Scan matrice interrompu par UI

Actions correctives :
- Vérifier OFF->ON / ON->OFF
- Forcer NoteOff explicite (0x80)
- Vérifier que le scan matrice est prioritaire
- Désactiver temporairement l’UI pour test

## 3) Symptôme : MATRICE LENTE / NON RÉACTIVE
Causes possibles :
- [ ] Redraw écran dans la boucle de scan
- [ ] Serial.print dans la boucle de scan
- [ ] delay()/delayMicroseconds trop élevés
- [ ] Scan non prioritaire
- [ ] UI bloquante

Actions correctives :
- Supprimer toute opération écran dans le scan
- Limiter delayMicroseconds à ≤ 5 µs
- Scanner la matrice à ~1 kHz
- Déplacer l’UI dans un tick séparé (20–30 FPS)

## 4) Symptôme : LATENCE ÉLEVÉE
Causes possibles :
- [ ] Debounce trop long (>10 ms)
- [ ] Encodeurs qui spamment les CC
- [ ] UI redraw trop fréquent
- [ ] Envoi MIDI bloquant

Actions correctives :
- Réduire debounce à 2–5 ms
- Throttler les CC encodeurs (2–5 ms)
- Limiter le rendu UI
- Vérifier que l’émission MIDI est non bloquante

## 5) Symptôme : AUCUN MESSAGE MIDI
Causes possibles :
- [ ] Baud incorrect
- [ ] Mauvais port série
- [ ] Pins UART réutilisées ailleurs
- [ ] Serial.begin non appelé
- [ ] Boucle principale bloquée

Actions correctives :
- Vérifier Serial1.begin(31250)
- Vérifier que 0/1 ne sont jamais configurées ailleurs
- Tester avec un analyseur MIDI ou moniteur DAW

## 6) Symptôme : ÉCRAN MUET (OLED3 ou OLED4)
Causes possibles :
- [ ] Mauvaise adresse I2C
- [ ] Adresse 7-bit / 8-bit confondue
- [ ] Pins I2C incorrectes
- [ ] Écran non alimenté
- [ ] Bus I2C saturé par redraws excessifs

Actions correctives :
- Scanner le bus I2C
- Vérifier adresses courantes (0x3C / 0x3D)
- Vérifier alimentation
- Limiter les sendBuffer()

## 7) Symptôme : LES 2 ÉCRANS AFFICHENT LA MÊME CHOSE
Causes possibles :
- [ ] Même fonction de rendu appelée pour tous
- [ ] Absence de séparation des rôles
- [ ] UI non routée

Actions correctives :
- Vérifier mapping écrans (VALUES / CONTROL)
- Vérifier que seuls les menus vont sur écran 4
- Implémenter un routeur UI central

## 8) Symptôme : MENUS CHAOTIQUES
Causes possibles :
- [ ] Focus UI mal géré
- [ ] Plusieurs encodeurs actifs en même temps
- [ ] Dirty flags absents

Actions correctives :
- Forcer un seul focus encodeur
- Initialiser menuIndex à l’ouverture
- Redraw uniquement sur changement

## 9) Procédure de debug recommandée (ordre strict)
1) Désactiver UI et écrans
2) Tester MIDI seul (notes simples)
3) Tester matrice seule
4) Tester encodeurs
5) Réactiver UI progressivement
6) Tester menus

## 10) Règle finale
- Ne jamais corriger “au hasard”.
- Toujours identifier le symptôme.
- Toujours vérifier la documentation figée.
- Toute correction doit préserver : MIDI canal 1, priorité matrice, séparation UI / scan.
