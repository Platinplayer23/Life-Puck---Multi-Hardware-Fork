#include "dice_coin.h"
#include <esp_random.h>

int roll_d4() { 
    return (esp_random() % 4) + 1; 
}

int roll_d6() { 
    return (esp_random() % 6) + 1; 
}

int roll_d8() { 
    return (esp_random() % 8) + 1; 
}

int roll_d10() { 
    return (esp_random() % 10) + 1; 
}

int roll_d12() { 
    return (esp_random() % 12) + 1; 
}

int roll_d20() { 
    return (esp_random() % 20) + 1; 
}

int roll_d100() { 
    return (esp_random() % 100) + 1; 
}

bool flip_coin() { 
    return (esp_random() % 2) == 0; 
}

// Würfel-Liste für UI
const DiceType DICE_TYPES[] = {
    {"W4", roll_d4},
    {"W6", roll_d6},
    {"W8", roll_d8},
    {"W10", roll_d10},
    {"W12", roll_d12},
    {"W20", roll_d20},
    {"W100", roll_d100}
};

const int DICE_TYPE_COUNT = 7;
