#include "power_settings.h"
#include "data/constants.h"
#include "ui/screens/menu/menu.h"
#include "ui/screens/settings/brightness.h"
#include "core/state_manager.h"
#include <Arduino.h>

// NVS Keys for power settings
#define KEY_AUTO_DIM_TIME "auto_dim_time"
#define KEY_SLEEP_TIME "sleep_time"
#define KEY_BATTERY_SAVER "battery_saver"

static lv_obj_t *power_settings_menu = nullptr;

// Static labels for updating button states
static lv_obj_t *lbl_auto_dim = nullptr;
static lv_obj_t *lbl_sleep = nullptr;
static lv_obj_t *lbl_battery_saver = nullptr;

// Auto-Dim Time options (in seconds): Off, 30s, 1min, 2min, 5min
static const int AUTO_DIM_OPTIONS[] = {0, 30, 60, 120, 300};
static const int AUTO_DIM_COUNT = 5;

// Sleep Time options (in seconds): Off, 2min, 5min, 10min
static const int SLEEP_OPTIONS[] = {0, 120, 300, 600};
static const int SLEEP_COUNT = 4;

// Helper function to format time
static String formatTime(int seconds) {
  if (seconds == 0) return "OFF";
  if (seconds < 60) return String(seconds) + "s";
  return String(seconds / 60) + "min";
}

// Helper function to get current auto-dim index
static int getAutoDimIndex() {
  int current = player_store.getInt(KEY_AUTO_DIM_TIME, 60); // Default: 1min
  for (int i = 0; i < AUTO_DIM_COUNT; i++) {
    if (AUTO_DIM_OPTIONS[i] == current) return i;
  }
  return 2; // Default to 1min if not found
}

// Helper function to get current sleep index
static int getSleepIndex() {
  int current = player_store.getInt(KEY_SLEEP_TIME, 300); // Default: 5min
  for (int i = 0; i < SLEEP_COUNT; i++) {
    if (SLEEP_OPTIONS[i] == current) return i;
  }
  return 2; // Default to 5min if not found
}

// Update button display
static void updateAutoDimButton(lv_obj_t *btn) {
  int index = getAutoDimIndex();
  int value = AUTO_DIM_OPTIONS[index];
  String text = "Auto-Dim: " + formatTime(value);
  lv_label_set_text(lbl_auto_dim, text.c_str());
  
  // Gray out if OFF
  if (value == 0) {
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x444444), 0);
  } else {
    lv_obj_set_style_bg_color(btn, LIGHTNING_BLUE_COLOR, 0);
  }
}

static void updateSleepButton(lv_obj_t *btn) {
  int index = getSleepIndex();
  int value = SLEEP_OPTIONS[index];
  String text = "Sleep: " + formatTime(value);
  lv_label_set_text(lbl_sleep, text.c_str());
  
  // Gray out if OFF
  if (value == 0) {
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x444444), 0);
  } else {
    lv_obj_set_style_bg_color(btn, LIGHTNING_BLUE_COLOR, 0);
  }
}

static void updateBatterySaverButton(lv_obj_t *btn) {
  bool enabled = player_store.getInt(KEY_BATTERY_SAVER, 1) != 0; // Default: ON
  String text = "Low Battery Dimming: " + String(enabled ? "ON" : "OFF");
  lv_label_set_text(lbl_battery_saver, text.c_str());
  
  // Gray out if OFF
  if (!enabled) {
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x444444), 0);
  } else {
    lv_obj_set_style_bg_color(btn, LIGHTNING_BLUE_COLOR, 0);
  }
}

void renderPowerSettingsMenu()
{
  if (power_settings_menu) {
    teardownPowerSettingsMenu();
  }

  // Create main container - same style as audio/timer settings
  power_settings_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(power_settings_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(power_settings_menu);
  lv_obj_set_style_bg_color(power_settings_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(power_settings_menu, 15, 0);
  lv_obj_set_style_border_width(power_settings_menu, 0, 0);
  lv_obj_set_flex_flow(power_settings_menu, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(power_settings_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scrollbar_mode(power_settings_menu, LV_SCROLLBAR_MODE_AUTO);

  // Grid Layout: Full width buttons (1 column, 6 rows)
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(power_settings_menu, col_dsc, row_dsc);
  lv_obj_set_layout(power_settings_menu, LV_LAYOUT_GRID);

  // Title (Row 0)
  lv_obj_t *title = lv_label_create(power_settings_menu);
  lv_label_set_text(title, "Power Settings");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_grid_cell(title, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  // Back button (Row 1)
  lv_obj_t *btn_back = lv_btn_create(power_settings_menu);
  lv_obj_set_size(btn_back, 120, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 1, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e) {
    renderMenu(MENU_SETTINGS);
  }, LV_EVENT_CLICKED, NULL);

  // Brightness Button (Row 2, Full Width)
  lv_obj_t *btn_brightness = lv_btn_create(power_settings_menu);
  lv_obj_set_size(btn_brightness, 280, 50);
  lv_obj_set_style_bg_color(btn_brightness, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_brightness, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_t *lbl_brightness = lv_label_create(btn_brightness);
  lv_label_set_text(lbl_brightness, "Brightness");
  lv_obj_set_style_text_font(lbl_brightness, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_brightness);
  lv_obj_add_event_cb(btn_brightness, [](lv_event_t *e) {
    renderBrightnessOverlay(MENU_POWER_SETTINGS);
  }, LV_EVENT_CLICKED, NULL);

  // Auto-Dim Button (Row 3, Full Width)
  lv_obj_t *btn_auto_dim = lv_btn_create(power_settings_menu);
  lv_obj_set_size(btn_auto_dim, 280, 50);
  lv_obj_set_style_bg_color(btn_auto_dim, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_auto_dim, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 3, 1);
  lbl_auto_dim = lv_label_create(btn_auto_dim);
  lv_obj_set_style_text_font(lbl_auto_dim, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_auto_dim);
  
  updateAutoDimButton(btn_auto_dim);
  
  lv_obj_add_event_cb(btn_auto_dim, [](lv_event_t *e) {
    lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
    
    // Cycle through options
    int index = getAutoDimIndex();
    index = (index + 1) % AUTO_DIM_COUNT;
    player_store.putInt(KEY_AUTO_DIM_TIME, AUTO_DIM_OPTIONS[index]);
    
    updateAutoDimButton(btn);
    printf("[PowerSettings] Auto-Dim set to %d seconds\n", AUTO_DIM_OPTIONS[index]);
  }, LV_EVENT_CLICKED, NULL);

  // Sleep Button (Row 4, Full Width)
  lv_obj_t *btn_sleep = lv_btn_create(power_settings_menu);
  lv_obj_set_size(btn_sleep, 280, 50);
  lv_obj_set_style_bg_color(btn_sleep, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_sleep, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 4, 1);
  lbl_sleep = lv_label_create(btn_sleep);
  lv_obj_set_style_text_font(lbl_sleep, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_sleep);
  
  updateSleepButton(btn_sleep);
  
  lv_obj_add_event_cb(btn_sleep, [](lv_event_t *e) {
    lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
    
    // Cycle through options
    int index = getSleepIndex();
    index = (index + 1) % SLEEP_COUNT;
    player_store.putInt(KEY_SLEEP_TIME, SLEEP_OPTIONS[index]);
    
    updateSleepButton(btn);
    printf("[PowerSettings] Sleep set to %d seconds\n", SLEEP_OPTIONS[index]);
  }, LV_EVENT_CLICKED, NULL);

  // Battery Saver Button (Row 5, Full Width)
  lv_obj_t *btn_battery_saver = lv_btn_create(power_settings_menu);
  lv_obj_set_size(btn_battery_saver, 280, 50);
  lv_obj_set_style_bg_color(btn_battery_saver, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_battery_saver, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 5, 1);
  lbl_battery_saver = lv_label_create(btn_battery_saver);
  lv_obj_set_style_text_font(lbl_battery_saver, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_battery_saver);
  
  updateBatterySaverButton(btn_battery_saver);
  
  lv_obj_add_event_cb(btn_battery_saver, [](lv_event_t *e) {
    lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
    
    // Toggle on/off
    bool current = player_store.getInt(KEY_BATTERY_SAVER, 1) != 0;
    player_store.putInt(KEY_BATTERY_SAVER, current ? 0 : 1);
    
    updateBatterySaverButton(btn);
    printf("[PowerSettings] Battery Saver %s\n", current ? "OFF" : "ON");
  }, LV_EVENT_CLICKED, NULL);

  // Extra Spacer for Scrolling (Row 6)
  lv_obj_t *extra_spacer = lv_obj_create(power_settings_menu);
  lv_obj_set_size(extra_spacer, 10, 200);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 6, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);

  // Add swipe-to-close gesture (same as audio/timer settings menu)
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled) {
    static lv_point_t start_point;
    static bool is_swiping = false;
    
    lv_obj_add_event_cb(power_settings_menu, [](lv_event_t *e) {
      lv_event_code_t code = lv_event_get_code(e);
      
      if (code == LV_EVENT_PRESSED) {
        lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &start_point);
        is_swiping = false;
      }
      else if (code == LV_EVENT_PRESSING) {
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        int dx = point.x - start_point.x;
        int dy = point.y - start_point.y;
        
        if (abs(dx) > 30 && abs(dx) > abs(dy) * 1.5) {
          is_swiping = true;
        }
      }
      else if (code == LV_EVENT_RELEASED) {
        if (is_swiping) {
          lv_indev_t *indev = lv_indev_get_act();
          lv_point_t point;
          lv_indev_get_point(indev, &point);
          int dx = point.x - start_point.x;
          int dy = point.y - start_point.y;
          
          if (dx > 80 && abs(dx) > abs(dy) * 2) {
            renderMenu(MENU_SETTINGS);
            return;
          }
        }
      }
    }, LV_EVENT_ALL, NULL);
  }

  printf("[PowerSettings] Menu rendered\n");
}

void teardownPowerSettingsMenu()
{
  if (power_settings_menu) {
    lv_obj_del(power_settings_menu);
    power_settings_menu = nullptr;
    lbl_auto_dim = nullptr;
    lbl_sleep = nullptr;
    lbl_battery_saver = nullptr;
    printf("[PowerSettings] Menu torn down\n");
  }
}

