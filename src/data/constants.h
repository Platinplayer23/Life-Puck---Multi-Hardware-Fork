#pragma once
#include "lvgl.h"

#define SCREEN_WIDTH 360
#define SCREEN_HEIGHT 360
#define SCREEN_DIAMETER SCREEN_WIDTH
#define ARC_WIDTH 8

// *** SMOOTH ANIMATION CONSTANTS ***
#define SMOOTH_ARC_STEPS 1000        // High-resolution animation steps for smooth arcs
#define ARC_ANIMATION_DURATION 300   // Fast, responsive 300ms animations

typedef struct
{
  int start_angle;
  int end_angle;
  lv_color_t color;
} arc_segment_t;

// Grouper constants
#define PLAYER_SINGLE 0
#define PLAYER_ONE 1
#define PLAYER_TWO 2
#define GROUPER_WINDOW 2000

enum PlayerMode
{
  PLAYER_MODE_ONE_PLAYER = 0,
  PLAYER_MODE_TWO_PLAYER = 1
};

// Enum for menu states
enum MenuState
{
  MENU_NONE,
  MENU_CONTEXTUAL,
  MENU_SETTINGS,
  MENU_LIFE_CONFIG,
  MENU_HISTORY,
  MENU_BRIGHTNESS,
  MENU_TCG, 
  MENU_DICE_LIST,
  MENU_PRESET_LIST,
  MENU_PRESET_EDITOR,    // Preset Editor
  MENU_TOUCH_CALIBRATION // NEW: Touch Calibration
};

enum ContextualQuadrant
{
  QUADRANT_TL = 0,
  QUADRANT_TR = 1,
  QUADRANT_BL = 2,
  QUADRANT_BR = 3,
  QUADRANT_EXIT = 4
};

// Color Constants
#define GREEN_COLOR lv_color_hex(0x00e31f)
#define YELLOW_COLOR lv_color_hex(0xebf700)
#define RED_COLOR lv_color_hex(0xe80000)
#define LIGHTNING_BLUE_COLOR lv_color_hex(0x0070ff)
#define WHITE_COLOR lv_color_hex(0xffffff)
#define BLACK_COLOR lv_color_hex(0x000000)
#define GRAY_COLOR lv_color_hex(0x808080)
#define DARK_GRAY_COLOR lv_color_hex(0x404040)
#define AMP_START_COLOR YELLOW_COLOR
#define AMP_END_COLOR RED_COLOR

// Constants for StateStore
#define PLAYER_STORE "player_store"
#define KEY_LIFE_MAX "life"
#define KEY_PLAYER_MODE "player_mode"
#define KEY_BRIGHTNESS "brightness"
#define KEY_AMP_MODE "amp_mode"
#define KEY_LIFE_STEP_SMALL "life_step_small"
#define KEY_LIFE_STEP_LARGE "life_step_large"
#define KEY_SHOW_TIMER "show_timer"
#define KEY_SWIPE_TO_CLOSE "swipe_close"  // NEW

// *** PERSISTENT LIFE VALUES ***
#define KEY_SAVED_LIFE_P1 "saved_life_p1"    // Single player or Player 1 life
#define KEY_SAVED_LIFE_P2 "saved_life_p2"    // Player 2 life (two-player mode)
#define KEY_LIFE_SAVE_VALID "life_save_valid" // Flag: 1 if saved values are valid

// define for life increment levels small and large
#define DEFAULT_LIFE_INCREMENT_SMALL 1
#define DEFAULT_LIFE_INCREMENT_LARGE 5
#define DEFAULT_LIFE_MAX 40

enum step_size_t
{
  STEP_SIZE_SMALL = 1,
  STEP_SIZE_LARGE = 2
};