#include "timer_settings.h"
#include "data/constants.h"
#include "ui/screens/menu/menu.h"
#include "ui/helpers/gestures.h"
#include "core/state_manager.h"
#include "ui/screens/tools/timer.h"
#include "ui/screens/life/life_counter.h"
#include "ui/screens/life/life_counter_two_player.h"
#include <lvgl.h>

static lv_obj_t *timer_menu = nullptr;

extern lv_obj_t *life_counter_container;
extern lv_obj_t *life_counter_container_2p;
extern lv_obj_t *timer_container;

// Shared input state struct (like in start_life.cpp)
struct SharedInputState
{
  lv_obj_t *ta;
  lv_obj_t *kb;
  int *current_var;
  lv_obj_t *current_label;
};

// Static variables for round time input
static int round_time_var = 10; // Default value
static lv_obj_t *lbl_set_time = nullptr;

// Static event callback for shared textarea (like in start_life.cpp)
void timer_shared_ta_event_cb(lv_event_t *e)
{
  SharedInputState *state = (SharedInputState *)lv_event_get_user_data(e);
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED && state->current_var && state->current_label)
  {
    int value = atoi(lv_textarea_get_text(state->ta));
    if (value >= 1 && value <= 60) // Round time: 1-60 minutes
    {
      *(state->current_var) = value;
      set_round_time(value * 60); // Convert minutes to seconds
      char buf[32];
      snprintf(buf, sizeof(buf), "Set Round Time: %d min", value);
      lv_label_set_text(state->current_label, buf);
    }
  }
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
  {
    // Always fully reset state
    lv_obj_add_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(state->kb, NULL);
    state->current_var = nullptr;
    state->current_label = nullptr;
  }
}

// Keyboard map (like in start_life.cpp)
static const char *kb_map[] = {
    "7", "8", "9", "\n",
    "4", "5", "6", "\n",
    "1", "2", "3", "\n",
    LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, NULL};

static const lv_buttonmatrix_ctrl_t kb_ctrl[] = {
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1};

void renderTimerSettingsMenu()
{
  teardownTimerSettingsMenu();
  
  // Initialize round time variable (convert seconds to minutes)
  round_time_var = get_round_time() / 60;

  // Create main container - same style as settings menu
  timer_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(timer_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(timer_menu);
  lv_obj_set_style_bg_color(timer_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(timer_menu, 15, 0);
  lv_obj_set_style_border_width(timer_menu, 0, 0);
  lv_obj_set_flex_flow(timer_menu, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(timer_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scrollbar_mode(timer_menu, LV_SCROLLBAR_MODE_AUTO);

  // Grid Layout: 2 columns, 4 rows
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(timer_menu, col_dsc, row_dsc);
  lv_obj_set_layout(timer_menu, LV_LAYOUT_GRID);

  // Title (Row 0, spanning both columns)
  lv_obj_t *title = lv_label_create(timer_menu);
  lv_label_set_text(title, "Timer Settings");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_grid_cell(title, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);

  // Back button (Row 1, spanning both columns)
  lv_obj_t *btn_back = lv_btn_create(timer_menu);
  lv_obj_set_size(btn_back, 120, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 1, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  // Timer Toggle button (Row 2, Col 0)
  lv_obj_t *btn_timer_toggle = lv_btn_create(timer_menu);
  lv_obj_set_size(btn_timer_toggle, 120, 50);
  lv_obj_set_style_bg_color(btn_timer_toggle, (player_store.getInt(KEY_SHOW_TIMER, 1) ? LIGHTNING_BLUE_COLOR : lv_color_hex(0x444444)), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_timer_toggle, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_t *lbl_timer = lv_label_create(btn_timer_toggle);
  uint64_t show_timer = player_store.getInt(KEY_SHOW_TIMER, 0);
  lv_label_set_text(lbl_timer, (show_timer ? "Timer On" : "Timer Off"));
  lv_obj_set_style_text_font(lbl_timer, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_timer);
  lv_obj_add_event_cb(btn_timer_toggle, [](lv_event_t *e)
                      { 
                        uint64_t show_timer = toggle_show_timer();
                        lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
                        lv_obj_t *label = lv_obj_get_child(btn, 0);
                        lv_obj_set_style_bg_color(btn, (show_timer ? LIGHTNING_BLUE_COLOR : lv_color_hex(0x444444)), LV_PART_MAIN);
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
                            printf("[renderTimerSettingsMenu] No active life counter found\n");
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

  // Timer Mode Toggle (Row 2, Col 1)
  TimerMode current_timer_mode = get_timer_mode();
  lv_obj_t *btn_timer_mode = lv_btn_create(timer_menu);
  lv_obj_set_size(btn_timer_mode, 120, 50);
  lv_obj_set_style_bg_color(btn_timer_mode, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_timer_mode, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 2, 1);
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
    // Re-render timer settings menu to update the Set Round Time button
    renderTimerSettingsMenu();
  }, LV_EVENT_CLICKED, NULL);

  // Set Round Time Button (Row 3, spanning both columns)
  TimerMode timer_mode_for_btn = get_timer_mode();
  bool is_countdown = (timer_mode_for_btn == TIMER_MODE_COUNTDOWN);
  lv_obj_t *btn_set_time = lv_btn_create(timer_menu);
  lv_obj_set_size(btn_set_time, 250, 50);
  lv_obj_set_style_bg_color(btn_set_time, (is_countdown ? LIGHTNING_BLUE_COLOR : lv_color_hex(0x444444)), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_set_time, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 3, 1);
  lbl_set_time = lv_label_create(btn_set_time); // Use static variable
  char time_text[32];
  snprintf(time_text, sizeof(time_text), "Set Round Time: %d min", get_round_time() / 60);
  lv_label_set_text(lbl_set_time, time_text);
  lv_obj_set_style_text_font(lbl_set_time, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_set_time);
  
          // Event callback will be added after shared input system is created

  // Spacer (Row 3, Col 1) - Empty space
  lv_obj_t *spacer = lv_obj_create(timer_menu);
  lv_obj_set_size(spacer, 120, 50);
  lv_obj_set_grid_cell(spacer, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 3, 1);
  lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);

  // Extra Spacer for Scrolling (Row 4, spanning both columns)
  lv_obj_t *extra_spacer = lv_obj_create(timer_menu);
  lv_obj_set_size(extra_spacer, 10, 200);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 4, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);


  // Add swipe-to-close gesture (same as main settings menu)
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled) {
    static lv_point_t start_point;
    static bool is_swiping = false;
    
    lv_obj_add_event_cb(timer_menu, [](lv_event_t *e) {
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

  // Initialize shared input state (like in start_life.cpp)
  static SharedInputState shared_input_state = {nullptr, nullptr, nullptr, nullptr};

  // Create shared text area - POSITIONED HIGHER
  shared_input_state.ta = lv_textarea_create(timer_menu);
  lv_textarea_set_one_line(shared_input_state.ta, true);
  lv_obj_set_style_text_font(shared_input_state.ta, &lv_font_montserrat_32, 0);  // Larger
  lv_obj_set_style_text_color(shared_input_state.ta, lv_color_white(), 0);
  lv_obj_set_size(shared_input_state.ta, SCREEN_WIDTH - 120, 60);
  lv_obj_align(shared_input_state.ta, LV_ALIGN_TOP_MID, 0, 20);  // Higher
  lv_obj_add_flag(shared_input_state.ta, LV_OBJ_FLAG_HIDDEN);

  // Create shared keyboard
  shared_input_state.kb = lv_keyboard_create(timer_menu);
  lv_keyboard_set_mode(shared_input_state.kb, LV_KEYBOARD_MODE_NUMBER);
  lv_obj_add_flag(shared_input_state.kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_text_font(shared_input_state.kb, &lv_font_montserrat_24, 0);
  lv_keyboard_set_map(shared_input_state.kb, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
  lv_keyboard_set_mode(shared_input_state.kb, LV_KEYBOARD_MODE_USER_1);
  lv_obj_set_size(shared_input_state.kb, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 150);
  lv_obj_align(shared_input_state.kb, LV_ALIGN_TOP_MID, 0, 80);

  // Attach event callback to shared textarea
  lv_obj_add_event_cb(shared_input_state.ta, timer_shared_ta_event_cb, LV_EVENT_ALL, &shared_input_state);

  // Add event callback to Set Round Time button (only if countdown mode)
  if (is_countdown) {
    lv_obj_add_event_cb(btn_set_time, [](lv_event_t *e) {
      // Show the shared input system
      lv_obj_clear_flag(shared_input_state.ta, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(shared_input_state.kb, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(shared_input_state.kb, shared_input_state.ta);
      lv_obj_move_foreground(shared_input_state.kb);
      
      // Set current round time value (convert seconds to minutes)
      char buf[16];
      snprintf(buf, sizeof(buf), "%d", get_round_time() / 60);
      lv_textarea_set_text(shared_input_state.ta, buf);
      shared_input_state.current_var = &round_time_var;
      shared_input_state.current_label = lbl_set_time;
    }, LV_EVENT_CLICKED, NULL);
  }
}

void teardownTimerSettingsMenu()
{
  if (timer_menu)
  {
    lv_obj_del(timer_menu);
    timer_menu = nullptr;
  }
}
