#include "tcg_presets.h"
#include "core/state_manager.h"
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
    // First load defaults (fallback)
    init_default_presets();
    
    // Then try to overwrite from storage
    for (int i = 0; i < 10; i++) {
        char key_name[32], key_life[32], key_small[32], key_large[32];
        
        snprintf(key_name, sizeof(key_name), "preset_%d_name", i);
        snprintf(key_life, sizeof(key_life), "preset_%d_life", i);
        snprintf(key_small, sizeof(key_small), "preset_%d_small", i);
        snprintf(key_large, sizeof(key_large), "preset_%d_large", i);
        
        String stored_name = player_store.getString(key_name, "");
        
        if (stored_name.length() > 0) {
            // Stored preset found - overwrite default
            strncpy(TCG_PRESETS[i].name, stored_name.c_str(), sizeof(TCG_PRESETS[i].name) - 1);
            TCG_PRESETS[i].name[sizeof(TCG_PRESETS[i].name) - 1] = '\0';
            
            TCG_PRESETS[i].starting_life = player_store.getInt(key_life, TCG_PRESETS[i].starting_life);
            TCG_PRESETS[i].small_step = player_store.getInt(key_small, TCG_PRESETS[i].small_step);
            TCG_PRESETS[i].large_step = player_store.getInt(key_large, TCG_PRESETS[i].large_step);
        }
        // If NOT stored, keep the default from init_default_presets()
    }
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
    current_preset_index = player_store.getInt("preset_idx", 0);
    if (current_preset_index < 0 || current_preset_index >= TCG_PRESET_COUNT) {
        current_preset_index = 0;
    }
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
    if (current_preset_index < 0 || current_preset_index >= TCG_PRESET_COUNT) {
        return TCG_PRESETS[0];
    }
    return TCG_PRESETS[current_preset_index];
}
