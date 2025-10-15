// ============================================
// Own Header (first!)
// ============================================
#include "gestures.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>
#include <functional>
#include <map>
#include <stdio.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"


using GestureCallback = std::function<void()>;

static std::map<GestureType, GestureCallback> gesture_callbacks;

void register_gesture_callback(GestureType gesture, GestureCallback cb)
{
  gesture_callbacks[gesture] = cb;
}

void trigger_gesture(GestureType gesture)
{
  if (gesture_callbacks.count(gesture))
  {
    printf("[Gesture] Triggering: %d\n", (int)gesture);
    gesture_callbacks[gesture]();
  }
  else
  {
    printf("[Gesture] No callback for: %d\n", (int)gesture);
  }
}

void lvgl_gesture_event_handler(lv_event_t *e)
{
  static bool swipe_detected = false;
  static bool long_press_active = false;
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);
  lv_point_t point = {0, 0};
  lv_indev_t *indev = lv_indev_get_act();
  if (indev)
  {
    lv_indev_get_point(indev, &point);
  }

  // Player Mode prüfen
  PlayerMode player_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  bool is_two_player = (player_mode == PLAYER_MODE_TWO_PLAYER);

  if (code == LV_EVENT_PRESSED)
  {
    swipe_detected = false;
    long_press_active = false;
  }
  else if (code == LV_EVENT_GESTURE)
  {
    lv_dir_t dir = lv_indev_get_gesture_dir(indev);
    
    // Swipe Down = Big Step MINUS
    if (dir == LV_DIR_BOTTOM)
    {
      if (is_two_player) {
        bool is_left = (point.x < SCREEN_WIDTH / 2);
        if (is_left) {
          trigger_gesture(GestureType::LongPressBottomLeft);  // Big step -
        } else {
          trigger_gesture(GestureType::LongPressBottomRight);
        }
      } else {
        trigger_gesture(GestureType::LongPressBottom);  // Big step -
      }
      swipe_detected = true;
    }
    // Swipe Up = Big Step PLUS
    else if (dir == LV_DIR_TOP)
    {
      if (is_two_player) {
        bool is_left = (point.x < SCREEN_WIDTH / 2);
        if (is_left) {
          trigger_gesture(GestureType::LongPressTopLeft);  // Big step +
        } else {
          trigger_gesture(GestureType::LongPressTopRight);
        }
      } else {
        trigger_gesture(GestureType::LongPressTop);  // Big step +
      }
      swipe_detected = true;
    }
  }
  else if (code == LV_EVENT_CLICKED)
  {
    if (long_press_active)
    {
      long_press_active = false;
      return;
    }
    if (!swipe_detected)
    {
      if (is_two_player) {
        // 2-Spieler Modus: Links/Rechts unterscheiden
        bool is_left = (point.x < SCREEN_WIDTH / 2);
        bool is_top = (point.y < SCREEN_HEIGHT / 2);
        
        if (is_left && is_top) {
          trigger_gesture(GestureType::TapTopLeft);  // Small +
        } else if (is_left && !is_top) {
          trigger_gesture(GestureType::TapBottomLeft);  // Small -
        } else if (!is_left && is_top) {
          trigger_gesture(GestureType::TapTopRight);  // Small +
        } else {
          trigger_gesture(GestureType::TapBottomRight);  // Small -
        }
      } else {
        // 1-Spieler Modus
        if (point.y < SCREEN_HEIGHT / 2) {
          trigger_gesture(GestureType::TapTop);  // Small +
        } else {
          trigger_gesture(GestureType::TapBottom);  // Small -
        }
      }
    }
  }
  else if (code == LV_EVENT_LONG_PRESSED)
  {
    long_press_active = true;
    
    // Long-Press Mitte = Kontextmenü
    int center_x = SCREEN_WIDTH / 2;
    int center_y = SCREEN_HEIGHT / 2;
    int dx = point.x - center_x;
    int dy = point.y - center_y;
    float dist = sqrtf(dx*dx + dy*dy);
    
    if (dist < 80) {
      trigger_gesture(GestureType::LongPressCenter);
    }
  }
}

void handle_menu_quadrant(int x, int y)
{
  if (x < LV_HOR_RES / 2 && y < LV_VER_RES / 2)
  {
    trigger_gesture(GestureType::MenuTL);
  }
  else if (x >= LV_HOR_RES / 2 && y < LV_VER_RES / 2)
  {
    trigger_gesture(GestureType::MenuTR);
  }
  else if (x < LV_HOR_RES / 2 && y >= LV_VER_RES / 2)
  {
    trigger_gesture(GestureType::MenuBL);
  }
  else
  {
    trigger_gesture(GestureType::MenuBR);
  }
}

void init_gesture_handling(lv_obj_t *root_obj)
{
  // Only listen to specific events we care about instead of LV_EVENT_ALL for better performance
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_LONG_PRESSED, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_GESTURE, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_RELEASED, NULL);
  lv_indev_set_long_press_time(lv_indev_get_act(), 500);
}

void clear_gesture_callbacks()
{
  gesture_callbacks.clear();
}