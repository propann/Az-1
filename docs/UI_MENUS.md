# AZ-1 / LMN-3 — UI & Menus (référence 2 écrans)

## 1. Vue d’ensemble de l’UI
- Interface matérielle sans OS, pilotée par une matrice de boutons, 4 encodeurs rotatifs avec clic, et affichage sur 2 écrans OLED indépendants.
- Objectifs : lisibilité, réactivité, absence de surcharge visuelle.

## 2. Architecture générale de l’UI
- UI pilotée par un état central (state machine). Un seul point de décision pour la page active, le focus utilisateur, et la sélection courante.
- Les écrans ne décident pas seuls de leur contenu : le rendu est orchestré par le state machine.
- Notion de dirty flag : seuls les écrans marqués dirty sont redraw, avec cadence limitée.

## 3. Rôles figés des écrans (version 2 écrans)
- Écran 3 — VALUES : dernière note jouée, dernier CC modifié, transposition/octave, feedback utilisateur immédiat.
- Écran 4 — CONTROL WINDOW : écran principal de contrôle, menus, navigation, validation. Aucun autre écran n’affiche de menu.

## 4. Fenêtre CONTROL (écran 4)
- Cockpit principal : seule surface qui affiche des listes, propose des actions et reçoit le focus UI.
- États possibles : HOME (pas de menu ouvert), MENU (liste d’actions affichée), SUBMENU (réservé futur).

## 5. Ouverture des menus (clic encodeur)
- Clic sur ENC1/ENC2/ENC3/ENC4 :
  - ouvre un menu sur l’écran 4,
  - associe le menu à l’encodeur cliqué (focus UI),
  - sélection initiale (menuIndex) à 0,
  - titre affiché de type « ENC1 MENU », etc.

## 6. Navigation dans un menu
- Rotation de l’encodeur associé : déplace la sélection dans la liste (scroll si liste plus longue).
- Clic encodeur : valide l’action sélectionnée.
- Bouton CONTROL : BACK/CLOSE, ferme le menu et revient à HOME.

## 7. Structure des menus (future-proof)
- Menus = listes d’actions (label lisible + fonction associée).
- Prévoir le défilement pour les listes plus longues que l’écran.
- La CONTROL WINDOW est la seule surface où ces listes sont rendues.

## 8. Interaction UI / Hardware
- Les actions de menu peuvent déclencher des envois (CC, états internes, modes, transposition).
- Ce document décrit le comportement UI et la navigation, pas le détail MIDI.

## 9. Règles de performance UI
- Aucun redraw dans les boucles de scan matrice/encodeurs ; aucun Serial.print dans ces chemins.
- Rendu UI limité à ~20–30 FPS, déclenché par dirty flags ou tick dédié.

## 10. Règles d’évolution
- Toute nouvelle page UI doit être ajoutée ici et respecter la séparation stricte : VALUES (OLED3) pour le feedback d’événements, CONTROL (OLED4) pour les menus.
- Aucune nouvelle interaction ne doit contourner la CONTROL WINDOW pour les menus et actions, et aucun menu ne doit être rendu sur OLED3.
