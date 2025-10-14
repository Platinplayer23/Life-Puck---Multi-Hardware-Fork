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
  settings_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(settings_menu, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(settings_menu, BLACK_COLOR, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(settings_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(settings_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(settings_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(settings_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_pad_all(settings_menu, 8, LV_PART_MAIN); // Etwas weniger Padding = mehr Scroll-Platz
  
  // Explizit Scrolling aktivieren
  lv_obj_clear_flag(settings_menu, LV_OBJ_FLAG_SCROLL_MOMENTUM);  // Einfaches Scrolling
  lv_obj_set_scroll_dir(settings_menu, LV_DIR_VER);              // Nur vertikal

  // SWIPE TO CLOSE
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled) {
    static lv_point_t menu_start_point;
    static bool menu_is_swiping = false;
    
    lv_obj_add_event_cb(settings_menu, [](lv_event_t *e) {
      lv_event_code_t code = lv_event_get_code(e);
      
      if (code == LV_EVENT_PRESSED) {
        lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &menu_start_point);
        menu_is_swiping = false;
      }
      else if (code == LV_EVENT_PRESSING) {
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        int dx = point.x - menu_start_point.x;
        int dy = point.y - menu_start_point.y;
        
        if (abs(dx) > 30 && abs(dx) > abs(dy) * 1.5) {
          menu_is_swiping = true;
        }
      }
      else if (code == LV_EVENT_RELEASED) {
        if (menu_is_swiping) {
          lv_indev_t *indev = lv_indev_get_act();
          lv_point_t point;
          lv_indev_get_point(indev, &point);
          int dx = point.x - menu_start_point.x;
          int dy = point.y - menu_start_point.y;
          
          if (dx > 80 && abs(dx) > abs(dy) * 2) {
            renderMenu(MENU_CONTEXTUAL, false);
            return;
          }
        }
      }
    }, LV_EVENT_ALL, NULL);
  }

  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, 50, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // Timer sound button added
  lv_obj_set_grid_dsc_array(settings_menu, col_dsc, row_dsc);
  lv_obj_set_layout(settings_menu, LV_LAYOUT_GRID);
  lv_obj_set_scrollbar_mode(settings_menu, LV_SCROLLBAR_MODE_AUTO);
  
  // Explizit Scrolling forcieren
  lv_obj_set_scroll_dir(settings_menu, LV_DIR_VER);
  lv_obj_clear_flag(settings_menu, LV_OBJ_FLAG_SCROLL_MOMENTUM);

  // Battery indicator (Row 0 - top, centered)
  lv_obj_t *battery_label = lv_label_create(settings_menu);
  lv_obj_set_grid_cell(battery_label, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);  // Row 0, spanning both columns
  lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_16, 0);  // Smaller font for less space
  lv_obj_set_style_text_color(battery_label, lv_color_white(), 0);
  
  // Get battery percentage and create display text
  int battery_percentage = (int)(battery_get_percent() + 0.5f);
  char battery_text[16];
  snprintf(battery_text, sizeof(battery_text), LV_SYMBOL_BATTERY_FULL " %d%%", battery_percentage);
  lv_label_set_text(battery_label, battery_text);

  // Back button (Row 1, Centered across both columns)
  lv_obj_t *btn_back = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_back, 120, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 1, 1);  // Row 1, span across both columns
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_CONTEXTUAL, false); }, LV_EVENT_CLICKED, NULL);


  // Start Life button (Row 2, Col 0)
  lv_obj_t *btn_life = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_life, 120, 50);
  lv_obj_set_style_bg_color(btn_life, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_life, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);  // Changed from Row 1 to Row 2
  lv_obj_t *lbl_life = lv_label_create(btn_life);
  lv_label_set_text(lbl_life, "Start Life");
  lv_obj_set_style_text_font(lbl_life, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_life);
  lv_obj_add_event_cb(btn_life, btn_life_event_cb, LV_EVENT_CLICKED, NULL);

  // Brightness button (Row 2, Col 1)
  lv_obj_t *btn_brightness = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_brightness, 120, 50);
  lv_obj_set_style_bg_color(btn_brightness, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_brightness, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 2, 1);  // Changed from Row 1 to Row 2
  lv_obj_add_event_cb(btn_brightness, [](lv_event_t *e)
                      { renderMenu(MENU_BRIGHTNESS); }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_brightness = lv_label_create(btn_brightness);
  lv_label_set_text(lbl_brightness, "Brightness");
  lv_obj_set_style_text_font(lbl_brightness, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_brightness);

  // Amp Counter Toggle (Row 2, Col 0)
  int amp_mode = player_store.getInt(KEY_AMP_MODE, 0);
  lv_obj_t *btn_amp_toggle = lv_btn_create(settings_menu);
  lv_obj_set_style_bg_color(btn_amp_toggle, (amp_mode ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
  lv_obj_set_size(btn_amp_toggle, 120, 50);
  lv_obj_set_grid_cell(btn_amp_toggle, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 3, 1);  // Changed from Row 2 to Row 3
  lv_obj_t *lbl_amp_label = lv_label_create(btn_amp_toggle);
  lv_label_set_text(lbl_amp_label, (amp_mode ? "Amp On" : "Amp Off"));
  lv_obj_set_style_text_font(lbl_amp_label, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_amp_label);
  lv_obj_add_event_cb(btn_amp_toggle, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    int current = player_store.getInt(KEY_AMP_MODE, 0);
    int new_mode = !current;
    player_store.putInt(KEY_AMP_MODE, new_mode);
    lv_label_set_text(label, (new_mode ? "Amp On" : "Amp Off"));
    lv_obj_set_style_bg_color(btn, (new_mode ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
    extern lv_obj_t *amp_button;
    if (amp_button && !new_mode)
    {
      clear_amp();
      lv_obj_add_flag(amp_button, LV_OBJ_FLAG_HIDDEN);
    } else if (amp_button && new_mode)
    {
      lv_obj_clear_flag(amp_button, LV_OBJ_FLAG_HIDDEN);
    } }, LV_EVENT_CLICKED, NULL);

  // Swipe Close Toggle (Row 2, Col 1)
  bool swipe_enabled_btn = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  lv_obj_t *btn_swipe = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_swipe, 120, 50);
  lv_obj_set_style_bg_color(btn_swipe, (swipe_enabled_btn ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_swipe, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 3, 1);  // Changed from Row 2 to Row 3
  lv_obj_t *lbl_swipe = lv_label_create(btn_swipe);
  lv_label_set_text(lbl_swipe, (swipe_enabled_btn ? "Swipe On" : "Swipe Off"));
  lv_obj_set_style_text_font(lbl_swipe, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_swipe);
  lv_obj_add_event_cb(btn_swipe, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    bool current = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
    bool new_mode = !current;
    player_store.putInt(KEY_SWIPE_TO_CLOSE, new_mode ? 1 : 0);
    lv_label_set_text(label, (new_mode ? "Swipe On" : "Swipe Off"));
    lv_obj_set_style_bg_color(btn, (new_mode ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN); }, LV_EVENT_CLICKED, NULL);

  // Timer Toggle button (Row 4, Col 0)
  lv_obj_t *btn_timer_toggle = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_timer_toggle, 120, 50);
  lv_obj_set_style_bg_color(btn_timer_toggle, (player_store.getInt(KEY_SHOW_TIMER, 1) ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_timer_toggle, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 4, 1);  // Changed from Row 3 to Row 4
  lv_obj_t *lbl_timer = lv_label_create(btn_timer_toggle);
  uint64_t show_timer = player_store.getInt(KEY_SHOW_TIMER, 0);
  lv_label_set_text(lbl_timer, (show_timer ? "Timer On" : "Timer Off"));
  lv_obj_set_style_text_font(lbl_timer, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_timer);
  lv_obj_add_event_cb(btn_timer_toggle, [](lv_event_t *e)
                      { 
                        uint64_t show_timer = toggle_show_timer();
                        lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
                        lv_obj_t *label = lv_obj_get_child(btn, 0);
                        lv_obj_set_style_bg_color(btn, (show_timer ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
                        lv_label_set_text(label, (show_timer ? "Timer On" : "Timer Off"));
                        uint64_t life_counter_mode = player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
                        if (!show_timer)
                        {
                          teardown_timer();
                        } else {
                          lv_label_set_text(label, "Timer On");
                          lv_obj_t *active_counter = (life_counter_mode == PLAYER_MODE_ONE_PLAYER) ?
                            life_counter_container : life_counter_container_2p;
                          if (!active_counter)
                          {
                            printf("[renderSettingsOverlay] No active life counter found\n");
                            return;
                          }
                          render_timer(active_counter);
                          if(player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER) == PLAYER_MODE_ONE_PLAYER)
                          {
                            lv_obj_set_grid_cell(timer_container, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
                          } else {
                            lv_obj_set_grid_cell(timer_container, LV_GRID_ALIGN_CENTER, 0, 5, LV_GRID_ALIGN_START, 2, 1);
                          }
                        } }, LV_EVENT_CLICKED, NULL);

  // Timer Mode Toggle (Row 4, Col 1)
  TimerMode current_timer_mode = get_timer_mode();
  lv_obj_t *btn_timer_mode = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_timer_mode, 120, 50);
  lv_obj_set_style_bg_color(btn_timer_mode, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_timer_mode, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 4, 1);  // Changed from Row 3 to Row 4
  lv_obj_t *lbl_timer_mode = lv_label_create(btn_timer_mode);
  lv_label_set_text(lbl_timer_mode, (current_timer_mode == TIMER_MODE_STOPWATCH ? "Stopwatch" : "Round"));
  lv_obj_set_style_text_font(lbl_timer_mode, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_timer_mode);
  lv_obj_add_event_cb(btn_timer_mode, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    TimerMode current = get_timer_mode();
    TimerMode new_mode = (current == TIMER_MODE_STOPWATCH) ? TIMER_MODE_COUNTDOWN : TIMER_MODE_STOPWATCH;
    set_timer_mode(new_mode);
    lv_label_set_text(label, (new_mode == TIMER_MODE_STOPWATCH ? "Stopwatch" : "Round"));
    renderMenu(MENU_SETTINGS);
  }, LV_EVENT_CLICKED, NULL);

  // Set Round Time Button (Row 4, Col 0+1 span 2)
  TimerMode timer_mode_for_btn = get_timer_mode();
  bool is_countdown = (timer_mode_for_btn == TIMER_MODE_COUNTDOWN);
  lv_obj_t *btn_set_time = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_set_time, 250, 50);
  lv_obj_set_style_bg_color(btn_set_time, (is_countdown ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_set_time, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 5, 1);  // Changed from Row 4 to Row 5
  lv_obj_t *lbl_set_time = lv_label_create(btn_set_time);
  char time_str[32];
  snprintf(time_str, sizeof(time_str), "Set Round Time (%d min)", get_round_time() / 60);
  lv_label_set_text(lbl_set_time, time_str);
  lv_obj_set_style_text_font(lbl_set_time, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_set_time);
  
  if (is_countdown) {
    static const char *timer_kb_map[] = {
        "7", "8", "9", "\n",
        "4", "5", "6", "\n",
        "1", "2", "3", "\n",
        LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, NULL};
    static const lv_buttonmatrix_ctrl_t timer_kb_ctrl[] = {
        LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
        LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
        LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
        LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1};
    
    lv_obj_add_event_cb(btn_set_time, [](lv_event_t *e)
                        {
      lv_obj_t *popup = lv_obj_create(lv_scr_act());
      lv_obj_set_size(popup, SCREEN_WIDTH, SCREEN_HEIGHT);
      lv_obj_center(popup);
      lv_obj_set_style_bg_color(popup, lv_color_hex(0x000000), 0);
      lv_obj_set_style_border_width(popup, 0, 0);
      lv_obj_set_style_radius(popup, 0, 0);
      lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
      
      lv_obj_t *lbl_title = lv_label_create(popup);
      lv_label_set_text(lbl_title, "Round Time (min)");
      lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_20, 0);
      lv_obj_set_style_text_color(lbl_title, lv_color_hex(0x00FFFF), 0);
      lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 10);
      
      lv_obj_t *ta = lv_textarea_create(popup);
      lv_textarea_set_one_line(ta, true);
      lv_obj_set_style_text_font(ta, &lv_font_montserrat_32, 0);
      lv_obj_set_style_text_color(ta, lv_color_white(), 0);
      lv_obj_set_size(ta, SCREEN_WIDTH - 120, 60);
      lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 40);
      char current_time[8];
      snprintf(current_time, sizeof(current_time), "%d", get_round_time() / 60);
      lv_textarea_set_text(ta, current_time);
      
      lv_obj_t *kb = lv_keyboard_create(popup);
      lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_USER_1);
      lv_obj_set_style_text_font(kb, &lv_font_montserrat_24, 0);
      lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_USER_1, timer_kb_map, timer_kb_ctrl);
      lv_obj_set_size(kb, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 180);
      lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, -10);
      lv_keyboard_set_textarea(kb, ta);
      
      lv_obj_set_user_data(ta, popup);
      lv_obj_add_event_cb(ta, [](lv_event_t *ev) {
        lv_event_code_t code = lv_event_get_code(ev);
        if (code == LV_EVENT_READY) {
          lv_obj_t *ta_src = (lv_obj_t*)lv_event_get_target(ev);
          const char *text = lv_textarea_get_text(ta_src);
          int minutes = atoi(text);
          if (minutes > 0 && minutes <= 999) {
            set_round_time(minutes * 60);
          }
          lv_obj_t *popup_del = (lv_obj_t*)lv_obj_get_user_data(ta_src);
          lv_obj_del(popup_del);
          renderMenu(MENU_SETTINGS);
        } else if (code == LV_EVENT_CANCEL) {
          lv_obj_t *ta_src = (lv_obj_t*)lv_event_get_target(ev);
          lv_obj_t *popup_del = (lv_obj_t*)lv_obj_get_user_data(ta_src);
          lv_obj_del(popup_del);
        }
      }, LV_EVENT_ALL, NULL);
    }, LV_EVENT_CLICKED, NULL);
  }

  // Audio Enable/Disable Button (Row 6, Left)
  bool audio_enabled = simple_audio_is_enabled();
  lv_obj_t *btn_audio_toggle = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_audio_toggle, 120, 50);
  lv_obj_set_style_bg_color(btn_audio_toggle, (audio_enabled ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_audio_toggle, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 6, 1);
  lv_obj_t *lbl_audio = lv_label_create(btn_audio_toggle);
  lv_label_set_text(lbl_audio, (audio_enabled ? "Audio On" : "Audio Off"));
  lv_obj_set_style_text_font(lbl_audio, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_audio);
  lv_obj_add_event_cb(btn_audio_toggle, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    bool current = simple_audio_is_enabled();
    bool new_mode = !current;
    simple_audio_set_enabled(new_mode);
    lv_label_set_text(label, (new_mode ? "Audio On" : "Audio Off"));
    lv_obj_set_style_bg_color(btn, (new_mode ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
    // No click sound for audio toggle
  }, LV_EVENT_CLICKED, NULL);

  // Audio Volume Button (Row 6, Right)
  int current_volume = simple_audio_get_volume();
  lv_obj_t *btn_audio_volume = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_audio_volume, 120, 50);
  lv_obj_set_style_bg_color(btn_audio_volume, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_audio_volume, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 6, 1);
  lv_obj_t *lbl_volume = lv_label_create(btn_audio_volume);
  char volume_text[16];
  snprintf(volume_text, sizeof(volume_text), "Vol: %d", current_volume);
  lv_label_set_text(lbl_volume, volume_text);
  lv_obj_set_style_text_font(lbl_volume, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_volume);
  lv_obj_add_event_cb(btn_audio_volume, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    int current = simple_audio_get_volume();
    int new_volume = (current + 3) % 22; // Cycle through 0-21 in steps of 3
    if (new_volume == 0) new_volume = 1; // Skip 0, go to 1
    simple_audio_set_volume(new_volume);
    char volume_text[16];
    snprintf(volume_text, sizeof(volume_text), "Vol: %d", new_volume);
    lv_label_set_text(label, volume_text);
    // Play volume preview sound
    simple_audio_play_sound(SOUND_SUCCESS);
  }, LV_EVENT_CLICKED, NULL);

  // Timer Sound Button (Row 7, Left)
  sound_type_t current_timer_sound = simple_audio_get_timer_sound();
  lv_obj_t *btn_timer_sound = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_timer_sound, 120, 50);
  lv_obj_set_style_bg_color(btn_timer_sound, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_timer_sound, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 7, 1);
  lv_obj_t *lbl_timer_sound = lv_label_create(btn_timer_sound);
  const char* sound_names[] = {"Sound 1", "Sound 2", "Sound 3", "Sound 4"};
  lv_label_set_text(lbl_timer_sound, sound_names[current_timer_sound]);
  lv_obj_set_style_text_font(lbl_timer_sound, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_timer_sound);
  lv_obj_add_event_cb(btn_timer_sound, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    sound_type_t current = simple_audio_get_timer_sound();
    sound_type_t next = (sound_type_t)((current + 1) % 4); // Cycle through 0-3
    simple_audio_set_timer_sound(next);
    const char* sound_names[] = {"Sound 1", "Sound 2", "Sound 3", "Sound 4"};
    lv_label_set_text(label, sound_names[next]);
    // Play preview of new sound
    simple_audio_play_sound(next);
  }, LV_EVENT_CLICKED, NULL);

  // Edit Presets Button (Row 7, Right)
  lv_obj_t *btn_edit_presets = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_edit_presets, 120, 50);
  lv_obj_set_style_bg_color(btn_edit_presets, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_edit_presets, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 7, 1);
  lv_obj_t *lbl_edit = lv_label_create(btn_edit_presets);
  lv_label_set_text(lbl_edit, "Edit Presets");
  lv_obj_set_style_text_font(lbl_edit, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_edit);
  lv_obj_add_event_cb(btn_edit_presets, [](lv_event_t *e)
                      { renderMenu(MENU_PRESET_EDITOR); }, LV_EVENT_CLICKED, NULL);

  // Touch Calibration Button (Row 8, Left) 
  lv_obj_t *btn_touch_cal = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_touch_cal, 120, 50);
  lv_obj_set_style_bg_color(btn_touch_cal, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_touch_cal, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 8, 1);
  lv_obj_t *lbl_touch_cal = lv_label_create(btn_touch_cal);
  lv_label_set_text(lbl_touch_cal, "Touch Cal");
  lv_obj_set_style_text_font(lbl_touch_cal, &lv_font_montserrat_18, 0); // Zurück zu normal font
  lv_obj_center(lbl_touch_cal);
  lv_obj_add_event_cb(btn_touch_cal, [](lv_event_t *e)
                      { renderMenu(MENU_TOUCH_CALIBRATION); }, LV_EVENT_CLICKED, NULL);

  // *** SCROLL-FIX: Große unsichtbare Spacer-Komponente für Scrolling ***
  lv_obj_t *spacer = lv_obj_create(settings_menu);
  lv_obj_set_size(spacer, 10, 200);  // 200px hoch für Scrolling
  lv_obj_set_grid_cell(spacer, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 9, 1);  // Row 9, spanning both columns
  lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);  // Unsichtbar
  lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);  // Keine Border
  lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);  // Nicht klickbar

}

void teardownSettingsOverlay()
{
  if (settings_menu)
  {
    lv_obj_del(settings_menu);
    settings_menu = nullptr;
  }
}