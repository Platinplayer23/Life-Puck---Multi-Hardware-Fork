// ============================================
// Own Header (first!)
// ============================================
#include "menu.h"

// ============================================
// System & Framework Headers
// ============================================
#include <Arduino.h>
#include <lvgl.h>
#include <math.h>

// ============================================
// Core System
// ============================================
#include "core/gui_main.h"
#include "core/state_manager.h"

// ============================================
// Hardware
// ============================================
#include "hardware/system/battery_state.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/life/life_counter.h"
#include "ui/screens/life/life_counter_two_player.h"
#include "ui/screens/settings/settings_overlay.h"
#include "ui/screens/settings/brightness.h"
#include "ui/screens/settings/touch_calibration.h"
#include "ui/screens/settings/audio_settings.h"
#include "ui/screens/settings/timer_settings.h"
#include "ui/screens/tools/timer.h"
#include "ui/screens/tools/dice_coin.h"
#include "ui/screens/menu/preset_editor.h"

// ============================================
// UI Components
// ============================================
#include "ui/components/start_life.h"

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/animation_helpers.h"
#include "ui/helpers/tap_layer.h"
#include "ui/helpers/event_grouper.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "data/history.h"
#include "data/tcg_presets.h"

lv_obj_t *contextual_menu = nullptr;
lv_obj_t *settings_menu = nullptr;
lv_obj_t *life_config_menu = nullptr;
lv_obj_t *history_menu = nullptr;
lv_obj_t *brightness_control = nullptr;
lv_obj_t *dice_list_menu = nullptr;
lv_obj_t *preset_list_menu = nullptr;

static MenuState currentMenu = MENU_NONE;
static int current_menu_page = 0;
static const int TOTAL_MENU_PAGES = 2;

static void togglePlayerMode();
void resetActiveCounter();
static void showHistoryOverlay();
static void showLifeScreen();
void hideLifeScreen();
void teardownContextualMenuOverlay();
static bool is_in_center_cancel_area(lv_event_t *e);
void renderMenu(MenuState menuType);
void renderMenu(MenuState menuType, bool animate_menu);
bool is_in_quadrant(lv_event_t *e, int angle_start, int angle_end);
void renderDiceListMenu();
void renderPresetListMenu();
void teardownDiceListMenu();
void teardownPresetListMenu();
void show_tcg_result_popup(const char* title, const char* result);

int circle_diameter = SCREEN_WIDTH;
int circle_radius = circle_diameter / 2;

MenuState getCurrentMenu()
{
  return currentMenu;
}

void handleContextualSelection(ContextualQuadrant quadrant)
{
  if (current_menu_page == 0) {
    switch (quadrant)
    {
    case QUADRANT_TL:
      renderMenu(MENU_SETTINGS);
      break;
    case QUADRANT_TR:
      togglePlayerMode();
      break;
    case QUADRANT_BL:
      resetActiveCounter();
      break;
    case QUADRANT_BR:
      renderMenu(MENU_PRESET_LIST);
      break;
    }
  } else {
    switch (quadrant)
    {
    case QUADRANT_TL:
      renderMenu(MENU_DICE_LIST);
      break;
    case QUADRANT_TR:
      {
        bool heads = flip_coin();
        show_tcg_result_popup("Coin flip", heads ? "HEADS" : "TAILS");
      }
      break;
    case QUADRANT_BL:
      renderMenu(MENU_HISTORY);
      break;
    case QUADRANT_BR:
      break;
    }
  }
}

static void togglePlayerMode()
{
  PlayerMode current_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  if (current_mode != PLAYER_MODE_ONE_PLAYER && current_mode != PLAYER_MODE_TWO_PLAYER)
    current_mode = PLAYER_MODE_ONE_PLAYER;
  PlayerMode new_mode = (current_mode == PLAYER_MODE_ONE_PLAYER) ? PLAYER_MODE_TWO_PLAYER : PLAYER_MODE_ONE_PLAYER;
  player_store.putInt(KEY_PLAYER_MODE, (int)new_mode);
  
  // Reset life points when switching player modes - use existing reset function
  resetActiveCounter();
  
  printf("[togglePlayerMode] Player mode toggled to %d, life points reset\n", new_mode);
  ui_init();
  renderMenu(MENU_NONE);
}

void resetActiveCounter()
{
  PlayerMode player_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  if (player_mode == PLAYER_MODE_ONE_PLAYER)
  {
    reset_life();
    clear_amp();
  }
  else if (player_mode == PLAYER_MODE_TWO_PLAYER)
  {
    reset_life_2p();
  }
  printf("[resetActiveCounter] Reset life counter and history for player mode %d\n", player_mode);
  reset_timer();
  showLifeScreen();
  teardownAllMenus();
}

void show_tcg_result_popup(const char* title, const char* result) {
  lv_obj_t *popup = lv_obj_create(lv_scr_act());
  lv_obj_set_size(popup, 180, 150);
  lv_obj_center(popup);
  lv_obj_set_style_bg_color(popup, lv_color_hex(0x202020), 0);
  lv_obj_set_style_border_width(popup, 3, 0);
  lv_obj_set_style_border_color(popup, LIGHTNING_BLUE_COLOR, 0);
  lv_obj_set_style_radius(popup, 20, 0);
  
  lv_obj_t *lbl_title = lv_label_create(popup);
  lv_label_set_text(lbl_title, title);
  lv_obj_set_style_text_color(lbl_title, LIGHTNING_BLUE_COLOR, 0);
  lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 15);
  
  lv_obj_t *lbl_result = lv_label_create(popup);
  lv_label_set_text(lbl_result, result);
  lv_obj_set_style_text_font(lbl_result, &lv_font_montserrat_40, 0);
  lv_obj_center(lbl_result);
  
  lv_timer_t *timer = lv_timer_create([](lv_timer_t *t) {
    lv_obj_t *obj = (lv_obj_t*)lv_timer_get_user_data(t);
    lv_obj_del(obj);
    lv_timer_del(t);
  }, 2000, nullptr);
  lv_timer_set_user_data(timer, popup);
  lv_timer_set_repeat_count(timer, 1);
}

void renderDiceListMenu() {
  teardownDiceListMenu();
  hideLifeScreen();
  
  dice_list_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(dice_list_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(dice_list_menu);
  lv_obj_set_style_bg_color(dice_list_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(dice_list_menu, 15, 0);
  lv_obj_set_style_border_width(dice_list_menu, 0, 0);
  lv_obj_set_flex_flow(dice_list_menu, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(dice_list_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scrollbar_mode(dice_list_menu, LV_SCROLLBAR_MODE_AUTO);
  
  // SWIPE DIREKT AUF DEM MENU
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled) {
    static lv_point_t dice_start_point;
    static bool dice_is_swiping = false;
    
    lv_obj_add_event_cb(dice_list_menu, [](lv_event_t *e) {
      lv_event_code_t code = lv_event_get_code(e);
      
      if (code == LV_EVENT_PRESSED) {
        lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &dice_start_point);
        dice_is_swiping = false;
      }
      else if (code == LV_EVENT_PRESSING) {
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        int dx = point.x - dice_start_point.x;
        int dy = point.y - dice_start_point.y;
        
        if (abs(dx) > 30 && abs(dx) > abs(dy) * 1.5) {
          dice_is_swiping = true;
        }
      }
      else if (code == LV_EVENT_RELEASED) {
        if (dice_is_swiping) {
          lv_indev_t *indev = lv_indev_get_act();
          lv_point_t point;
          lv_indev_get_point(indev, &point);
          int dx = point.x - dice_start_point.x;
          int dy = point.y - dice_start_point.y;
          
          if (dx > 80 && abs(dx) > abs(dy) * 2) {
            current_menu_page = 1;
            renderMenu(MENU_CONTEXTUAL, false);
            return;
          }
        }
      }
    }, LV_EVENT_ALL, NULL);
  }
  
  lv_obj_t *title = lv_label_create(dice_list_menu);
  lv_label_set_text(title, "DICE");
  lv_obj_set_style_text_color(title, LIGHTNING_BLUE_COLOR, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_pad_bottom(title, 10, 0);
  
  for (int i = 0; i < DICE_TYPE_COUNT; i++) {
    lv_obj_t *btn = lv_btn_create(dice_list_menu);
    lv_obj_set_size(btn, 160, 40);
    lv_obj_set_style_bg_color(btn, LIGHTNING_BLUE_COLOR, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)i);
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
      lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
      int idx = (int)(intptr_t)lv_obj_get_user_data(target);
      int result = DICE_TYPES[idx].roll_func();
      char buf[16];
      snprintf(buf, sizeof(buf), "%d", result);
      show_tcg_result_popup(DICE_TYPES[idx].name, buf);
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, DICE_TYPES[i].name);
    lv_obj_center(lbl);
  }
  
  lv_obj_t *btn_back = lv_btn_create(dice_list_menu);
  lv_obj_set_size(btn_back, 160, 40);
  lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x444444), 0);
  lv_obj_set_style_margin_top(btn_back, 10, 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e) {
    current_menu_page = 1;
    renderMenu(MENU_CONTEXTUAL, false);
  }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_color(lbl_back, lv_color_white(), 0);
  lv_obj_center(lbl_back);
  
  currentMenu = MENU_DICE_LIST;
}

void renderPresetListMenu() {
  teardownPresetListMenu();
  hideLifeScreen();
  
  preset_list_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(preset_list_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(preset_list_menu);
  lv_obj_set_style_bg_color(preset_list_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(preset_list_menu, 15, 0);
  lv_obj_set_style_border_width(preset_list_menu, 0, 0);
  lv_obj_set_flex_flow(preset_list_menu, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(preset_list_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scrollbar_mode(preset_list_menu, LV_SCROLLBAR_MODE_AUTO);
  
  // SWIPE DIREKT AUF DEM MENU
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled) {
    static lv_point_t preset_start_point;
    static bool preset_is_swiping = false;
    
    lv_obj_add_event_cb(preset_list_menu, [](lv_event_t *e) {
      lv_event_code_t code = lv_event_get_code(e);
      
      if (code == LV_EVENT_PRESSED) {
        lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &preset_start_point);
        preset_is_swiping = false;
      }
      else if (code == LV_EVENT_PRESSING) {
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        int dx = point.x - preset_start_point.x;
        int dy = point.y - preset_start_point.y;
        
        if (abs(dx) > 30 && abs(dx) > abs(dy) * 1.5) {
          preset_is_swiping = true;
        }
      }
      else if (code == LV_EVENT_RELEASED) {
        if (preset_is_swiping) {
          lv_indev_t *indev = lv_indev_get_act();
          lv_point_t point;
          lv_indev_get_point(indev, &point);
          int dx = point.x - preset_start_point.x;
          int dy = point.y - preset_start_point.y;
          
          if (dx > 80 && abs(dx) > abs(dy) * 2) {
            current_menu_page = 0;
            renderMenu(MENU_CONTEXTUAL, false);
            return;
          }
        }
      }
    }, LV_EVENT_ALL, NULL);
  }
  
  lv_obj_t *title = lv_label_create(preset_list_menu);
  lv_label_set_text(title, "PRESETS");
  lv_obj_set_style_text_color(title, LIGHTNING_BLUE_COLOR, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_pad_bottom(title, 10, 0);
  
  for (int i = 0; i < TCG_PRESET_COUNT; i++) {
    lv_obj_t *btn = lv_btn_create(preset_list_menu);
    lv_obj_set_size(btn, 180, 45);
    lv_obj_set_style_bg_color(btn, LIGHTNING_BLUE_COLOR, 0);
    lv_obj_set_user_data(btn, (void*)(intptr_t)i);
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
      lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
      int idx = (int)(intptr_t)lv_obj_get_user_data(target);
      save_preset(idx);
      
      TCGPreset preset = get_preset();
      player_store.putInt(KEY_LIFE_MAX, preset.starting_life);
      player_store.putInt(KEY_LIFE_STEP_SMALL, preset.small_step);
      player_store.putInt(KEY_LIFE_STEP_LARGE, preset.large_step);
      
      ui_init();
      renderMenu(MENU_NONE);
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *lbl = lv_label_create(btn);
    char buf[64];
    snprintf(buf, sizeof(buf), "%s (%d)", TCG_PRESETS[i].name, TCG_PRESETS[i].starting_life);
    lv_label_set_text(lbl, buf);
    lv_obj_center(lbl);
  }
  
  lv_obj_t *btn_back = lv_btn_create(preset_list_menu);
  lv_obj_set_size(btn_back, 160, 40);
  lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x444444), 0);
  lv_obj_set_style_margin_top(btn_back, 10, 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e) {
    current_menu_page = 0;
    renderMenu(MENU_CONTEXTUAL, false);
  }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_color(lbl_back, lv_color_white(), 0);
  lv_obj_center(lbl_back);
  
  currentMenu = MENU_PRESET_LIST;
}

void renderContextualMenuOverlay(bool animate_menu)
{
  teardownContextualMenuOverlay();
  int circle_diameter = (SCREEN_WIDTH < SCREEN_HEIGHT ? SCREEN_WIDTH : SCREEN_HEIGHT);
  int circle_radius = circle_diameter / 2;

  contextual_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(contextual_menu, circle_diameter, circle_diameter);
  lv_obj_set_style_bg_color(contextual_menu, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(contextual_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(contextual_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(contextual_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(contextual_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_align(contextual_menu, LV_ALIGN_CENTER, 0, 0);
  lv_obj_clear_flag(contextual_menu, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(contextual_menu, LV_OBJ_FLAG_GESTURE_BUBBLE);

  int ring_radius = circle_radius;
  
  if (current_menu_page == 0) {
    lv_obj_t *lbl_tl = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_tl, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_font(lbl_tl, &lv_font_montserrat_40, 0);
    lv_obj_align(lbl_tl, LV_ALIGN_CENTER, -ring_radius / 2, -ring_radius / 2);

    lv_obj_t *lbl_tr = lv_label_create(contextual_menu);
    const char *lbl_text = player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER) == PLAYER_MODE_ONE_PLAYER ? "2P" : "1P";
    lv_label_set_text(lbl_tr, lbl_text);
    lv_obj_set_style_text_font(lbl_tr, &lv_font_montserrat_40, 0);
    lv_obj_align(lbl_tr, LV_ALIGN_CENTER, ring_radius / 2, -ring_radius / 2);

    lv_obj_t *lbl_bl = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_bl, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_font(lbl_bl, &lv_font_montserrat_40, 0);
    lv_obj_align(lbl_bl, LV_ALIGN_CENTER, -ring_radius / 2, ring_radius / 2);

    lv_obj_t *lbl_br = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_br, LV_SYMBOL_LIST);
    lv_obj_set_style_text_font(lbl_br, &lv_font_montserrat_40, 0);
    lv_obj_align(lbl_br, LV_ALIGN_CENTER, ring_radius / 2, ring_radius / 2);
  } else {
    lv_obj_t *lbl_tl = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_tl, "D");
    lv_obj_set_style_text_font(lbl_tl, &lv_font_montserrat_40, 0);
    lv_obj_align(lbl_tl, LV_ALIGN_CENTER, -ring_radius / 2, -ring_radius / 2);

    lv_obj_t *lbl_tr = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_tr, "C");
    lv_obj_set_style_text_font(lbl_tr, &lv_font_montserrat_40, 0);
    lv_obj_align(lbl_tr, LV_ALIGN_CENTER, ring_radius / 2, -ring_radius / 2);

    lv_obj_t *lbl_bl = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_bl, LV_SYMBOL_DOWNLOAD);
    lv_obj_set_style_text_font(lbl_bl, &lv_font_montserrat_40, 0);
    lv_obj_align(lbl_bl, LV_ALIGN_CENTER, -ring_radius / 2, ring_radius / 2);
  }

  static bool swipe_detected = false;
  
  lv_obj_add_event_cb(contextual_menu, [](lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
      swipe_detected = false;
    }
    else if (code == LV_EVENT_GESTURE) {
      lv_indev_t *indev = lv_indev_get_act();
      lv_dir_t dir = lv_indev_get_gesture_dir(indev);
      if (dir == LV_DIR_LEFT) {
        current_menu_page = (current_menu_page + 1) % TOTAL_MENU_PAGES;
        renderMenu(MENU_CONTEXTUAL, false);
        swipe_detected = true;
      } else if (dir == LV_DIR_RIGHT) {
        current_menu_page = (current_menu_page - 1 + TOTAL_MENU_PAGES) % TOTAL_MENU_PAGES;
        renderMenu(MENU_CONTEXTUAL, false);
        swipe_detected = true;
      }
    }
    else if (code == LV_EVENT_CLICKED && !swipe_detected) {
      if (is_in_center_cancel_area(e)) {
        return;
      }
      if (is_in_quadrant(e, -180, -90)) {
        handleContextualSelection(QUADRANT_TL);
      } else if (is_in_quadrant(e, -90, 0)) {
        handleContextualSelection(QUADRANT_TR);
      } else if (is_in_quadrant(e, 0, 90)) {
        handleContextualSelection(QUADRANT_BR);
      } else if (is_in_quadrant(e, 90, 180)) {
        handleContextualSelection(QUADRANT_BL);
      }
    }
  }, LV_EVENT_ALL, NULL);

  lv_obj_add_flag(contextual_menu, LV_OBJ_FLAG_CLICKABLE);

  int hole_diameter = (ring_radius / 3) * 2;
  lv_obj_t *center_cancel = lv_btn_create(contextual_menu);
  lv_obj_set_size(center_cancel, hole_diameter, hole_diameter);
  lv_obj_align(center_cancel, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(center_cancel, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(center_cancel, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(center_cancel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_opa(center_cancel, LV_OPA_TRANSP, 0);
  lv_obj_add_event_cb(center_cancel, [](lv_event_t *e)
                      {
    renderMenu(MENU_NONE); }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_cancel = lv_label_create(center_cancel);
  lv_label_set_text(lbl_cancel, LV_SYMBOL_CLOSE);
  lv_obj_set_style_text_font(lbl_cancel, &lv_font_montserrat_48, 0);
  lv_obj_center(lbl_cancel);

  int dot_size = 8;
  int dot_spacing = 20;
  int start_x = (circle_diameter / 2) - 10;
  int y_pos = circle_radius + 75;
  
  for (int i = 0; i < TOTAL_MENU_PAGES; i++) {
    lv_obj_t *dot = lv_obj_create(contextual_menu);
    lv_obj_set_size(dot, dot_size, dot_size);
    lv_obj_set_pos(dot, start_x + (i * dot_spacing) - (TOTAL_MENU_PAGES * dot_spacing / 2), y_pos);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
    
    if (i == current_menu_page) {
      lv_obj_set_style_bg_color(dot, LIGHTNING_BLUE_COLOR, 0);
      lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    } else {
      lv_obj_set_style_bg_color(dot, lv_color_hex(0x666666), 0);
      lv_obj_set_style_bg_opa(dot, LV_OPA_50, 0);
    }
  }

  if (animate_menu)
  {
    lv_obj_set_y(contextual_menu, -SCREEN_HEIGHT);
    slide_in_obj_vertical(contextual_menu, -SCREEN_HEIGHT, 0, 250, 0, nullptr);
  }
}

void renderMenu(MenuState menuType)
{
  renderMenu(menuType, true);
}

void renderMenu(MenuState menuType, bool animate_menu)
{
  teardownAllMenus();
  hideLifeScreen();
  switch (menuType)
  {
  case MENU_CONTEXTUAL:
    renderContextualMenuOverlay(animate_menu);
    currentMenu = MENU_CONTEXTUAL;
    break;
  case MENU_SETTINGS:
    renderSettingsOverlay();
    currentMenu = MENU_SETTINGS;
    break;
  case MENU_LIFE_CONFIG:
    renderLifeConfigScreen();
    currentMenu = MENU_LIFE_CONFIG;
    break;
  case MENU_HISTORY:
    renderHistoryOverlay();
    currentMenu = MENU_HISTORY;
    break;
  case MENU_BRIGHTNESS:
    renderBrightnessOverlay();
    currentMenu = MENU_BRIGHTNESS;
    break;
  case MENU_DICE_LIST:
    renderDiceListMenu();
    break;
  case MENU_PRESET_LIST:
    renderPresetListMenu();
    break;
  case MENU_PRESET_EDITOR:
    renderPresetEditorMenu();
    currentMenu = MENU_PRESET_EDITOR;
    break;
  case MENU_TOUCH_CALIBRATION:
    renderTouchCalibrationScreen();
    currentMenu = MENU_TOUCH_CALIBRATION;
    break;
  case MENU_AUDIO_SETTINGS:
    teardownAudioSettingsMenu();
    renderAudioSettingsMenu();
    currentMenu = MENU_AUDIO_SETTINGS;
    break;
  case MENU_TIMER_SETTINGS:
    teardownTimerSettingsMenu();
    renderTimerSettingsMenu();
    currentMenu = MENU_TIMER_SETTINGS;
    break;
  case MENU_NONE:
  default:
    showLifeScreen();
    currentMenu = MENU_NONE;
    break;
  }
}

static bool is_in_center_cancel_area(lv_event_t *e)
{
  lv_point_t p;
  lv_indev_get_point(lv_indev_get_act(), &p);
  int x = p.x, y = p.y;
  int circle_x = (SCREEN_WIDTH - circle_diameter) / 2;
  int circle_y = (SCREEN_HEIGHT - circle_diameter) / 2;
  int cx = circle_x + circle_radius;
  int cy = circle_y + circle_radius;
  int ring_radius = circle_radius;
  int dx = x - cx, dy = y - cy;
  float dist = sqrtf(dx * dx + dy * dy);
  int hole_radius = ring_radius / 3;
  return (dist < hole_radius);
}

bool is_in_quadrant(lv_event_t *e, int angle_start, int angle_end)
{
  lv_point_t p;
  lv_indev_get_point(lv_indev_get_act(), &p);
  int x = p.x, y = p.y;
  int circle_x = (SCREEN_WIDTH - circle_diameter) / 2;
  int circle_y = (SCREEN_HEIGHT - circle_diameter) / 2;
  int cx = circle_x + circle_radius;
  int cy = circle_y + circle_radius;
  int ring_diameter = circle_diameter;
  int ring_radius = ring_diameter / 2;
  int dx = x - cx, dy = y - cy;
  float angle = atan2f(dy, dx) * 180.0f / M_PI;
  float dist = sqrtf(dx * dx + dy * dy);
  int hole_radius = ring_radius / 3;
  return (dist >= hole_radius && dist <= ring_radius && angle >= angle_start && angle < angle_end);
}

extern lv_obj_t *life_counter_container;
extern lv_obj_t *life_counter_container_2p;

void hideLifeScreen()
{
  if (life_counter_container)
    lv_obj_add_flag(life_counter_container, LV_OBJ_FLAG_HIDDEN);
  if (life_counter_container_2p)
    lv_obj_add_flag(life_counter_container_2p, LV_OBJ_FLAG_HIDDEN);
}

void showLifeScreen()
{
  if (life_counter_container)
    lv_obj_clear_flag(life_counter_container, LV_OBJ_FLAG_HIDDEN);
  if (life_counter_container_2p)
    lv_obj_clear_flag(life_counter_container_2p, LV_OBJ_FLAG_HIDDEN);
}

void teardownAllMenus()
{
  currentMenu = MENU_NONE;
  teardownContextualMenuOverlay();
  teardownSettingsOverlay();
  teardownStartLifeScreen();
  teardownHistoryOverlay();
  teardownBrightnessOverlay();
  teardownDiceListMenu();
  teardownPresetListMenu();
  teardownPresetEditorMenu();
  teardownTouchCalibrationScreen();
  teardownAudioSettingsMenu();
  teardownTimerSettingsMenu();
}

void teardownContextualMenuOverlay()
{
  if (contextual_menu)
  {
    lv_obj_del(contextual_menu);
    contextual_menu = nullptr;
  }
}

void teardownDiceListMenu()
{
  if (dice_list_menu)
  {
    lv_obj_del(dice_list_menu);
    dice_list_menu = nullptr;
  }
}

void teardownPresetListMenu()
{
  if (preset_list_menu)
  {
    lv_obj_del(preset_list_menu);
    preset_list_menu = nullptr;
  }
}