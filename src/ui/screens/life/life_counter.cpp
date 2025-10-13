// ============================================
// Own Header (first!)
// ============================================
#include "life_counter.h"

// ============================================
// System & Framework Headers
// ============================================
#include <Arduino.h>
#include <lvgl.h>
#include <math.h>
#include <stdio.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/menu/menu.h"
#include "ui/screens/tools/timer.h"

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/animation_helpers.h"
#include "ui/helpers/gestures.h"
#include "ui/helpers/event_grouper.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

// ============================================
// Hardware/Storage
// ============================================
#include <ArduinoNvs.h>


// --- Life Counter GUI State ---
lv_obj_t *life_counter_container = nullptr;
lv_obj_t *amp_button = nullptr;
static lv_obj_t *life_arc = nullptr;
static lv_obj_t *life_label = nullptr;
static lv_obj_t *grouped_change_label = nullptr;
static lv_obj_t *lbl_amp_label = nullptr;
static int amp_value = 0;
static int peak_amp = 8;

EventGrouper event_grouper(GROUPER_WINDOW, player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX), PLAYER_SINGLE);

// --- Forward Declarations ---
void update_life_label(int value);
static void arc_sweep_anim_cb(void *var, int32_t value);
static void arc_sweep_anim_ready_cb(lv_anim_t *anim);
void lvgl_gesture_event_handler(lv_event_t *e);
static lv_color_t interpolate_color(lv_color_t c1, lv_color_t c2, uint8_t t);
void increment_life(int value);
void decrement_life(int value);
void reset_life();
void queue_life_change(int player, int value);
void increment_amp();
void clear_amp();

// *** PERSISTENT LIFE STORAGE ***
void saveLifeToNVS(int life_value, int player);
int loadLifeFromNVS(int player);
void clearSavedLife();

static bool is_initializing = false;

void init_life_counter()
{
  is_initializing = true;
  teardown_life_counter();
  
  // Use default max life for initial UI setup - will be updated later
  int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  event_grouper.resetHistory(max_life);
  
  int amp_mode = player_store.getInt(KEY_AMP_MODE, PLAYER_SINGLE);

  if (!life_counter_container)
  {
    life_counter_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(life_counter_container, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(life_counter_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(life_counter_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(life_counter_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(life_counter_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(life_counter_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(life_counter_container, LV_OPA_TRANSP, 0);
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {130, 150, 60, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(life_counter_container, col_dsc, row_dsc);
    lv_obj_set_layout(life_counter_container, LV_LAYOUT_GRID);
  }
  
  if (!life_arc)
  {
    life_arc = lv_arc_create(life_counter_container);
    lv_obj_add_flag(life_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(life_arc, SCREEN_DIAMETER, SCREEN_DIAMETER);
    lv_obj_align(life_arc, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(life_arc, 0, 360);
    lv_arc_set_angles(life_arc, 270, 270);
    lv_obj_set_style_arc_color(life_arc, GREEN_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(life_arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_remove_style(life_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(life_arc, LV_OBJ_FLAG_CLICKABLE);
  }
  
  if (!life_label)
  {
    life_label = lv_label_create(life_counter_container);
    lv_obj_add_flag(life_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(life_label, "0");
    if (event_grouper.getLifeTotal() > 999)
      lv_obj_set_style_text_font(life_label, &lv_font_montserrat_48, 0);
    else
      lv_obj_set_style_text_font(life_label, &lv_font_montserrat_72, 0);
    lv_obj_set_style_text_color(life_label, lv_color_white(), 0);
    lv_obj_align(life_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_opa(life_label, LV_OPA_TRANSP, 0);
    lv_obj_set_grid_cell(life_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 1, 1);
  }
  
  if (!grouped_change_label)
  {
    grouped_change_label = lv_label_create(life_counter_container);
    lv_obj_add_flag(grouped_change_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(grouped_change_label, "0");
    lv_obj_set_style_text_font(grouped_change_label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(grouped_change_label, lv_color_white(), 0);
    lv_obj_set_grid_cell(grouped_change_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_END, 0, 1);
  }
  
  if (!amp_button)
  {
    amp_button = lv_btn_create(life_counter_container);
    lv_obj_set_size(amp_button, 110, 110);
    lv_obj_set_style_radius(amp_button, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(amp_button, AMP_START_COLOR, 0);
    static bool amp_long_press = false;
    lv_obj_add_event_cb(amp_button, [](lv_event_t *e)
                        { amp_long_press = false; }, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(amp_button, [](lv_event_t *e)
                        {
        amp_long_press = true;
        clear_amp(); }, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(amp_button, [](lv_event_t *e)
                        {
        if (!amp_long_press) increment_amp(); }, LV_EVENT_CLICKED, NULL);
    lbl_amp_label = lv_label_create(amp_button);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", amp_value);
    lv_label_set_text(lbl_amp_label, buf);
    lv_obj_set_style_text_color(lbl_amp_label, WHITE_COLOR, 0);
    lv_obj_set_style_text_font(lbl_amp_label, &lv_font_montserrat_36, 0);
    lv_obj_center(lbl_amp_label);
    lv_obj_align_to(amp_button, life_label, LV_ALIGN_RIGHT_MID, 141, 0);
    if (amp_mode)
    {
      lv_obj_set_style_opa(amp_button, LV_OPA_TRANSP, 0);
      lv_obj_clear_flag(amp_button, LV_OBJ_FLAG_HIDDEN);
      fade_in_obj(amp_button, 1500, 500, NULL);
    }
    else
    {
      lv_obj_add_flag(amp_button, LV_OBJ_FLAG_HIDDEN);
    }
  }
  
  if (life_arc)
  {
    lv_obj_clear_flag(life_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_arc_opa(life_arc, LV_OPA_COVER, LV_PART_INDICATOR);
    
    // *** AUTO-LOAD: Load saved life BEFORE animation to prevent blink ***
    int saved_life = loadLifeFromNVS(1);  // Single-player = Player 1
    event_grouper.resetHistory(saved_life);
    
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, NULL);
    lv_anim_set_exec_cb(&anim, arc_sweep_anim_cb);
    lv_anim_set_values(&anim, 0, SMOOTH_ARC_STEPS);  // Smooth animation with 1000 steps
    lv_anim_set_time(&anim, ARC_ANIMATION_DURATION);  // Faster, responsive animation
    lv_anim_set_delay(&anim, 0);
    lv_anim_set_ready_cb(&anim, arc_sweep_anim_ready_cb);
    lv_anim_start(&anim);
  }

  if (life_label)
  {
    lv_obj_clear_flag(life_label, LV_OBJ_FLAG_HIDDEN);
    fade_in_obj(life_label, 1000, 0, NULL);
  }

  uint64_t show_timer = player_store.getInt(KEY_SHOW_TIMER, 0);
  if (!timer_container && show_timer)
  {
    render_timer(life_counter_container);
    lv_obj_set_grid_cell(timer_container, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  }
}

void increment_amp()
{
  amp_value += 1;
  char buf[8];
  snprintf(buf, sizeof(buf), "+%d", amp_value);
  if (amp_button && lbl_amp_label)
  {
    lv_label_set_text(lbl_amp_label, buf);
    uint8_t t = (uint8_t)(((amp_value > peak_amp ? peak_amp : amp_value) * 255) / peak_amp);
    if (t > 255)
      t = 255;
    lv_color_t amp_color = interpolate_color(AMP_START_COLOR, AMP_END_COLOR, t);
    lv_obj_set_style_bg_color(amp_button, amp_color, 0);
  }
}

void clear_amp()
{
  amp_value = 0;
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", amp_value);
  if (amp_button && lbl_amp_label)
  {
    lv_label_set_text(lbl_amp_label, buf);
    lv_obj_set_style_bg_color(amp_button, AMP_START_COLOR, 0);
  }
}

void increment_life(step_size_t step_size)
{
  int value = (step_size == STEP_SIZE_SMALL) ? player_store.getInt(KEY_LIFE_STEP_SMALL, DEFAULT_LIFE_INCREMENT_SMALL)
                                             : player_store.getInt(KEY_LIFE_STEP_LARGE, DEFAULT_LIFE_INCREMENT_LARGE);
  queue_life_change(PLAYER_SINGLE, value);
}

void decrement_life(step_size_t step_size)
{
  int value = (step_size == STEP_SIZE_SMALL) ? player_store.getInt(KEY_LIFE_STEP_SMALL, DEFAULT_LIFE_INCREMENT_SMALL)
                                             : player_store.getInt(KEY_LIFE_STEP_LARGE, DEFAULT_LIFE_INCREMENT_LARGE);
  queue_life_change(PLAYER_SINGLE, -value);
}

void reset_life()
{
  int life_value = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  event_grouper.resetHistory(life_value);
  update_life_label(life_value);
  
  // *** AUTO-SAVE: Clear saved data when user resets ***
  clearSavedLife();
}

void teardown_life_counter()
{
  int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  clear_gesture_callbacks();
  teardown_timer();
  clear_amp();
  event_grouper.resetHistory(max_life);
  
  if (life_counter_container)
  {
    lv_obj_del(life_counter_container);
    life_counter_container = nullptr;
  }
  
  life_arc = nullptr;
  life_label = nullptr;
  grouped_change_label = nullptr;
  amp_button = nullptr;
  lbl_amp_label = nullptr;
}

static void arc_anim_cb(void *arc_obj, int32_t v)
{
  lv_arc_set_angles((lv_obj_t *)arc_obj, 0, v);
}

static void life_fadein_ready_cb(lv_anim_t *a)
{
  if (a && a->var)
  {
    fade_out_obj((lv_obj_t *)a->var, 1500, 0, [](lv_anim_t *anim)
                 {
      if (anim && anim->var) {
        lv_obj_add_flag((lv_obj_t *)anim->var, LV_OBJ_FLAG_HIDDEN);
      }
      init_life_counter(); });
  }
}

// *** SMOOTH ARC ANIMATION CALLBACK ***
// Uses high-resolution interpolation for smooth movement even with low life values
static void arc_sweep_anim_cb(void *var, int32_t v)
{
  int target_life = event_grouper.getLifeTotal(); // Use loaded life value instead of max
  // v goes from 0 to SMOOTH_ARC_STEPS (1000) for smooth interpolation
  int interpolated_life = (v * target_life) / SMOOTH_ARC_STEPS;
  if (interpolated_life > target_life)
    interpolated_life = target_life;
  update_life_label(interpolated_life);
}

static void arc_sweep_anim_ready_cb(lv_anim_t *a)
{
  is_initializing = false;
  register_gesture_callback(GestureType::TapTop, []()
                            { increment_life(step_size_t::STEP_SIZE_SMALL); });
  register_gesture_callback(GestureType::TapBottom, []()
                            { decrement_life(step_size_t::STEP_SIZE_SMALL); });
  register_gesture_callback(GestureType::LongPressTop, []()
                            { increment_life(step_size_t::STEP_SIZE_LARGE); });
  register_gesture_callback(GestureType::LongPressBottom, []()
                            { decrement_life(step_size_t::STEP_SIZE_LARGE); });
  register_gesture_callback(GestureType::SwipeDown, []()
                            {
                              if(getCurrentMenu() == MENU_NONE)
                                renderMenu(MENU_CONTEXTUAL); });
  
  // NEW: Long-Press Center for Menu
  register_gesture_callback(GestureType::LongPressCenter, []() {
      if (getCurrentMenu() == MENU_NONE) {
          renderMenu(MENU_CONTEXTUAL);
      }
  });
}

static arc_segment_t life_to_arc(int life_total)
{
  int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  arc_segment_t seg = {0};
  int arc_life = life_total;
  if (arc_life < 0)
    arc_life = 0;
  if (max_life <= 0)
    max_life = 40;
  float circumference = M_PI * SCREEN_DIAMETER;
  float gap_px = 200.0f;
  float gap_deg = (gap_px / circumference) * 360.0f;
  float arc_span = 360.0f - gap_deg;
  float arc_half = arc_span / 2.0f;
  int base_start = (int)(270.0f - arc_half + 0.5f);
  int base_end = (int)(270.0f + arc_half + 0.5f);

  lv_color_t arc_color;
  if (arc_life >= (int)(0.875 * max_life))
  {
    arc_color = GREEN_COLOR;
  }
  else if (arc_life >= (int)(0.55 * max_life))
  {
    uint8_t t = (uint8_t)(((arc_life - (int)(0.55 * max_life)) * 255) / ((int)(0.875 * max_life) - (int)(0.55 * max_life)));
    arc_color = interpolate_color(YELLOW_COLOR, GREEN_COLOR, t);
  }
  else if (arc_life >= (int)(0.25 * max_life))
  {
    uint8_t t = (uint8_t)(((arc_life - (int)(0.25 * max_life)) * 255) / ((int)(0.55 * max_life) - (int)(0.25 * max_life)));
    arc_color = interpolate_color(RED_COLOR, YELLOW_COLOR, t);
  }
  else
  {
    arc_color = RED_COLOR;
  }

  if (arc_life >= (int)(0.875 * max_life))
  {
    arc_color = GREEN_COLOR;
  }

  if (arc_life > max_life)
  {
    seg.start_angle = base_start;
    seg.end_angle = base_start + (int)arc_span;
    seg.color = arc_color;
    return seg;
  }
  if (arc_life == max_life)
  {
    seg.start_angle = base_start;
    seg.end_angle = base_end % 360;
    seg.color = arc_color;
    return seg;
  }
  if (arc_life <= 0)
  {
    seg.start_angle = base_start;
    seg.end_angle = base_start;
    seg.color = arc_color;
    return seg;
  }

  int sweep = (int)(arc_span * ((float)arc_life / (float)max_life) + 0.5f);
  int end_angle = (base_start + sweep) % 360;
  seg.start_angle = base_start;
  seg.end_angle = end_angle;
  seg.color = arc_color;
  return seg;
}

void update_life_label(int new_life_total)
{
  if (life_label != nullptr)
  {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", new_life_total);
    lv_label_set_text(life_label, buf);
  }
  if (life_arc != nullptr)
  {
    arc_segment_t seg = life_to_arc(new_life_total);
    uint16_t c16 = lv_color_to_u16(seg.color);
    uint8_t r = (c16 >> 11) & 0x1F;
    uint8_t g = (c16 >> 5) & 0x3F;
    uint8_t b = c16 & 0x1F;
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    lv_arc_set_angles(life_arc, seg.start_angle, seg.end_angle);
    lv_obj_set_style_arc_color(life_arc, seg.color, LV_PART_INDICATOR);
  }
}

static lv_color_t interpolate_color(lv_color_t c1, lv_color_t c2, uint8_t t)
{
  uint16_t c1_16 = lv_color_to_u16(c1);
  uint16_t c2_16 = lv_color_to_u16(c2);
  uint8_t r1 = (c1_16 >> 11) & 0x1F;
  uint8_t g1 = (c1_16 >> 5) & 0x3F;
  uint8_t b1 = c1_16 & 0x1F;
  uint8_t r2 = (c2_16 >> 11) & 0x1F;
  uint8_t g2 = (c2_16 >> 5) & 0x3F;
  uint8_t b2 = c2_16 & 0x1F;
  r1 = (r1 << 3) | (r1 >> 2);
  g1 = (g1 << 2) | (g1 >> 4);
  b1 = (b1 << 3) | (b1 >> 2);
  r2 = (r2 << 3) | (r2 >> 2);
  g2 = (g2 << 2) | (g2 >> 4);
  b2 = (b2 << 3) | (b2 >> 2);
  uint8_t r = (uint8_t)(r1 + ((int)r2 - (int)r1) * t / 255);
  uint8_t g = (uint8_t)(g1 + ((int)g2 - (int)g1) * t / 255);
  uint8_t b = (uint8_t)(b1 + ((int)b2 - (int)b1) * t / 255);
  return lv_color_make(r, g, b);
}

void life_counter_loop()
{
  if (event_grouper.isCommitPending())
  {
    event_grouper.loop();
  }
}

void queue_life_change(int player, int value)
{
  if (grouped_change_label != nullptr && !is_initializing)
  {
    int pending_change = event_grouper.getPendingChange() + value;
    int current_life = event_grouper.getLifeTotal();
    update_life_label((current_life + pending_change));
    char buf[8];
    if (pending_change > 0)
    {
      snprintf(buf, sizeof(buf), "+%d", pending_change);
    }
    else
    {
      snprintf(buf, sizeof(buf), "%d", pending_change);
    }
    lv_obj_set_style_text_color(grouped_change_label, pending_change >= 0 ? GREEN_COLOR : RED_COLOR, 0);
    lv_label_set_text(grouped_change_label, buf);

    lv_obj_clear_flag(grouped_change_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_opa(grouped_change_label, LV_OPA_COVER, 0);
    fade_out_obj(grouped_change_label, 100, GROUPER_WINDOW, [](lv_anim_t *fade_out_anim)
                 {
      if (fade_out_anim && fade_out_anim->var) {
        lv_obj_add_flag((lv_obj_t *)fade_out_anim->var, LV_OBJ_FLAG_HIDDEN);
      } });
    event_grouper.handleChange(player, value, get_elapsed_seconds(), [](const LifeHistoryEvent &evt) {
      // *** AUTO-SAVE: Save life to NVS whenever a change is committed ***
      saveLifeToNVS(evt.life_total, evt.player_id);
    });
  }
  else if (grouped_change_label == nullptr && !is_initializing)
  {
    // printf("[queue_life_change] grouped_change_label is NULL!\n");
  }
}

// ============================================
// PERSISTENT LIFE STORAGE FUNCTIONS
// ============================================

/**
 * @brief Save current life value to NVS for persistence across reboots
 * @param life_value Current life value to save
 * @param player Player ID (1 for single/player1, 2 for player2)
 */
void saveLifeToNVS(int life_value, int player) {
    const char* key = (player == 2) ? KEY_SAVED_LIFE_P2 : KEY_SAVED_LIFE_P1;
    
    player_store.putInt(key, life_value);
    player_store.putInt(KEY_LIFE_SAVE_VALID, 1);  // Mark as valid
    
    printf("[LifePersist] Saved P%d life: %d\n", player, life_value);
}

/**
 * @brief Load saved life value from NVS
 * @param player Player ID (1 for single/player1, 2 for player2) 
 * @return Saved life value, or default max life if no valid save exists
 */
int loadLifeFromNVS(int player) {
    // Check if we have valid saved data
    int is_valid = player_store.getInt(KEY_LIFE_SAVE_VALID, 0);
    
    if (is_valid != 1) {
        printf("[LifePersist] No saved data, using default\n");
        return player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
    }
    
    const char* key = (player == 2) ? KEY_SAVED_LIFE_P2 : KEY_SAVED_LIFE_P1;
    int default_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
    int saved_life = player_store.getInt(key, default_life);
    
    printf("[LifePersist] Loaded P%d life: %d\n", player, saved_life);
    return saved_life;
}

/**
 * @brief Clear saved life data (e.g., when user resets game)
 */
void clearSavedLife() {
    player_store.putInt(KEY_LIFE_SAVE_VALID, 0);
    player_store.putInt(KEY_SAVED_LIFE_P1, 0);  
    player_store.putInt(KEY_SAVED_LIFE_P2, 0);
    
    printf("[LifePersist] Cleared all saved life data\n");
}