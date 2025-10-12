#pragma once

// Würfel-Funktionen
int roll_d4();
int roll_d6();
int roll_d8();
int roll_d10();
int roll_d12();
int roll_d20();
int roll_d100();

// Münzwurf
bool flip_coin();

// Würfel-Namen für UI
struct DiceType {
    const char* name;
    int (*roll_func)();
};

extern const DiceType DICE_TYPES[];
extern const int DICE_TYPE_COUNT;
