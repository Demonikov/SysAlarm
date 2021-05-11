#ifndef KEYPADSETUP_H_
#define KEYPADSETUP_H_

#define ROWS 4
#define COLS 3

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'},
};

byte rowPins[ROWS] = {62, 63, 64, 65};
byte colPins[COLS] = {66, 67, 68};

#endif
