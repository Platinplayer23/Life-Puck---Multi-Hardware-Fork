#include "tcg_presets.h"
#include "state/state_store.h"
#include <cstring>

TCGPreset TCG_PRESETS[10];
int TCG_PRESET_COUNT = 10;
int current_preset_index = 0;

void init_presets() {
    // MTG Standard
    strncpy(TCG_PRESETS[0].name, "MTG Standard", sizeof(TCG_PRESETS[0].name) - 1);
    TCG_PRESETS[0].starting_life = 20;
    TCG_PRESETS[0].small_step = 1;
    TCG_PRESETS[0].large_step = 5;

    // MTG Commander
    strncpy(TCG_PRESETS[1].name, "MTG Commander", sizeof(TCG_PRESETS[1].name) - 1);
    TCG_PRESETS[1].starting_life = 40;
    TCG_PRESETS[1].small_step = 1;
    TCG_PRESETS[1].large_step = 10;

    // Pokemon TCG
    strncpy(TCG_PRESETS[2].name, "Pokemon TCG", sizeof(TCG_PRESETS[2].name) - 1);
    TCG_PRESETS[2].starting_life = 60;
    TCG_PRESETS[2].small_step = 10;
    TCG_PRESETS[2].large_step = 30;

    // Yu-Gi-Oh!
    strncpy(TCG_PRESETS[3].name, "Yu-Gi-Oh!", sizeof(TCG_PRESETS[3].name) - 1);
    TCG_PRESETS[3].starting_life = 8000;
    TCG_PRESETS[3].small_step = 50;
    TCG_PRESETS[3].large_step = 500;
    
    // Custom 5-10
    for (int i = 4; i < 10; i++) {
        snprintf(TCG_PRESETS[i].name, sizeof(TCG_PRESETS[i].name), "Custom %d", i + 1);
        TCG_PRESETS[i].starting_life = 20;
        TCG_PRESETS[i].small_step = 1;
        TCG_PRESETS[i].large_step = 5;
    }
}

void load_preset() {
    current_preset_index = player_store.getInt("preset_idx", 0);
    if (current_preset_index < 0 || current_preset_index >= TCG_PRESET_COUNT) {
        current_preset_index = 0;
    }
}

void save_preset(int index) {
    if (index >= 0 && index < TCG_PRESET_COUNT) {
        current_preset_index = index;
        player_store.putInt("preset_idx", index);
    }
}

TCGPreset get_preset() {
    if (current_preset_index < 0 || current_preset_index >= TCG_PRESET_COUNT) {
        return TCG_PRESETS[0];
    }
    return TCG_PRESETS[current_preset_index];
}
