// ============================================
// Own Header (first!)
// ============================================
#include "settings_overlay.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// Hardware
// ============================================
#include "hardware/system/battery_state.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/menu/menu.h"
#include "ui/screens/life/life_counter.h"
#include "ui/screens/life/life_counter_two_player.h"
#include "ui/screens/tools/timer.h"

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/animation_helpers.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "hardware/audio/simple_audio.h"


extern lv_obj_t *settings_menu;
extern lv_obj_t *life_counter_container;
extern lv_obj_t *life_counter_container_2p;

static void btn_life_event_cb(lv_event_t *e)
{
  renderMenu(MENU_LIFE_CONFIG);
}

void renderSettingsOverlay()
{
  teardownSettingsOverlay();
  hideLifeScreen();
  
  settings_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(settings_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(settings_menu);
  lv_obj_set_style_bg_color(settings_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(settings_menu, 15, 0);
  lv_obj_set_style_border_width(settings_menu, 0, 0);
  lv_obj_set_flex_flow(settings_menu, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(settings_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scrollbar_mode(settings_menu, LV_SCROLLBAR_MODE_AUTO);

  // SWIPE TO CLOSE - Always add handler, but check setting inside
  static lv_point_t start_point;
  static bool is_swiping = false;
    
    lv_obj_add_event_cb(settings_menu, [](lv_event_t *e) {
    // Check if swipe is enabled for this specific event
    bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
    if (!swipe_enabled) return; // Exit early if disabled
    
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
            renderMenu(MENU_CONTEXTUAL, false);
            return;
          }
        }
      }
    }, LV_EVENT_ALL, NULL);

  // Grid Layout: 2 columns, 6 rows
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(settings_menu, col_dsc, row_dsc);
  lv_obj_set_layout(settings_menu, LV_LAYOUT_GRID);
  lv_obj_set_scrollbar_mode(settings_menu, LV_SCROLLBAR_MODE_AUTO);

  // Battery indicator (Row 0 - top, centered)
  lv_obj_t *battery_label = lv_label_create(settings_menu);
  lv_obj_set_grid_cell(battery_label, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(battery_label, lv_color_white(), 0);
  
  int battery_percentage = (int)(battery_get_percent() + 0.5f);
  char battery_text[16];
  snprintf(battery_text, sizeof(battery_text), LV_SYMBOL_BATTERY_FULL " %d%%", battery_percentage);
  lv_label_set_text(battery_label, battery_text);

  // Back button (Row 1, Centered across both columns)
  lv_obj_t *btn_back = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_back, 120, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 1, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_CONTEXTUAL, false); }, LV_EVENT_CLICKED, NULL);

  // Audio Settings Submenu (Row 2, Col 0)
  lv_obj_t *btn_audio_settings = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_audio_settings, 140, 50);
  lv_obj_set_style_bg_color(btn_audio_settings, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_audio_settings, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_t *lbl_audio_settings = lv_label_create(btn_audio_settings);
  lv_label_set_text(lbl_audio_settings, "Audio");
  lv_obj_set_style_text_font(lbl_audio_settings, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_audio_settings);
  lv_obj_add_event_cb(btn_audio_settings, [](lv_event_t *e)
                      { renderMenu(MENU_AUDIO_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  // Timer Settings Submenu (Row 2, Col 1)
  lv_obj_t *btn_timer_settings = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_timer_settings, 140, 50);
  lv_obj_set_style_bg_color(btn_timer_settings, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_timer_settings, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_t *lbl_timer_settings = lv_label_create(btn_timer_settings);
  lv_label_set_text(lbl_timer_settings, "Timer");
  lv_obj_set_style_text_font(lbl_timer_settings, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_timer_settings);
  lv_obj_add_event_cb(btn_timer_settings, [](lv_event_t *e)
                      { renderMenu(MENU_TIMER_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  // Touch Calibration button (Row 3, Col 0)
  lv_obj_t *btn_touch_cal = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_touch_cal, 120, 50);
  lv_obj_set_style_bg_color(btn_touch_cal, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_touch_cal, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 3, 1);
  lv_obj_t *lbl_touch_cal = lv_label_create(btn_touch_cal);
  lv_label_set_text(lbl_touch_cal, "Touch Cal");
  lv_obj_set_style_text_font(lbl_touch_cal, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_touch_cal);
  lv_obj_add_event_cb(btn_touch_cal, [](lv_event_t *e)
                      { renderMenu(MENU_TOUCH_CALIBRATION); }, LV_EVENT_CLICKED, NULL);

  // Start Life button (Row 3, Col 1)
  lv_obj_t *btn_life = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_life, 120, 50);
  lv_obj_set_style_bg_color(btn_life, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_life, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 3, 1);
  lv_obj_t *lbl_life = lv_label_create(btn_life);
  lv_label_set_text(lbl_life, "Start Life");
  lv_obj_set_style_text_font(lbl_life, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_life);
  lv_obj_add_event_cb(btn_life, btn_life_event_cb, LV_EVENT_CLICKED, NULL);

  // Brightness button (Row 4, Col 0)
  lv_obj_t *btn_brightness = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_brightness, 120, 50);
  lv_obj_set_style_bg_color(btn_brightness, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_brightness, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 4, 1);
  lv_obj_add_event_cb(btn_brightness, [](lv_event_t *e)
                      { renderMenu(MENU_BRIGHTNESS); }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_brightness = lv_label_create(btn_brightness);
  lv_label_set_text(lbl_brightness, "Brightness");
  lv_obj_set_style_text_font(lbl_brightness, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_brightness);

  // Edit Presets button (Row 4, Col 1)
  lv_obj_t *btn_edit_presets = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_edit_presets, 120, 50);
  lv_obj_set_style_bg_color(btn_edit_presets, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_edit_presets, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 4, 1);
  lv_obj_t *lbl_edit_presets = lv_label_create(btn_edit_presets);
  lv_label_set_text(lbl_edit_presets, "Edit Presets");
  lv_obj_set_style_text_font(lbl_edit_presets, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_edit_presets);
  lv_obj_add_event_cb(btn_edit_presets, [](lv_event_t *e)
                      { renderMenu(MENU_PRESET_EDITOR); }, LV_EVENT_CLICKED, NULL);

  // AMP Button (Row 5, Col 0)
  int amp_mode = player_store.getInt(KEY_AMP_MODE, PLAYER_SINGLE);
  bool amp_enabled = (amp_mode == PLAYER_MODE_TWO_PLAYER);
  lv_obj_t *btn_amp = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_amp, 120, 50);
  lv_obj_set_style_bg_color(btn_amp, (amp_enabled ? LIGHTNING_BLUE_COLOR : lv_color_hex(0x444444)), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_amp, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 5, 1);
  lv_obj_t *lbl_amp = lv_label_create(btn_amp);
  lv_label_set_text(lbl_amp, amp_enabled ? "AMP: ON" : "AMP: OFF");
  lv_obj_set_style_text_font(lbl_amp, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_amp);
  lv_obj_add_event_cb(btn_amp, [](lv_event_t *e)
                      {
                        int amp_mode = player_store.getInt(KEY_AMP_MODE, PLAYER_SINGLE);
                        amp_mode = (amp_mode == PLAYER_SINGLE) ? PLAYER_MODE_TWO_PLAYER : PLAYER_SINGLE;
                        player_store.putInt(KEY_AMP_MODE, amp_mode);
                        printf("[Settings] AMP mode toggled to: %d\n", amp_mode);
                        
                        // Toggle AMP button visibility in life counter
                        toggle_amp_visibility();
                        
                        // Update button text and color directly (no re-render)
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
                        bool amp_enabled = (amp_mode == PLAYER_MODE_TWO_PLAYER);
                        lv_label_set_text(label, amp_enabled ? "AMP: ON" : "AMP: OFF");
                        lv_obj_set_style_bg_color(btn, (amp_enabled ? LIGHTNING_BLUE_COLOR : lv_color_hex(0x444444)), LV_PART_MAIN);
                      }, LV_EVENT_CLICKED, NULL);

  // Swipe to Close Button (Row 5, Col 1)
  bool swipe_enabled_btn = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  lv_obj_t *btn_swipe = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_swipe, 120, 50);
  lv_obj_set_style_bg_color(btn_swipe, (swipe_enabled_btn ? LIGHTNING_BLUE_COLOR : lv_color_hex(0x444444)), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_swipe, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 5, 1);
  lv_obj_t *lbl_swipe = lv_label_create(btn_swipe);
  lv_label_set_text(lbl_swipe, swipe_enabled_btn ? "Swipe: ON" : "Swipe: OFF");
  lv_obj_set_style_text_font(lbl_swipe, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_swipe);
  lv_obj_add_event_cb(btn_swipe, [](lv_event_t *e)
                      {
                        int swipe_state = player_store.getInt(KEY_SWIPE_TO_CLOSE, 0);
                        swipe_state = (swipe_state == 0) ? 1 : 0;
                        player_store.putInt(KEY_SWIPE_TO_CLOSE, swipe_state);
                        printf("[Settings] Swipe to close toggled to: %d\n", swipe_state);
                        // Update button text and color directly (no re-render)
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
                        bool swipe_enabled = (swipe_state != 0);
                        lv_label_set_text(label, swipe_enabled ? "Swipe: ON" : "Swipe: OFF");
                        lv_obj_set_style_bg_color(btn, (swipe_enabled ? LIGHTNING_BLUE_COLOR : lv_color_hex(0x444444)), LV_PART_MAIN);
                        
                        // No need to update event handlers - they check the setting dynamically
  }, LV_EVENT_CLICKED, NULL);

  // Spacer (Row 6, Col 0) - Empty space for scrolling
  lv_obj_t *spacer = lv_obj_create(settings_menu);
  lv_obj_set_size(spacer, 120, 50);
  lv_obj_set_grid_cell(spacer, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 6, 1);
  lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);

  // Extra Spacer for Scrolling (Row 7, spanning both columns)
  lv_obj_t *extra_spacer = lv_obj_create(settings_menu);
  lv_obj_set_size(extra_spacer, 10, 200);  // 200px hoch für Scrolling
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 7, 1);  // Row 7, spanning both columns
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);  // Unsichtbar
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);  // Keine Border
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);  // Nicht klickbar
}

void teardownSettingsOverlay()
{
  if (settings_menu)
  {
    lv_obj_del(settings_menu);
    settings_menu = nullptr;
  }
}

