#include "audio_settings.h"
#include "data/constants.h"
#include "ui/screens/menu/menu.h"
#include "ui/helpers/gestures.h"
#include "hardware/audio/simple_audio.h"
#include "core/state_manager.h"
#include <lvgl.h>

static lv_obj_t *audio_menu = nullptr;

void renderAudioSettingsMenu()
{
  teardownAudioSettingsMenu();

  // Create main container - same style as settings menu
  audio_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(audio_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(audio_menu);
  lv_obj_set_style_bg_color(audio_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(audio_menu, 15, 0);
  lv_obj_set_style_border_width(audio_menu, 0, 0);
  lv_obj_set_flex_flow(audio_menu, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(audio_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scrollbar_mode(audio_menu, LV_SCROLLBAR_MODE_AUTO);

  // Grid Layout: Full width buttons (1 column, 5 rows)
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(audio_menu, col_dsc, row_dsc);
  lv_obj_set_layout(audio_menu, LV_LAYOUT_GRID);

  // Title (Row 0)
  lv_obj_t *title = lv_label_create(audio_menu);
  lv_label_set_text(title, "Audio Settings");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_grid_cell(title, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  // Back button (Row 1)
  lv_obj_t *btn_back = lv_btn_create(audio_menu);
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

  // Audio Enable/Disable Button (Row 2, Full Width)
  bool audio_enabled = simple_audio_is_enabled();
  lv_obj_t *btn_audio_toggle = lv_btn_create(audio_menu);
  lv_obj_set_size(btn_audio_toggle, 280, 50);
  lv_obj_set_style_bg_color(btn_audio_toggle, (audio_enabled ? LIGHTNING_BLUE_COLOR : lv_color_hex(0x444444)), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_audio_toggle, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_t *lbl_audio = lv_label_create(btn_audio_toggle);
  lv_label_set_text(lbl_audio, (audio_enabled ? "Audio: ON" : "Audio: OFF"));
  lv_obj_set_style_text_font(lbl_audio, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_audio);
  lv_obj_add_event_cb(btn_audio_toggle, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    bool current = simple_audio_is_enabled();
    bool new_mode = !current;
    simple_audio_set_enabled(new_mode);
    lv_label_set_text(label, (new_mode ? "Audio: ON" : "Audio: OFF"));
    lv_obj_set_style_bg_color(btn, (new_mode ? LIGHTNING_BLUE_COLOR : lv_color_hex(0x444444)), LV_PART_MAIN);
    // No click sound for audio toggle
  }, LV_EVENT_CLICKED, NULL);

  // Audio Volume Button (Row 3, Full Width) - with Percent Display
  int current_volume = simple_audio_get_volume();
  lv_obj_t *btn_audio_volume = lv_btn_create(audio_menu);
  lv_obj_set_size(btn_audio_volume, 280, 50);
  lv_obj_set_style_bg_color(btn_audio_volume, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_audio_volume, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 3, 1);
  lv_obj_t *lbl_volume = lv_label_create(btn_audio_volume);
  
  // Helper function to convert volume (0-8) to rounded percent (0%, 10%, 20%, ..., 100%)
  auto volume_to_percent = [](int vol) -> int {
    // Map 0-8 to 0%, 10%, 20%, 30%, 50%, 60%, 80%, 90%, 100%
    const int percent_map[] = {0, 10, 20, 30, 50, 60, 80, 90, 100};
    return (vol >= 0 && vol <= 8) ? percent_map[vol] : 0;
  };
  
  char volume_text[32];
  snprintf(volume_text, sizeof(volume_text), "Volume: %d%%", volume_to_percent(current_volume));
  lv_label_set_text(lbl_volume, volume_text);
  lv_obj_set_style_text_font(lbl_volume, &lv_font_montserrat_18, 0);
  lv_obj_center(lbl_volume);
  lv_obj_add_event_cb(btn_audio_volume, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    int current = simple_audio_get_volume();
    int new_volume = (current + 1) % 9; // Cycle through 0-8
    simple_audio_set_volume(new_volume);
    
    // Convert to percent
    const int percent_map[] = {0, 10, 20, 30, 50, 60, 80, 90, 100};
    int percent = (new_volume >= 0 && new_volume <= 8) ? percent_map[new_volume] : 0;
    
    char volume_text[32];
    snprintf(volume_text, sizeof(volume_text), "Volume: %d%%", percent);
    lv_label_set_text(label, volume_text);
    // Play volume preview sound
    simple_audio_play_sound(SOUND_SUCCESS);
  }, LV_EVENT_CLICKED, NULL);

  // Timer Sound Button (Row 4, Full Width)
  sound_type_t current_timer_sound = simple_audio_get_timer_sound();
  lv_obj_t *btn_timer_sound = lv_btn_create(audio_menu);
  lv_obj_set_size(btn_timer_sound, 280, 50);
  lv_obj_set_style_bg_color(btn_timer_sound, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_timer_sound, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 4, 1);
  lv_obj_t *lbl_timer_sound = lv_label_create(btn_timer_sound);
  const char* sound_names[] = {"Timer Sound: 1", "Timer Sound: 2", "Timer Sound: 3", "Timer Sound: 4"};
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
    const char* sound_names[] = {"Timer Sound: 1", "Timer Sound: 2", "Timer Sound: 3", "Timer Sound: 4"};
    lv_label_set_text(label, sound_names[next]);
    // Play preview of new sound
    simple_audio_play_sound(next);
  }, LV_EVENT_CLICKED, NULL);

  // Extra Spacer for Scrolling (Row 5)
  lv_obj_t *extra_spacer = lv_obj_create(audio_menu);
  lv_obj_set_size(extra_spacer, 10, 200);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 5, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);


  // Add swipe-to-close gesture (same as main settings menu)
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled) {
    static lv_point_t start_point;
    static bool is_swiping = false;
    
    lv_obj_add_event_cb(audio_menu, [](lv_event_t *e) {
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
}

void teardownAudioSettingsMenu()
{
  if (audio_menu)
  {
    lv_obj_del(audio_menu);
    audio_menu = nullptr;
  }
}
