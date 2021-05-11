# SysAlarm
SysAlarm est un système d'alarme conçu pour les microcontrôleurs Arduino en ayant comme objectif la sécurité et la simplicité

## Fonctionnement
Pour armer le système, il suffit d'entrer un des codes valides sur le clavier numérique, le nom associé au code
apparaitra aussi sur le LCD. Lorsqu'armé le système déclenche l'alarme en cas d'intrusion ou dans le cas de l'ouverture
de la porte d'entrée un délai configurable donne le temps de désarmer le système.

Quand le système est désarmé, on peut utiliser la touche **#** pour recommencer la saisie du code et __*__ pour
lancer le menu de configuration. Noter que le menu ne peut être accédé que si le système est __désarmé__ et que
celui-ci mettra le reste du système en attente par précaution.

## Fonctionnalités
* Délai permettant de sortir avant l'armement du système 
* 5 codes à 6 chiffres configurables
* 5 noms d'utilisateur configurables de 1 à 8 caractères
* Un écran LCD et des témoins lumineux pour indiquer l'état du système
* Un menu de configuration sur le port série permettant de changer les codes et leur utilisateur associé
**tout changement exigent une authentification**
* Une configuration enregistrée dans l'EEPROM de l'Arduino

## Requis au fonctionnement
* Arduino
* Librairie Arduino [Keypad](https://playground.arduino.cc/Code/Keypad/#Download "Page de téléchargement")
pour le clavier numérique
* Clavier numérique
* 1 à 4 interrupteurs
* DELs
* "Buzzer"
* LCD 16x2
