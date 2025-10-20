// ============================================================================
// LIFEPUCK ESP32-S3 CONFIGURATION FILE
// ============================================================================
// 
// SINGLE SOURCE OF TRUTH for all user-configurable settings
// ALL values defined here are used exclusively by the code
// Change values here and recompile to apply
//
// Hardware-specific values (pins, SPI, I2C) remain in board_config.h
//
// ============================================================================

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// ⚠️  FORCE RESET FLAGS - FOR TESTING AND DEVELOPMENT
// ============================================================================
// WARNING: These flags OVERRIDE all saved user settings!
// ⚠️  IMPORTANT: Set back to false for production releases!
// ============================================================================

#define FORCE_DEFAULT_CALIBRATION false  // Force touch calibration to defaults
#define FORCE_DEFAULT_PRESETS false      // Force presets to factory defaults
#define FORCE_FACTORY_RESET false        // Reset ALL settings to defaults
#define FORCE_NVS_CLEAR false            // Erase ALL saved data (nuclear option)
#define SKIP_INITIAL_CALIBRATION false   // Skip calibration screen on first boot


// ============================================================================
// 🎨 THEME CONFIGURATION
// ============================================================================
// Theme colors, opacity, and visual appearance
// Used by: src/data/themes.cpp
// ============================================================================

// Theme Background Opacity (0-255: 0=transparent, 255=opaque)
#define THEME_OPACITY_DEFAULT 180
#define THEME_OPACITY_YUGIOH 180
#define THEME_OPACITY_MAGIC 180
#define THEME_OPACITY_POKEMON 180

// Theme Button Colors (RGB hex: 0xRRGGBB)
#define THEME_COLOR_DEFAULT 0x0070FF   // Lightning Blue
#define THEME_COLOR_YUGIOH 0xFFD700    // Gold
#define THEME_COLOR_MAGIC 0x9B30FF     // Purple
#define THEME_COLOR_POKEMON 0xDC0A2D   // Red

// Theme Icon Colors
#define THEME_ICON_COLOR_DEFAULT 0xFFFFFF  // White
#define THEME_ICON_COLOR_YUGIOH 0xFFD700   // Gold
#define THEME_ICON_COLOR_MAGIC 0x9B30FF    // Purple
#define THEME_ICON_COLOR_POKEMON 0xDC0A2D  // Red

// Theme Text Colors (ensures proper contrast)
#define THEME_TEXT_COLOR_DEFAULT 0xFFFFFF  // White
#define THEME_TEXT_COLOR_YUGIOH 0x000000   // Black
#define THEME_TEXT_COLOR_MAGIC 0xFFFFFF    // White
#define THEME_TEXT_COLOR_POKEMON 0xFFFFFF  // White

// Back Button Text Color
#define BACK_BUTTON_TEXT_COLOR 0x000000    // Black


// ============================================================================
// 🎮 PRESET DEFINITIONS
// ============================================================================
// TCG game presets: name, starting life, steps, theme assignment
// Used by: src/data/tcg_presets.cpp
// Theme values: 0=Off, 1=Yugioh, 2=Magic, 3=Pokemon, 4=Default
// ============================================================================

// PRESET 0: MTG Standard
#define PRESET_0_NAME "MTG Standard"
#define PRESET_0_STARTING_LIFE 20
#define PRESET_0_SMALL_STEP 1
#define PRESET_0_LARGE_STEP 5
#define PRESET_0_THEME 2                 // Magic

// PRESET 1: MTG Commander
#define PRESET_1_NAME "MTG Commander"
#define PRESET_1_STARTING_LIFE 40
#define PRESET_1_SMALL_STEP 1
#define PRESET_1_LARGE_STEP 10
#define PRESET_1_THEME 2                 // Magic

// PRESET 2: Pokemon TCG
#define PRESET_2_NAME "Pokemon TCG"
#define PRESET_2_STARTING_LIFE 60
#define PRESET_2_SMALL_STEP 10
#define PRESET_2_LARGE_STEP 30
#define PRESET_2_THEME 3                 // Pokemon

// PRESET 3: Yu-Gi-Oh!
#define PRESET_3_NAME "Yu-Gi-Oh!"
#define PRESET_3_STARTING_LIFE 8000
#define PRESET_3_SMALL_STEP 50
#define PRESET_3_LARGE_STEP 500
#define PRESET_3_THEME 1                 // Yugioh

// PRESET 4: Flesh & Blood
#define PRESET_4_NAME "Flesh & Blood"
#define PRESET_4_STARTING_LIFE 40
#define PRESET_4_SMALL_STEP 1
#define PRESET_4_LARGE_STEP 5
#define PRESET_4_THEME 1                 // Yugioh

// PRESET 5: Lorcana
#define PRESET_5_NAME "Lorcana"
#define PRESET_5_STARTING_LIFE 20
#define PRESET_5_SMALL_STEP 1
#define PRESET_5_LARGE_STEP 5
#define PRESET_5_THEME 1                 // Yugioh

// PRESET 6: One Piece TCG
#define PRESET_6_NAME "One Piece TCG"
#define PRESET_6_STARTING_LIFE 5
#define PRESET_6_SMALL_STEP 1
#define PRESET_6_LARGE_STEP 2
#define PRESET_6_THEME 1                 // Yugioh

// PRESET 7-9: Custom Presets
#define PRESET_7_NAME "Custom 8"
#define PRESET_7_STARTING_LIFE 20
#define PRESET_7_SMALL_STEP 1
#define PRESET_7_LARGE_STEP 5
#define PRESET_7_THEME 1

#define PRESET_8_NAME "Custom 9"
#define PRESET_8_STARTING_LIFE 20
#define PRESET_8_SMALL_STEP 1
#define PRESET_8_LARGE_STEP 5
#define PRESET_8_THEME 1

#define PRESET_9_NAME "Custom 10"
#define PRESET_9_STARTING_LIFE 20
#define PRESET_9_SMALL_STEP 1
#define PRESET_9_LARGE_STEP 5
#define PRESET_9_THEME 1


// ============================================================================
// 👆 TOUCH CALIBRATION DEFAULTS
// ============================================================================
// 7-parameter affine transformation matrix
// Used by: src/hardware/display/lvgl_driver.cpp
// Tested values: scale_x=0.85, scale_y=0.90 for ESP32-S3-Touch-LCD-1.85C
// ============================================================================

#define TOUCH_CAL_DEFAULT_OFFSET_X 0.0f
#define TOUCH_CAL_DEFAULT_SCALE_X 0.85f
#define TOUCH_CAL_DEFAULT_SHEAR_XY 0.0f
#define TOUCH_CAL_DEFAULT_OFFSET_Y 0.0f
#define TOUCH_CAL_DEFAULT_SHEAR_YX 0.0f
#define TOUCH_CAL_DEFAULT_SCALE_Y 0.90f
#define TOUCH_CAL_DEFAULT_DIVISOR 1.0f


// ============================================================================
// 🖥️  DISPLAY & POWER DEFAULTS
// ============================================================================
// Default settings for screen, auto-dim, sleep, battery management
// Used by various files - will be centralized to use these values
// ============================================================================

// Screen Brightness (0-255)
#define BRIGHTNESS_DEFAULT 100           // Default: 100 (39%)

// Auto-Dim Settings (in seconds)
// Available UI options: 0, 30, 60, 120, 300
#define AUTO_DIM_DEFAULT 60              // Default: 60s (1 minute)

// Sleep Settings (in seconds)
// Available UI options: 0, 120, 300, 600
#define SLEEP_DEFAULT 300                // Default: 300s (5 minutes)

// Low Battery Dimming
#define LOW_BATTERY_DIM_DEFAULT 1        // Default: 1 (ON)


// ============================================================================
// 🔊 AUDIO DEFAULTS
// ============================================================================
// Audio configuration
// Used by: src/hardware/audio/simple_audio.h and simple_audio.cpp
// ============================================================================

#define AUDIO_ENABLED_DEFAULT 1          // 1=ON, 0=OFF
#define AUDIO_VOLUME_DEFAULT 10          // Volume: 0-100
#define TIMER_SOUND_DEFAULT 1            // 1=ON, 0=OFF


// ============================================================================
// ⏱️  TIMER DEFAULTS
// ============================================================================
// Timer configuration
// Used by: src/ui/screens/settings/timer_settings.cpp
// ============================================================================

#define TIMER_ENABLED_DEFAULT 0          // 0=OFF, 1=ON
#define TIMER_DEFAULT_MINUTES 10         // Default round time in minutes


// ============================================================================
// 🎨 UI TIMING & BEHAVIOR
// ============================================================================
// Animation speeds, gesture detection, interaction timing
// Used throughout the UI code
// ============================================================================

// Animation Durations (milliseconds)
#define ARC_ANIMATION_DURATION 600       // Life counter arc animation
#define GROUPER_WINDOW 500               // Life change grouping window for auto-save

// Touch & Gesture
#define TOUCH_BLOCK_AFTER_WAKE_MS 300    // Block touches after wake from sleep
#define SWIPE_THRESHOLD_PX 80            // Pixels to trigger swipe gesture

// Touch Calibration Parameters
#define CALIBRATION_TIMEOUT_MS 10000     // Confirmation timeout (10 seconds)
#define CALIBRATION_LOCK_FRAMES 10       // Frames to lock calibration step


// ============================================================================
// 🔧 ADVANCED FORCE FLAGS (Development only)
// ============================================================================

#define FORCE_THEME_MODE false
#define FORCED_THEME_MODE_VALUE 0        // 0=OFF, 1=Automatic, 2=Manual

#define FORCE_THEME_SELECTION false
#define FORCED_THEME_VALUE 0             // 0=Off, 1=Yugioh, 2=Magic, 3=Pokemon, 4=Default

#define FORCE_AUDIO_SETTINGS false
#define FORCED_AUDIO_ENABLED 1
#define FORCED_AUDIO_VOLUME 75

#define FORCE_BRIGHTNESS false
#define FORCED_BRIGHTNESS_VALUE 200

// ============================================================================

#endif // CONFIG_H
