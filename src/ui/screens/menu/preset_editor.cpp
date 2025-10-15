// ============================================
// Own Header (first!)
// ============================================
#include "preset_editor.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>
#include <cstring>
#include <cstdlib>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// UI Screens (same folder)
// ============================================
#include "menu.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "data/tcg_presets.h"


static lv_obj_t *preset_editor_menu = nullptr;
static lv_obj_t *preset_editor_swipe_layer = nullptr;
static lv_obj_t *name_popup = nullptr;
static lv_obj_t *values_popup = nullptr;
static lv_obj_t *ta_name = nullptr;
static lv_obj_t *keyboard = nullptr;

struct SharedInputState
{
  lv_obj_t *ta;
  lv_obj_t *kb;
  int *current_var;
  lv_obj_t *current_label;
};

static int temp_life = 0;
static int temp_small = 0;
static int temp_large = 0;
static int current_preset_idx = -1;

static void show_name_popup(int preset_idx);
static void show_values_popup(int preset_idx);
static void close_name_popup();
static void close_values_popup();
static void save_preset_to_storage(int preset_idx);
static void load_presets_from_storage();

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

void renderPresetEditorMenu() {
  if (preset_editor_menu) {
    lv_obj_del(preset_editor_menu);
  }
  
  if (preset_editor_swipe_layer) {
    lv_obj_del(preset_editor_swipe_layer);
    preset_editor_swipe_layer = nullptr;
  }

  load_presets_from_storage();
  
  preset_editor_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(preset_editor_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(preset_editor_menu);
  lv_obj_set_style_bg_color(preset_editor_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(preset_editor_menu, 15, 0);
  lv_obj_set_style_border_width(preset_editor_menu, 0, 0);
  lv_obj_set_flex_flow(preset_editor_menu, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(preset_editor_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scrollbar_mode(preset_editor_menu, LV_SCROLLBAR_MODE_AUTO);
  
  // SWIPE DETECTION DIREKT AUF DEM MENU
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled) {
    static lv_point_t menu_start_point;
    static bool menu_is_swiping = false;
    
    lv_obj_add_event_cb(preset_editor_menu, [](lv_event_t *e) {
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
            renderMenu(MENU_SETTINGS);
            return;
          }
        }
      }
    }, LV_EVENT_ALL, NULL);
  }
  
  lv_obj_t *title = lv_label_create(preset_editor_menu);
  lv_label_set_text(title, "PRESETS");
  lv_obj_set_style_text_color(title, LIGHTNING_BLUE_COLOR, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_pad_bottom(title, 10, 0);
  
  for (int i = 0; i < 10; i++) {
    lv_obj_t *row = lv_obj_create(preset_editor_menu);
    lv_obj_set_size(row, 220, 50);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(0x333333), 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(row, (void*)(intptr_t)i);
    
    lv_obj_t *lbl = lv_label_create(row);
    char buf[64];
    snprintf(buf, sizeof(buf), "%s (%d)", TCG_PRESETS[i].name, TCG_PRESETS[i].starting_life);
    lv_label_set_text(lbl, buf);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_center(lbl);
    
    lv_obj_add_event_cb(row, [](lv_event_t *e) {
      if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
        int idx = (int)(intptr_t)lv_obj_get_user_data(target);
        show_name_popup(idx);
      }
    }, LV_EVENT_CLICKED, NULL);
  }
  
  lv_obj_t *btn_back = lv_btn_create(preset_editor_menu);
  lv_obj_set_size(btn_back, 160, 40);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), 0);
  lv_obj_set_style_margin_top(btn_back, 15, 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e) {
    renderMenu(MENU_SETTINGS);
  }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_center(lbl_back);
}

static void show_name_popup(int preset_idx) {
  close_name_popup();
  current_preset_idx = preset_idx;
  
  name_popup = lv_obj_create(lv_scr_act());
  lv_obj_set_size(name_popup, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_center(name_popup);
  lv_obj_set_style_bg_color(name_popup, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(name_popup, 0, 0);
  lv_obj_set_style_radius(name_popup, 0, 0);
  lv_obj_clear_flag(name_popup, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *lbl_title = lv_label_create(name_popup);
  lv_label_set_text(lbl_title, "Name:");
  lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(lbl_title, LIGHTNING_BLUE_COLOR, 0);
  lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 10);
  
  ta_name = lv_textarea_create(name_popup);
  lv_obj_set_size(ta_name, SCREEN_WIDTH - 40, 50);
  lv_obj_align(ta_name, LV_ALIGN_TOP_MID, 0, 45);
  lv_textarea_set_text(ta_name, TCG_PRESETS[preset_idx].name);
  lv_textarea_set_max_length(ta_name, 20); // Maximum 20 characters
  lv_textarea_set_one_line(ta_name, true);
  lv_obj_set_style_text_font(ta_name, &lv_font_montserrat_24, 0);
  
  keyboard = lv_keyboard_create(name_popup);
  lv_keyboard_set_textarea(keyboard, ta_name);
  lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
  lv_obj_set_size(keyboard, SCREEN_WIDTH - 20, 180);
  lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -50);
  
  lv_obj_add_event_cb(ta_name, [](lv_event_t *ev) {
    lv_event_code_t code = lv_event_get_code(ev);
    if (code == LV_EVENT_VALUE_CHANGED) {
      // Limit name length to 20 characters
      lv_obj_t *ta_src = (lv_obj_t*)lv_event_get_target(ev);
      const char *text = lv_textarea_get_text(ta_src);
      if (strlen(text) > 20) {
        char limited[21];
        strncpy(limited, text, 20);
        limited[20] = '\0';
        lv_textarea_set_text(ta_src, limited);
      }
    }
    else if (code == LV_EVENT_READY) {
      lv_obj_t *ta_src = (lv_obj_t*)lv_event_get_target(ev);
      const char *name = lv_textarea_get_text(ta_src);
      // Ensure name is max 20 characters
      char limited[21];
      strncpy(limited, name, 20);
      limited[20] = '\0';
      strncpy(TCG_PRESETS[current_preset_idx].name, limited, sizeof(TCG_PRESETS[current_preset_idx].name) - 1);
      TCG_PRESETS[current_preset_idx].name[sizeof(TCG_PRESETS[current_preset_idx].name) - 1] = '\0';
      close_name_popup();
      show_values_popup(current_preset_idx);
    }
  }, LV_EVENT_ALL, NULL);
  
  lv_obj_t *btn_apply = lv_btn_create(name_popup);
  lv_obj_set_size(btn_apply, 120, 45);
  lv_obj_align(btn_apply, LV_ALIGN_BOTTOM_MID, 0, -3);
  lv_obj_set_style_bg_color(btn_apply, lv_color_hex(0x00AA00), 0);
  lv_obj_set_user_data(btn_apply, (void*)(intptr_t)preset_idx);
  lv_obj_add_event_cb(btn_apply, [](lv_event_t *e) {
    lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
    int idx = (int)(intptr_t)lv_obj_get_user_data(target);
    
    const char *name = lv_textarea_get_text(ta_name);
    // Ensure name is max 20 characters
    char limited[21];
    strncpy(limited, name, 20);
    limited[20] = '\0';
    strncpy(TCG_PRESETS[idx].name, limited, sizeof(TCG_PRESETS[idx].name) - 1);
    TCG_PRESETS[idx].name[sizeof(TCG_PRESETS[idx].name) - 1] = '\0';
    
    close_name_popup();
    show_values_popup(idx);
  }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_apply = lv_label_create(btn_apply);
  lv_label_set_text(lbl_apply, "Apply");
  lv_obj_set_style_text_font(lbl_apply, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_apply);
}

static void open_input_cb(lv_event_t *e)
{
  SharedInputState *state = (SharedInputState *)lv_event_get_user_data(e);
  lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
  int *var = (int *)lv_obj_get_user_data(btn);
  lv_obj_t *lbl = lv_obj_get_child(btn, 0);
  
  lv_obj_add_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_textarea(state->kb, NULL);
  state->current_var = nullptr;
  state->current_label = nullptr;
  
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", *var);
  lv_textarea_set_text(state->ta, buf);
  state->current_var = var;
  state->current_label = lbl;
  lv_obj_clear_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_textarea(state->kb, state->ta);
  lv_obj_move_foreground(state->kb);
}

static void shared_ta_event_cb(lv_event_t *e)
{
  SharedInputState *state = (SharedInputState *)lv_event_get_user_data(e);
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED && state->current_var && state->current_label)
  {
    int value = atoi(lv_textarea_get_text(state->ta));
    if (value >= 0 && value < 10000)
    {
      // Check if this is the temp_life variable and limit it to the actual maximum
      if (state->current_var == &temp_life)
      {
        if (value > 9999)
        {
          value = 9999;
          // Update the textarea with the corrected value
          char buf[16];
          snprintf(buf, sizeof(buf), "%d", value);
          lv_textarea_set_text(state->ta, buf);
        }
      }
      // Check if this is a step variable and limit it to the actual maximum
      else if (state->current_var == &temp_small || state->current_var == &temp_large)
      {
        if (value > 9999)
        {
          value = 9999;
          // Update the textarea with the corrected value
          char buf[16];
          snprintf(buf, sizeof(buf), "%d", value);
          lv_textarea_set_text(state->ta, buf);
        }
      }
      
      *(state->current_var) = value;
      char buf[16];
      snprintf(buf, sizeof(buf), "%d", value);
      lv_label_set_text(state->current_label, buf);
    }
  }
  else if (code == LV_EVENT_READY && state->current_var && state->current_label)
  {
    // Final validation when user presses OK
    int value = atoi(lv_textarea_get_text(state->ta));
    if (value >= 0) // Allow all positive values, but validate against maximums
    {
      // Check if this is the temp_life variable and limit it to the actual maximum
      if (state->current_var == &temp_life)
      {
        if (value > 9999)
        {
          value = 9999;
        }
      }
      // Check if this is a step variable and limit it to the actual maximum
      else if (state->current_var == &temp_small || state->current_var == &temp_large)
      {
        if (value > 9999)
        {
          value = 9999;
        }
      }
      
      *(state->current_var) = value;
      char buf[16];
      snprintf(buf, sizeof(buf), "%d", value);
      lv_label_set_text(state->current_label, buf);
    }
  }
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
  {
    lv_obj_add_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(state->kb, NULL);
    state->current_var = nullptr;
    state->current_label = nullptr;
  }
}

static void show_values_popup(int preset_idx) {
  close_values_popup();
  current_preset_idx = preset_idx;
  
  temp_life = TCG_PRESETS[preset_idx].starting_life;
  temp_small = TCG_PRESETS[preset_idx].small_step;
  temp_large = TCG_PRESETS[preset_idx].large_step;
  
  values_popup = lv_obj_create(lv_scr_act());
  lv_obj_set_size(values_popup, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(values_popup, BLACK_COLOR, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(values_popup, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(values_popup, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(values_popup, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(values_popup, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_pad_all(values_popup, 16, LV_PART_MAIN);

  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {70, 70, 70, 70, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(values_popup, col_dsc, row_dsc);
  lv_obj_set_layout(values_popup, LV_LAYOUT_GRID);

  lv_obj_t *btn_apply = lv_btn_create(values_popup);
  lv_obj_set_size(btn_apply, 100, 60);
  lv_obj_set_style_bg_color(btn_apply, lv_color_hex(0x00AA00), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_apply, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_t *lbl_apply = lv_label_create(btn_apply);
  lv_label_set_text(lbl_apply, LV_SYMBOL_OK " Apply");
  lv_obj_set_style_text_font(lbl_apply, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_apply);
  lv_obj_set_user_data(btn_apply, (void*)(intptr_t)preset_idx);
  lv_obj_add_event_cb(btn_apply, [](lv_event_t *e)
                      { 
                        lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
                        int idx = (int)(intptr_t)lv_obj_get_user_data(target);
                        
                        TCG_PRESETS[idx].starting_life = temp_life;
                        TCG_PRESETS[idx].small_step = temp_small;
                        TCG_PRESETS[idx].large_step = temp_large;
                        
                        save_preset_to_storage(idx);
                        
                        close_values_popup();
                        renderPresetEditorMenu(); }, LV_EVENT_CLICKED, NULL);

  lv_obj_t *lbl_life_max = lv_label_create(values_popup);
  lv_label_set_text(lbl_life_max, "Life Start");
  lv_obj_set_style_text_font(lbl_life_max, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_life_max, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_life_max, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *btn_max_life = lv_btn_create(values_popup);
  lv_obj_set_size(btn_max_life, 80, 40);
  lv_obj_set_style_text_font(btn_max_life, &lv_font_montserrat_24, 0);
  lv_obj_set_grid_cell(btn_max_life, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_t *lbl_max_life_val = lv_label_create(btn_max_life);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", temp_life);
  lv_label_set_text(lbl_max_life_val, buf);
  lv_obj_center(lbl_max_life_val);

  lv_obj_t *lbl_small_step = lv_label_create(values_popup);
  lv_label_set_text(lbl_small_step, "Small Step");
  lv_obj_set_style_text_font(lbl_small_step, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_small_step, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_small_step, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_t *btn_small_step = lv_btn_create(values_popup);
  lv_obj_set_size(btn_small_step, 80, 40);
  lv_obj_set_style_text_font(btn_small_step, &lv_font_montserrat_24, 0);
  lv_obj_set_grid_cell(btn_small_step, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_t *lbl_small_step_val = lv_label_create(btn_small_step);
  snprintf(buf, sizeof(buf), "%d", temp_small);
  lv_label_set_text(lbl_small_step_val, buf);
  lv_obj_center(lbl_small_step_val);

  lv_obj_t *lbl_large_step = lv_label_create(values_popup);
  lv_label_set_text(lbl_large_step, "Big Step");
  lv_obj_set_style_text_font(lbl_large_step, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_large_step, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_large_step, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

  lv_obj_t *btn_large_step = lv_btn_create(values_popup);
  lv_obj_set_size(btn_large_step, 80, 40);
  lv_obj_set_style_text_font(btn_large_step, &lv_font_montserrat_24, 0);
  lv_obj_set_grid_cell(btn_large_step, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_t *lbl_large_step_val = lv_label_create(btn_large_step);
  snprintf(buf, sizeof(buf), "%d", temp_large);
  lv_label_set_text(lbl_large_step_val, buf);
  lv_obj_center(lbl_large_step_val);

  static SharedInputState shared_input_state = {nullptr, nullptr, nullptr, nullptr};

  shared_input_state.ta = lv_textarea_create(values_popup);
  lv_textarea_set_one_line(shared_input_state.ta, true);
  lv_obj_set_style_text_font(shared_input_state.ta, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(shared_input_state.ta, lv_color_white(), 0);
  lv_obj_set_size(shared_input_state.ta, SCREEN_WIDTH - 120, 60);
  lv_obj_align(shared_input_state.ta, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_add_flag(shared_input_state.ta, LV_OBJ_FLAG_HIDDEN);

  shared_input_state.kb = lv_keyboard_create(values_popup);
  lv_keyboard_set_mode(shared_input_state.kb, LV_KEYBOARD_MODE_NUMBER);
  lv_obj_add_flag(shared_input_state.kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_text_font(shared_input_state.kb, &lv_font_montserrat_24, 0);
  lv_keyboard_set_map(shared_input_state.kb, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
  lv_keyboard_set_mode(shared_input_state.kb, LV_KEYBOARD_MODE_USER_1);
  lv_obj_set_size(shared_input_state.kb, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 150);
  lv_obj_align(shared_input_state.kb, LV_ALIGN_TOP_MID, 0, 80);
  lv_obj_clear_flag(shared_input_state.kb, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_user_data(btn_max_life, &temp_life);
  lv_obj_set_user_data(btn_small_step, &temp_small);
  lv_obj_set_user_data(btn_large_step, &temp_large);

  lv_obj_add_event_cb(btn_max_life, open_input_cb, LV_EVENT_CLICKED, &shared_input_state);
  lv_obj_add_event_cb(btn_small_step, open_input_cb, LV_EVENT_CLICKED, &shared_input_state);
  lv_obj_add_event_cb(btn_large_step, open_input_cb, LV_EVENT_CLICKED, &shared_input_state);

  lv_obj_add_event_cb(shared_input_state.ta, shared_ta_event_cb, LV_EVENT_ALL, &shared_input_state);
}

static void save_preset_to_storage(int preset_idx) {
  char key_name[32], key_life[32], key_small[32], key_large[32];
  
  snprintf(key_name, sizeof(key_name), "preset_%d_name", preset_idx);
  snprintf(key_life, sizeof(key_life), "preset_%d_life", preset_idx);
  snprintf(key_small, sizeof(key_small), "preset_%d_small", preset_idx);
  snprintf(key_large, sizeof(key_large), "preset_%d_large", preset_idx);
  
  player_store.putString(key_name, TCG_PRESETS[preset_idx].name);
  player_store.putInt(key_life, TCG_PRESETS[preset_idx].starting_life);
  player_store.putInt(key_small, TCG_PRESETS[preset_idx].small_step);
  player_store.putInt(key_large, TCG_PRESETS[preset_idx].large_step);
  
  printf("[Preset] Saved preset %d: %s (%d/%d/%d)\n", preset_idx, 
         TCG_PRESETS[preset_idx].name,
         TCG_PRESETS[preset_idx].starting_life,
         TCG_PRESETS[preset_idx].small_step,
         TCG_PRESETS[preset_idx].large_step);
}

static void load_presets_from_storage() {
  for (int i = 0; i < 10; i++) {
    char key_name[32], key_life[32], key_small[32], key_large[32];
    
    snprintf(key_name, sizeof(key_name), "preset_%d_name", i);
    snprintf(key_life, sizeof(key_life), "preset_%d_life", i);
    snprintf(key_small, sizeof(key_small), "preset_%d_small", i);
    snprintf(key_large, sizeof(key_large), "preset_%d_large", i);
    
    String stored_name = player_store.getString(key_name, "");
    if (stored_name.length() > 0) {
      strncpy(TCG_PRESETS[i].name, stored_name.c_str(), sizeof(TCG_PRESETS[i].name) - 1);
      TCG_PRESETS[i].name[sizeof(TCG_PRESETS[i].name) - 1] = '\0';
      
      TCG_PRESETS[i].starting_life = player_store.getInt(key_life, TCG_PRESETS[i].starting_life);
      TCG_PRESETS[i].small_step = player_store.getInt(key_small, TCG_PRESETS[i].small_step);
      TCG_PRESETS[i].large_step = player_store.getInt(key_large, TCG_PRESETS[i].large_step);
    }
  }
  printf("[Preset] Loaded all presets from storage\n");
}

static void close_name_popup() {
  if (name_popup) {
    lv_obj_del(name_popup);
    name_popup = nullptr;
  }
  ta_name = nullptr;
  keyboard = nullptr;
}

static void close_values_popup() {
  if (values_popup) {
    lv_obj_del(values_popup);
    values_popup = nullptr;
  }
}

void teardownPresetEditorMenu() {
  if (preset_editor_menu) {
    lv_obj_del(preset_editor_menu);
    preset_editor_menu = nullptr;
  }
  if (preset_editor_swipe_layer) {
    lv_obj_del(preset_editor_swipe_layer);
    preset_editor_swipe_layer = nullptr;
  }
  close_name_popup();
  close_values_popup();
}
