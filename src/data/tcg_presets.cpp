#include "tcg_presets.h"
#include "core/state_manager.h"
#include "ui/screens/menu/menu.h"
#include <cstring>

TCGPreset TCG_PRESETS[10];
int TCG_PRESET_COUNT = 10;
int current_preset_index = 0;

// ============================================
// DEFAULT PRESETS - Only on FIRST startup
// ============================================
/**
 * @brief Initialize default TCG game presets
 * 
 * Sets up the default configurations for popular trading card games
 * including Magic: The Gathering, Pokemon, Yu-Gi-Oh!, and others.
 * These serve as factory defaults and fallback values.
 */
void init_default_presets() {
    // ---- SLOT 0: MTG Standard ----
    strncpy(TCG_PRESETS[0].name, "MTG Standard", sizeof(TCG_PRESETS[0].name) - 1);
    TCG_PRESETS[0].starting_life = 20;
    TCG_PRESETS[0].small_step = 1;
    TCG_PRESETS[0].large_step = 5;

    // ---- SLOT 1: MTG Commander ----
    strncpy(TCG_PRESETS[1].name, "MTG Commander", sizeof(TCG_PRESETS[1].name) - 1);
    TCG_PRESETS[1].starting_life = 40;
    TCG_PRESETS[1].small_step = 1;
    TCG_PRESETS[1].large_step = 10;

    // ---- SLOT 2: Pokemon TCG ----
    strncpy(TCG_PRESETS[2].name, "Pokemon TCG", sizeof(TCG_PRESETS[2].name) - 1);
    TCG_PRESETS[2].starting_life = 60;
    TCG_PRESETS[2].small_step = 10;
    TCG_PRESETS[2].large_step = 30;

    // ---- SLOT 3: Yu-Gi-Oh! ----
    strncpy(TCG_PRESETS[3].name, "Yu-Gi-Oh!", sizeof(TCG_PRESETS[3].name) - 1);
    TCG_PRESETS[3].starting_life = 8000;
    TCG_PRESETS[3].small_step = 50;
    TCG_PRESETS[3].large_step = 500;
    
    // ---- SLOT 4: Flesh and Blood ----
    strncpy(TCG_PRESETS[4].name, "Flesh & Blood", sizeof(TCG_PRESETS[4].name) - 1);
    TCG_PRESETS[4].starting_life = 40;
    TCG_PRESETS[4].small_step = 1;
    TCG_PRESETS[4].large_step = 5;

    // ---- SLOT 5: Lorcana ----
    strncpy(TCG_PRESETS[5].name, "Lorcana", sizeof(TCG_PRESETS[5].name) - 1);
    TCG_PRESETS[5].starting_life = 20;
    TCG_PRESETS[5].small_step = 1;
    TCG_PRESETS[5].large_step = 5;

    // ---- SLOT 6: One Piece TCG ----
    strncpy(TCG_PRESETS[6].name, "One Piece TCG", sizeof(TCG_PRESETS[6].name) - 1);
    TCG_PRESETS[6].starting_life = 5;
    TCG_PRESETS[6].small_step = 1;
    TCG_PRESETS[6].large_step = 2;

    // ---- SLOT 7: Custom ----
    strncpy(TCG_PRESETS[7].name, "Custom 8", sizeof(TCG_PRESETS[7].name) - 1);
    TCG_PRESETS[7].starting_life = 20;
    TCG_PRESETS[7].small_step = 1;
    TCG_PRESETS[7].large_step = 5;

    // ---- SLOT 8: Custom ----
    strncpy(TCG_PRESETS[8].name, "Custom 9", sizeof(TCG_PRESETS[8].name) - 1);
    TCG_PRESETS[8].starting_life = 20;
    TCG_PRESETS[8].small_step = 1;
    TCG_PRESETS[8].large_step = 5;

    // ---- SLOT 9: Custom ----
    strncpy(TCG_PRESETS[9].name, "Custom 10", sizeof(TCG_PRESETS[9].name) - 1);
    TCG_PRESETS[9].starting_life = 20;
    TCG_PRESETS[9].small_step = 1;
    TCG_PRESETS[9].large_step = 5;
}

// ============================================
// INIT: Load from storage OR use defaults
// ============================================
/**
 * @brief Initialize preset system
 * 
 * Loads presets from persistent storage if available, otherwise
 * initializes with factory defaults. This is called during system startup.
 */
void init_presets() {
    printf("[Presets] Starting init_presets()\n");
    
    // First load defaults (fallback)
    printf("[Presets] Calling init_default_presets()\n");
    init_default_presets();
    printf("[Presets] init_default_presets() completed\n");
    
    // Then try to overwrite from storage
    printf("[Presets] Starting storage loop\n");
    for (int i = 0; i < 10; i++) {
        printf("[Presets] Processing preset %d\n", i);
        char key_name[32], key_life[32], key_small[32], key_large[32];
        
        snprintf(key_name, sizeof(key_name), "preset_%d_name", i);
        snprintf(key_life, sizeof(key_life), "preset_%d_life", i);
        snprintf(key_small, sizeof(key_small), "preset_%d_small", i);
        snprintf(key_large, sizeof(key_large), "preset_%d_large", i);
        
        printf("[Presets] Getting stored name for preset %d\n", i);
        String stored_name = player_store.getString(key_name, "");
        printf("[Presets] Stored name: '%s'\n", stored_name.c_str());
        
        if (stored_name.length() > 0) {
            printf("[Presets] Stored preset found for %d, overwriting default\n", i);
            // Stored preset found - overwrite default
            strncpy(TCG_PRESETS[i].name, stored_name.c_str(), sizeof(TCG_PRESETS[i].name) - 1);
            TCG_PRESETS[i].name[sizeof(TCG_PRESETS[i].name) - 1] = '\0';
            
            printf("[Presets] Getting stored values for preset %d\n", i);
            TCG_PRESETS[i].starting_life = player_store.getInt(key_life, TCG_PRESETS[i].starting_life);
            TCG_PRESETS[i].small_step = player_store.getInt(key_small, TCG_PRESETS[i].small_step);
            TCG_PRESETS[i].large_step = player_store.getInt(key_large, TCG_PRESETS[i].large_step);
            printf("[Presets] Preset %d loaded: %s, life=%d, small=%d, large=%d\n", 
                   i, TCG_PRESETS[i].name, TCG_PRESETS[i].starting_life, 
                   TCG_PRESETS[i].small_step, TCG_PRESETS[i].large_step);
        } else {
            printf("[Presets] No stored preset for %d, keeping default\n", i);
        }
        // If NOT stored, keep the default from init_default_presets()
    }
    printf("[Presets] init_presets() completed successfully\n");
}

// ============================================
// PRESET ACCESS FUNCTIONS - DO NOT MODIFY
// ============================================
/**
 * @brief Load active preset index from storage
 * 
 * Retrieves the currently selected preset index from persistent storage.
 * Defaults to preset 0 if no valid index is found.
 */
void load_preset() {
    printf("[Presets] Starting load_preset()\n");
    printf("[Presets] Getting preset_idx from player_store\n");
    current_preset_index = player_store.getInt("preset_idx", 0);
    printf("[Presets] Got preset_idx: %d\n", current_preset_index);
    
    if (current_preset_index < 0 || current_preset_index >= TCG_PRESET_COUNT) {
        printf("[Presets] Invalid preset_idx %d, resetting to 0\n", current_preset_index);
        current_preset_index = 0;
    }
    printf("[Presets] Final preset_idx: %d\n", current_preset_index);
    printf("[Presets] load_preset() completed successfully\n");
}

/**
 * @brief Save active preset index to storage
 * 
 * @param index Index of the preset to make active (0-9)
 * 
 * Stores the preset index in persistent storage if valid.
 */
void save_preset(int index) {
    if (index >= 0 && index < TCG_PRESET_COUNT) {
        current_preset_index = index;
        player_store.putInt("preset_idx", index);
        
        // Reset life points to preset values
        resetActiveCounter();
        
        printf("[Preset] Applied preset %s with %d starting life\n", TCG_PRESETS[index].name, TCG_PRESETS[index].starting_life);
    }
}

/**
 * @brief Get the currently active preset
 * 
 * @return TCGPreset The current preset configuration
 * 
 * Returns the preset at the current index, or preset 0 as fallback.
 */
TCGPreset get_preset() {
    printf("[Presets] Starting get_preset()\n");
    printf("[Presets] current_preset_index: %d, TCG_PRESET_COUNT: %d\n", current_preset_index, TCG_PRESET_COUNT);
    
    if (current_preset_index < 0 || current_preset_index >= TCG_PRESET_COUNT) {
        printf("[Presets] Invalid index, returning TCG_PRESETS[0]\n");
        printf("[Presets] TCG_PRESETS[0] address: %p\n", &TCG_PRESETS[0]);
        return TCG_PRESETS[0];
    }
    
    printf("[Presets] Returning TCG_PRESETS[%d]\n", current_preset_index);
    printf("[Presets] TCG_PRESETS[%d] address: %p\n", current_preset_index, &TCG_PRESETS[current_preset_index]);
    printf("[Presets] TCG_PRESETS[%d] name: '%s'\n", current_preset_index, TCG_PRESETS[current_preset_index].name);
    printf("[Presets] TCG_PRESETS[%d] starting_life: %d\n", current_preset_index, TCG_PRESETS[current_preset_index].starting_life);
    printf("[Presets] TCG_PRESETS[%d] small_step: %d\n", current_preset_index, TCG_PRESETS[current_preset_index].small_step);
    printf("[Presets] TCG_PRESETS[%d] large_step: %d\n", current_preset_index, TCG_PRESETS[current_preset_index].large_step);
    
    TCGPreset result = TCG_PRESETS[current_preset_index];
    printf("[Presets] Copied preset - name: '%s', starting_life: %d\n", result.name, result.starting_life);
    return result;
}
