// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/event_grouper.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"


void init_life_counter();
void reset_life();
void clear_amp();
void toggle_amp_visibility();
void life_counter_loop();
void teardown_life_counter();

// *** PERSISTENT LIFE STORAGE ***
void saveLifeToNVS(int life_value, int player = 1);
int loadLifeFromNVS(int player = 1);
void clearSavedLife();

// Extern so we can access from main.cpp
extern EventGrouper event_grouper;