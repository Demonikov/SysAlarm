/*
 * Système d'alarme
 *      
 *      Crée le 26 avril 2021
 *      Autheur: Vincent Dupont
 */

#include <Keypad.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "LCDPorts.h"
#include "KeypadSetup.h"

/*
 * Temps cible : 1 sec -> 1 Hz
 * Diviseur      = 256
 * 16 MHz / 256  = 62500
 * 62500 / 1 Hz  = 62500
 * 65536 - 62500 = 3036
 */
#define TIMER1_VAL 3036
#define BUZZER 69
#define DOOR_DELAY 10 // sec

volatile int sec = 0; // Compteur de seconde
char cBuffer[6];
char users[5][9], cPasses[5][7];

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
LiquidCrystal lcd(RS, RW, ENABLE, DB0, DB1, DB2, DB3, DB4, DB5, DB6, DB7);

// ++++++++++++++++++++++++++++++++++++++++++++++++++
// +++ --- FONCTIONS DE TRAITEMENT DE DONNÉES --- +++
// ++++++++++++++++++++++++++++++++++++++++++++++++++

// Fonction qui retourne ce qui est reçu sur port série
String getSerial(String mesg = "\0")
{
    Serial.print(mesg);
    while (!Serial.available());
    String input = Serial.readStringUntil('\n');
    Serial.println(input);
    return input;
}

// Lis le keypad
void getCode(bool permitConfig = false)
{
    static int i;
    char key = keypad.getKey();
    if (key != NO_KEY) {
        tone(BUZZER, 698.46, 100);
        if (i == 0)
            memset(cBuffer, '\0', 6); // Efface le buffer
        switch (key) {
            case '*':
                if (!permitConfig) // Rends la touche inutile
                    return;
                i = 0;
                menuconfig();
                break;
            case '#':
                i = 0;
                break;
            default:
                cBuffer[i] = key;
                i = ++i % 6;
                break;
        }
    }
}

// Vérifie si le code entré est valide
bool checkCode(int &user)
{
    // Si le code n'a pas encore été entré, annuler la vérification
    if (cBuffer[5] == '\0')
        return false;

    // Rows = 5;  Cols = 6
    bool match; // Si 'true' après avoir vérifié une 'row' complète, le mot de passe est identique
    for (int row = 0; row < 5; row++) {
        match = true;

        for (int col = 0; col < 6 && match; col++) {
            if (cPasses[row][col] != cBuffer[col])
                match = false;
        }

        if (match) {
            user = row;
            memset(cBuffer, '\0', 6); // Efface le buffer par précaution
            return true;
        }
    }
    return false;
}

// +++++++++++++++++++++++++++++
// +++ --- FONCTIONS LCD --- +++
// +++++++++++++++++++++++++++++

// LCD - Stauts Armé / Désarmé
void Status(bool Arme, int userNum)
{
    lcd.clear();
    if (Arme)
        lcd.print("******ARME*******");
    else
        lcd.print("****DESARME*****");
    lcd.setCursor(0, 1);
    lcd.print("Par ");
    lcd.setCursor(4, 1);
    lcd.print(users[userNum]);
}

// LCD - Status lors d'intrusion
void Status(byte zone)
{
    lcd.clear();
    lcd.print("!! INTRUSION !!");
    lcd.setCursor(0, 1);
    switch (zone) {
        case B0001:
            lcd.print("Porte entree");
            break;
        case B0010:
            lcd.print("Porte-patio");
            break;
        case B0100:
            lcd.print("Fenetre chambre");
            break;
        default:
            lcd.print("Fenetre salon");
            break;
    };
}

// ++++++++++++++++++++++++++++++++++++++++++
// +++ --- FONCTIONS DE CONFIGURATION --- +++
// ++++++++++++++++++++++++++++++++++++++++++

// Modifie le nom d'un utilisateur
void editName()
{
    int userNum = getSerial("Nom a modifier: ").toInt();
    String newName = getSerial("Nouveau nom (8 caracteres max): ");
    if (newName.length() < 9 && newName.length()) { // Entre 1 et 8 carac.
        // Enregistre dans l'EEPROM & la RAM
        for (int n = 0; n < 8; n++) {
            EEPROM.write(userNum * 8 + n, newName[n]);
            users[userNum][n] = newName[n];
        }
    } else {
        Serial.println("Nom ou utilisateur invalide!");
    }
}

// Modifie le code d'un utilisateur
void editCode()
{
    int matchedUser;
    int userNum = getSerial("Code a modifier: ").toInt();

    // Authentification
    Serial.println("Entrez le code actuel sur le clavier numerique");
    while (cBuffer[5] == '\0')
        getCode();
    checkCode(matchedUser);

    if (matchedUser == userNum) {
        Serial.println("Entrez le nouveau code");
        while (cBuffer[5] == '\0')
            getCode();

        for (int c = 0; c < 6; c++) {
            EEPROM.write(40 + userNum * 6 + c, cBuffer[c]);
            cPasses[userNum][c] = cBuffer[c];
        }
    } else {
        Serial.println("Mauvais code!");
    }
}

// Menu de Configuration
void menuconfig()
{
    // Vide le buffer du port série
    while (Serial.available()) {
        Serial.read();
    }
    PORTB |= B1000;
    tone(BUZZER, 523.25, 100);
    int iBuffer = 0;

    while (iBuffer != 4) {
        iBuffer = getSerial((
                                "\n[1] Afficher les utilisateurs"
                                "\n[2] Modifier nom d'utilisateur"
                                "\n[3] Modifier code d'utilisateur"
                                "\n[4] Quitter\n"
                            )).toInt();

        // Liste des utilisateurs
        if (iBuffer > 0 && iBuffer < 4) {
            for (int i = 0; i < 5; i++) {
                Serial.print("[");
                Serial.print(i);
                Serial.print("] ");
                Serial.println(users[i]);
            }
        }
        if (iBuffer == 2)
            editName();
        else if (iBuffer == 3)
            editCode();
    }

    PORTB &= B0110;
    return;
}

// +++++++++++++++++++++
// +++ --- AUTRE --- +++
// +++++++++++++++++++++

// Vérifie si l'alarme doit être déclenché
void checkAlarm(bool &Alarm)
{
    static bool Pending;
    switch (PIND) {
        case B0000: // Aucune intrusion
            break;
        case B0001: // Par la porte!
            if (!Pending) {
                sec = DOOR_DELAY + 1;
                TIMSK1 |= (1 << TOIE1);
                Pending = true;
            }
            if (sec > 0) // ouverte depuis <10 sec
                return;
        default:    // Par ailleur!
            if (!Alarm) {
                Status(PIND);
                Alarm = true;
                Pending = false;
            }
            break;
    }
}

void leds(bool Arme)
{
    if (Arme)
        PORTB = B0010; // Rouge
    else
        PORTB = B0100; // Verte
}

void setup()
{
    Serial.begin(9600);

    // Lecture des noms et codes d'utilisateur dans l'EEPROM
    for (int s = 0; s < 5; s++) { // Cycle string
        for (int n = 0; n < 8; n++) // Cycle bits de nom
            users[s][n] = char(EEPROM.read(s * 8 + n));
        for (int c = 0; c < 6; c++) // Cycle bits de code
            cPasses[s][c] = char(EEPROM.read(40 + s * 6 + c));
    }

    DDRD = B0000; // Switches
    DDRB = B1111; // Dels
    PORTB = B0100;
    pinMode(BUZZER, OUTPUT);

    lcd.begin(16, 2);
    lcd.print("****DESARME*****");

    noInterrupts();
    TCCR1A = 0; // Compteur 16 Bit (65535)
    TCCR1B = 0;
    TCCR1B |= (1 << CS12); // Choisi diviseur programmable 256
    interrupts();
}

// 1 seconde est passé; décompte des secondes
ISR(TIMER1_OVF_vect)
{
    if (sec > 0) {
        TCNT1 = TIMER1_VAL;
        sec--;
    } else { // Décompte terminé, désactive les interruptions de débordement
        TIMSK1 &= ~_BV(TOIE1);
    }
}

void loop()
{
    static bool Arme, Alarm, Arming;
    static int iUser;

    getCode(!Arme);
    if (checkCode(iUser)) {
        if (!Arme && !Arming) { // 10 s pour sortir
            Arming = true;
            sec = DOOR_DELAY + 1; // +1 pour compenser le débordement instantané
            TIMSK1 |= (1 << TOIE1); // Active les interruptions de débordement
            lcd.clear();
            lcd.print("---- ARMAGE ----");
        } else { // Désarmage
            Alarm = Arme = Arming = false;
            Status(Arme, iUser);
        }
        tone(BUZZER, 554.25, 125); delay(150);
        tone(BUZZER, 493.88, 125); delay(150);
        tone(BUZZER, 554.25, 250); delay(300);
        tone(BUZZER, 369.99, 500);
    }
    leds(Arme);

    // Armage du système d'alarme
    if (!Arme && !sec && Arming) {
        Arme = true;
        Arming = false;
        Status(Arme, iUser);
    }
    if (Arme && !Alarm)
        checkAlarm(Alarm);
    if (Alarm)
        tone(BUZZER, millis() % 2000); // Plaisant
}
