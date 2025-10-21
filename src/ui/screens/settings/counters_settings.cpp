#include "counters_settings.h"
#include "config.h"
#include "data/constants.h"
#include "data/themes.h"
#include "ui/screens/menu/menu.h"
#include "ui/helpers/gestures.h"
#include "core/state_manager.h"
#include "../life/simple_counters.h"
#include <lvgl.h>

static lv_obj_t *counters_menu = nullptr;

void teardown_counters_settings() {
  if (counters_menu) {
    lv_obj_del(counters_menu);
    counters_menu = nullptr;
  }
  
  // Show all enabled counter UI again
  for (int i = 0; i < 4; i++) {
    if (SimpleCounters::counters[i].container != nullptr && SimpleCounters::counters[i].enabled) {
      lv_obj_clear_flag(SimpleCounters::counters[i].container, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void render_counters_settings() {
  // Clean up any existing menu
  teardown_counters_settings();
  
  // Hide all counter UI while in settings
  for (int i = 0; i < 4; i++) {
    if (SimpleCounters::counters[i].container != nullptr) {
      lv_obj_add_flag(SimpleCounters::counters[i].container, LV_OBJ_FLAG_HIDDEN);
    }
  }

  // Create main container - same style as timer menu
  counters_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(counters_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(counters_menu);
  lv_obj_set_style_bg_color(counters_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(counters_menu, 15, 0);
  lv_obj_set_style_border_width(counters_menu, 0, 0);
  lv_obj_set_flex_flow(counters_menu, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(counters_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scrollbar_mode(counters_menu, LV_SCROLLBAR_MODE_AUTO);

  // Grid Layout: Full width buttons (1 column, 7 rows)
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(counters_menu, col_dsc, row_dsc);
  lv_obj_set_layout(counters_menu, LV_LAYOUT_GRID);

  // Title (Row 0)
  lv_obj_t *title = lv_label_create(counters_menu);
  lv_label_set_text(title, "Counter Settings");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_grid_cell(title, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  // Back button (Row 1)
  lv_obj_t *btn_back = lv_btn_create(counters_menu);
  lv_obj_set_size(btn_back, 120, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 1, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  // Spacer at bottom for scrolling (Row 6)
  lv_obj_t *spacer = lv_obj_create(counters_menu);
  lv_obj_set_size(spacer, 280, 50);
  lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_grid_cell(spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 6, 1);

  // Counter toggle buttons (Rows 2-5)
  const char* slot_names[] = {"Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right"};
  
  for (int i = 0; i < 4; i++) {
    // Create toggle button - same style as timer toggle
    lv_obj_t *btn_toggle = lv_btn_create(counters_menu);
    lv_obj_set_size(btn_toggle, 280, 50);
    
    // Load state and set colors
    bool enabled = SimpleCounters::is_enabled(i);
    lv_obj_set_style_bg_color(btn_toggle, (enabled ? getThemeButtonColor() : lv_color_hex(0x444444)), LV_PART_MAIN);
    lv_obj_set_grid_cell(btn_toggle, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, i + 2, 1);
    
    // Button label
    lv_obj_t *lbl_toggle = lv_label_create(btn_toggle);
    char label_text[32];
    snprintf(label_text, sizeof(label_text), "%s: %s", slot_names[i], enabled ? "ON" : "OFF");
    lv_label_set_text(lbl_toggle, label_text);
    lv_obj_set_style_text_font(lbl_toggle, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lbl_toggle, (enabled ? getThemeTextColor() : lv_color_white()), 0);
    lv_obj_center(lbl_toggle);
    
    // Button event handler
    lv_obj_add_event_cb(btn_toggle, [](lv_event_t *e) {
      lv_event_code_t code = lv_event_get_code(e);
      if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        
        // Get slot from user data
        int slot = (int)(intptr_t)lv_event_get_user_data(e);
        
        // Toggle state
        bool current_state = SimpleCounters::is_enabled(slot);
        if (current_state) {
          SimpleCounters::disable_counter(slot);
        } else {
          SimpleCounters::enable_counter(slot);
        }
        
        // Update button appearance
        bool new_state = SimpleCounters::is_enabled(slot);
        lv_obj_set_style_bg_color(btn, (new_state ? getThemeButtonColor() : lv_color_hex(0x444444)), LV_PART_MAIN);
        lv_obj_set_style_text_color(label, (new_state ? getThemeTextColor() : lv_color_white()), 0);
        
        // Update label text
        const char* slot_names[] = {"Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right"};
        char new_text[32];
        snprintf(new_text, sizeof(new_text), "%s: %s", slot_names[slot], new_state ? "ON" : "OFF");
        lv_label_set_text(label, new_text);
      }
    }, LV_EVENT_ALL, (void*)(intptr_t)i);
  }
}