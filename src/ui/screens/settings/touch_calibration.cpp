// =========================================================================
// TOUCH_CALIBRATION.CPP - CENTRE-ANCHORED CALIBRATION (Suthe's Method v2)
//
// Step 1: CENTER   - direct measurement of raw touch at the screen centre
// Step 2: SCALE_X  - edge dot, proportional-feedback loop (coarse + fine)
// Step 3: SCALE_Y  - edge dot, proportional-feedback loop (coarse + fine)
// Step 4: VERIFY   - centre dot again, informational residual-error readout
// Step 5: SUMMARY  - save and exit
//
// See docs/Suthes_Method_Touch_Calibration.md for the full write-up of why
// scaling is anchored at the measured centre instead of the raw origin.
// =========================================================================

#include "touch_calibration.h"

#include <lvgl.h>
#include <ArduinoNvs.h>
#include <math.h>

#include "config.h"
#include "core/state_manager.h"
#include "hardware/display/LVGL_Driver.h"
#include "hardware/touch/Touch_CST816.h"
#include "hardware/system/power_management.h"
#include "ui/screens/menu/menu.h"
#include "data/constants.h"

extern struct CST816_Touch touch_data;

#define KEY_TOUCH_CAL_VALID "touch_valid"
#define KEY_TOUCH_PENDING_CONFIRM "touch_pending"

#define TOLERANCE_COARSE 5.0f
#define TOLERANCE_FINE 2.0f
#define CORRECTION_SPEED_COARSE 0.3f
#define CORRECTION_SPEED_FINE 0.1f

// Screen-space anchor for the centre-anchored scaling model. See the "New
// step sequence" section of the design doc for the algebra.
#define CENTER_X (SCREEN_WIDTH / 2)
#define CENTER_Y (SCREEN_HEIGHT / 2)

typedef enum
{
  CAL_STEP_CENTER,
  CAL_STEP_SCALE_X,
  CAL_STEP_SCALE_Y,
  CAL_STEP_VERIFY,

  CAL_STEP_SUMMARY,
  CAL_STEP_FINISHED
} calibration_step_t;

typedef enum
{
  CAL_PASS_COARSE,
  CAL_PASS_FINE
} calibration_pass_t;

static lv_obj_t *cal_screen = nullptr;
static lv_timer_t *cal_timer = nullptr;
static calibration_step_t current_step;
static calibration_pass_t current_pass;
static float temp_cal_matrix[7];
static lv_obj_t *info_label = nullptr;
static lv_obj_t *target_visual = nullptr;
static bool step_is_locked = false;
static int lock_frame_counter = 0;

// Centre-anchored model parameters: cal_x = CENTER_X + sx * (raw_x - rx0),
// cal_y = CENTER_Y + sy * (raw_y - ry0). rx0/ry0 are the raw touch
// coordinates measured at the screen centre (CAL_STEP_CENTER).
static float current_scale_x = 1.0f;
static float current_scale_y = 1.0f;
static float current_rx0 = 0.0f;
static float current_ry0 = 0.0f;

// Moving-average window for the SCALE_X / SCALE_Y proportional-feedback
// loop. TOLERANCE_FINE (2px) is below what a finger reproducibly hits, so
// noise is filtered by averaging raw samples rather than trusting a single
// 50ms reading.
static float scale_sample_buf[CALIBRATION_SAMPLE_FRAMES];
static int scale_sample_count = 0;
static int scale_sample_idx = 0;

static lv_obj_t *confirmation_screen = nullptr;
static lv_obj_t *countdown_label = nullptr;
static uint32_t confirmation_start_time = 0;
static lv_timer_t *confirmation_timer = nullptr;

static const char *TXT_CENTER = "Hold red dot\nin the centre";
static const char *TXT_SCALE_X_COARSE = "Hold red dot\n(Coarse)";
static const char *TXT_SCALE_X_FINE = "Hold red dot\n(Fine)";
static const char *TXT_SCALE_Y_COARSE = "Hold red dot\n(Coarse)";
static const char *TXT_SCALE_Y_FINE = "Hold red dot\n(Fine)";
static const char *TXT_VERIFY = "Touch the centre\nto verify";

static const char *TXT_LOCKED = "OK!\nRelease finger";
static const char *TXT_SUMMARY = "Calibration complete!\n\nTouch to exit";

static void setup_ui_for_step(calibration_step_t step);
static void process_calibration_logic(lv_timer_t *timer);
static void process_measurement_step(calibration_step_t step);
static void save_and_exit_calibration();
static void advance_to_next_step();
static void rebuild_matrix();
static void reset_scale_samples();
static float push_scale_sample(int16_t raw_value);
static lv_obj_t *create_target_dot();

void resetTouchCalibrationToDefaults()
{
  printf("[TouchCal] Resetting to factory defaults\n");
  const float *defaults = getDefaultCalibrationMatrix();
  NVS.setInt(KEY_TOUCH_CAL_VALID, (int32_t)1);
  NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)0);
  for (int i = 0; i < 7; i++)
  {
    char key[20];
    snprintf(key, sizeof(key), "touch_cal_%d", i);
    NVS.setFloat(key, defaults[i]);
  }
  updateTouchCalibrationMatrix(defaults);
}

void loadTouchCalibrationFromNVS()
{
  if (NVS.getInt(KEY_TOUCH_CAL_VALID, 0) == 1)
  {
    float matrix[7];
    for (int i = 0; i < 7; i++)
    {
      char key[20];
      snprintf(key, sizeof(key), "touch_cal_%d", i);
      matrix[i] = NVS.getFloat(key, 0.0f);
    }
    updateTouchCalibrationMatrix(matrix);
    printf("[TouchCal] Loaded from NVS\n");
  }
  else
  {
    resetTouchCalibrationToDefaults();
  }
}

bool needsTouchCalibrationConfirmation()
{
  return NVS.getInt(KEY_TOUCH_PENDING_CONFIRM, 0) == 1;
}

void confirmTouchCalibration()
{
  NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)0);
  printf("[TouchCal] Confirmed\n");
}

// Rebuild the 7-parameter runtime matrix from the centre-anchored model
// (current_scale_x, current_scale_y, current_rx0, current_ry0). Call this
// whenever any of those four values change -- it is the only place that
// writes temp_cal_matrix, so offset and scale can never drift out of sync.
//
// cal_x = CENTER_X + sx * (raw_x - rx0) = sx*raw_x + (CENTER_X - sx*rx0)
// cal_y = CENTER_Y + sy * (raw_y - ry0) = sy*raw_y + (CENTER_Y - sy*ry0)
static void rebuild_matrix()
{
  temp_cal_matrix[0] = CENTER_X - current_scale_x * current_rx0; // C
  temp_cal_matrix[1] = current_scale_x;                          // A
  temp_cal_matrix[2] = 0.0f;                                     // B (no shear/rotation)
  temp_cal_matrix[3] = CENTER_Y - current_scale_y * current_ry0; // F
  temp_cal_matrix[4] = 0.0f;                                     // D (no shear/rotation)
  temp_cal_matrix[5] = current_scale_y;                          // E
  temp_cal_matrix[6] = 1.0f;                                     // divisor
}

static void reset_scale_samples()
{
  scale_sample_count = 0;
  scale_sample_idx = 0;
}

static float push_scale_sample(int16_t raw_value)
{
  scale_sample_buf[scale_sample_idx] = (float)raw_value;
  scale_sample_idx = (scale_sample_idx + 1) % CALIBRATION_SAMPLE_FRAMES;
  if (scale_sample_count < CALIBRATION_SAMPLE_FRAMES)
  {
    scale_sample_count++;
  }

  float sum = 0.0f;
  for (int i = 0; i < scale_sample_count; i++)
  {
    sum += scale_sample_buf[i];
  }
  return sum / (float)scale_sample_count;
}

static lv_obj_t *create_target_dot()
{
  lv_obj_t *dot = lv_obj_create(cal_screen);
  lv_obj_remove_style_all(dot);
  lv_obj_set_style_bg_color(dot, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_size(dot, 30, 30);
  lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
  return dot;
}

void renderTouchCalibrationConfirmation()
{
  teardownTouchCalibrationConfirmation();

  confirmation_screen = lv_obj_create(lv_scr_act());
  lv_obj_set_size(confirmation_screen, 320, 240);
  lv_obj_align(confirmation_screen, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(confirmation_screen, lv_color_hex(0x2C2C2C), 0);
  lv_obj_set_style_border_width(confirmation_screen, 2, 0);
  lv_obj_set_style_border_color(confirmation_screen, lv_color_hex(0x00FF00), 0);
  lv_obj_move_foreground(confirmation_screen);

  lv_obj_t *title = lv_label_create(confirmation_screen);
  lv_label_set_text(title, "Touch Calibration");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  lv_obj_t *msg = lv_label_create(confirmation_screen);
  lv_label_set_text(msg, "New calibration applied!\nKeep these settings?");
  lv_obj_set_style_text_color(msg, lv_color_white(), 0);
  lv_obj_set_style_text_font(msg, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(msg, 300);
  lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 45);

  countdown_label = lv_label_create(confirmation_screen);
  lv_label_set_text(countdown_label, "Auto-revert in: 10s");
  lv_obj_set_style_text_color(countdown_label, lv_color_hex(0xFFAA00), 0);
  lv_obj_set_style_text_font(countdown_label, &lv_font_montserrat_14, 0);
  lv_obj_set_width(countdown_label, 300);
  lv_obj_set_style_text_align(countdown_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(countdown_label, LV_ALIGN_CENTER, 0, 30);

  lv_obj_t *keep_btn = lv_btn_create(confirmation_screen);
  lv_obj_set_size(keep_btn, 120, 50);
  lv_obj_align(keep_btn, LV_ALIGN_BOTTOM_LEFT, 20, -10);
  lv_obj_set_style_bg_color(keep_btn, lv_color_hex(0x00AA00), 0);
  lv_obj_t *keep_lbl = lv_label_create(keep_btn);
  lv_label_set_text(keep_lbl, "Keep");
  lv_obj_set_style_text_color(keep_lbl, lv_color_white(), 0);
  lv_obj_center(keep_lbl);
  lv_obj_add_event_cb(keep_btn, [](lv_event_t *e)
                      {
        confirmTouchCalibration();
        teardownTouchCalibrationConfirmation();
        renderMenu(MENU_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  lv_obj_t *revert_btn = lv_btn_create(confirmation_screen);
  lv_obj_set_size(revert_btn, 120, 50);
  lv_obj_align(revert_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
  lv_obj_set_style_bg_color(revert_btn, lv_color_hex(0xAA0000), 0);
  lv_obj_t *revert_lbl = lv_label_create(revert_btn);
  lv_label_set_text(revert_lbl, "Revert");
  lv_obj_set_style_text_color(revert_lbl, lv_color_white(), 0);
  lv_obj_center(revert_lbl);
  lv_obj_add_event_cb(revert_btn, [](lv_event_t *e)
                      {
        resetTouchCalibrationToDefaults();
        teardownTouchCalibrationConfirmation();
        renderMenu(MENU_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  confirmation_start_time = millis();
  confirmation_timer = lv_timer_create([](lv_timer_t *timer)
                                       {
        long remaining_ms = (long)CALIBRATION_TIMEOUT_MS - (long)(millis() - confirmation_start_time);
        if (remaining_ms > 0) {
            char txt[50];
            snprintf(txt, sizeof(txt), "Auto-revert in: %lds", (remaining_ms / 1000) + 1);
            if (countdown_label) lv_label_set_text(countdown_label, txt);
        } else {
            resetTouchCalibrationToDefaults();
            teardownTouchCalibrationConfirmation();
            renderMenu(MENU_SETTINGS);
        } }, 500, NULL);
}

void teardownTouchCalibrationConfirmation()
{
  power_suspend(false);
  if (confirmation_timer)
  {
    lv_timer_del(confirmation_timer);
    confirmation_timer = nullptr;
  }
  if (confirmation_screen)
  {
    lv_obj_del(confirmation_screen);
    confirmation_screen = nullptr;
  }
}

void renderTouchCalibrationScreen()
{
  printf("[TouchCal] Starting centre-anchored calibration\n");

  power_suspend(true);
  teardownTouchCalibrationScreen();
  vTaskDelay(pdMS_TO_TICKS(50));

  cal_screen = lv_obj_create(lv_scr_act());
  lv_obj_remove_style_all(cal_screen);
  lv_obj_set_style_bg_color(cal_screen, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(cal_screen, LV_OPA_COVER, 0);
  lv_obj_set_size(cal_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_center(cal_screen);

  const float *defaults = getDefaultCalibrationMatrix();

  // Seed the centre-anchored model from the currently stored (possibly
  // origin-anchored) matrix. This is only a starting point: CAL_STEP_CENTER
  // re-measures rx0/ry0 directly, and CAL_STEP_SCALE_X/Y re-measure scale,
  // so any shear in the stored matrix (not representable in this model) is
  // simply discarded rather than approximated.
  current_scale_x = defaults[1];
  current_scale_y = defaults[5];
  current_rx0 = (current_scale_x != 0.0f) ? (CENTER_X - defaults[0]) / current_scale_x : (float)CENTER_X;
  current_ry0 = (current_scale_y != 0.0f) ? (CENTER_Y - defaults[3]) / current_scale_y : (float)CENTER_Y;
  rebuild_matrix();

  printf("[TouchCal] Init: scale=(%.3f,%.3f) rx0/ry0=(%.1f,%.1f)\n",
         current_scale_x, current_scale_y, current_rx0, current_ry0);

  current_step = CAL_STEP_CENTER;
  current_pass = CAL_PASS_COARSE;
  step_is_locked = false;
  lock_frame_counter = 0;
  reset_scale_samples();

  setup_ui_for_step(CAL_STEP_CENTER);
  cal_timer = lv_timer_create(process_calibration_logic, 50, NULL);
}

void teardownTouchCalibrationScreen()
{
  power_suspend(false);
  if (cal_timer)
  {
    lv_timer_del(cal_timer);
    cal_timer = nullptr;
  }
  if (cal_screen)
  {
    lv_obj_del(cal_screen);
    cal_screen = nullptr;
  }
  info_label = nullptr;
  target_visual = nullptr;
}

static void advance_to_next_step()
{
  printf("[TouchCal] Advancing from step %d (pass %d)\n", current_step, current_pass);

  // SCALE_X / SCALE_Y each get a Coarse -> Fine pass before moving on.
  bool has_passes = (current_step == CAL_STEP_SCALE_X || current_step == CAL_STEP_SCALE_Y);
  if (has_passes && current_pass == CAL_PASS_COARSE)
  {
    current_pass = CAL_PASS_FINE;
    step_is_locked = false;
    lock_frame_counter = 0;
    setup_ui_for_step(current_step);
    return;
  }

  step_is_locked = false;
  lock_frame_counter = 0;
  current_pass = CAL_PASS_COARSE;
  current_step = (calibration_step_t)(current_step + 1);

  if (current_step == CAL_STEP_FINISHED)
  {
    save_and_exit_calibration();
  }
  else
  {
    setup_ui_for_step(current_step);
  }
}

static void setup_ui_for_step(calibration_step_t step)
{
  printf("[TouchCal] UI for step %d (pass %d)\n", step, current_pass);

  lv_obj_clean(cal_screen);
  target_visual = nullptr;
  info_label = nullptr;

  if (step == CAL_STEP_SUMMARY)
  {
    info_label = lv_label_create(cal_screen);
    lv_obj_set_style_text_color(info_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(info_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(info_label, SCREEN_WIDTH - 40);

    char summary[300];
    snprintf(summary, sizeof(summary),
             "%s\n\n"
             "Centre raw: (%.1f, %.1f)\n"
             "Scale: (%.3f, %.3f)",
             TXT_SUMMARY,
             current_rx0, current_ry0,
             current_scale_x, current_scale_y);

    lv_label_set_text(info_label, summary);
    lv_obj_center(info_label);
    return;
  }

  info_label = lv_label_create(cal_screen);
  lv_obj_set_style_text_color(info_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(info_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(info_label, SCREEN_WIDTH - 40);
  lv_obj_align(info_label, LV_ALIGN_TOP_MID, 0, 20);

  switch (step)
  {
  case CAL_STEP_CENTER:
  {
    lv_label_set_text_static(info_label, TXT_CENTER);
    target_visual = create_target_dot();
    lv_obj_align(target_visual, LV_ALIGN_CENTER, 0, 0);
    break;
  }

  case CAL_STEP_SCALE_X:
  {
    reset_scale_samples();
    lv_label_set_text_static(info_label,
                             current_pass == CAL_PASS_COARSE ? TXT_SCALE_X_COARSE : TXT_SCALE_X_FINE);
    target_visual = create_target_dot();
    lv_obj_align(target_visual, LV_ALIGN_RIGHT_MID, -20, 0);
    break;
  }

  case CAL_STEP_SCALE_Y:
  {
    reset_scale_samples();
    lv_label_set_text_static(info_label,
                             current_pass == CAL_PASS_COARSE ? TXT_SCALE_Y_COARSE : TXT_SCALE_Y_FINE);
    target_visual = create_target_dot();
    lv_obj_align(target_visual, LV_ALIGN_BOTTOM_MID, 0, -20);
    break;
  }

  case CAL_STEP_VERIFY:
  {
    lv_label_set_text_static(info_label, TXT_VERIFY);
    target_visual = create_target_dot();
    lv_obj_align(target_visual, LV_ALIGN_CENTER, 0, 0);
    break;
  }

  default:
    break;
  }

  lv_obj_update_layout(cal_screen);
}

// CAL_STEP_CENTER and CAL_STEP_VERIFY are direct measurements, not
// proportional-feedback loops: hold the dot, discard the first few frames
// (finger settling), average a fixed number of frames, then act on the
// result. No error correction loop is needed for either.
static void process_measurement_step(calibration_step_t step)
{
  static int settle_counter = 0;
  static float sum_x = 0.0f;
  static float sum_y = 0.0f;
  static int sample_n = 0;
  static uint32_t release_time = 0;

  if (step_is_locked)
  {
    // Wait for a clean release (300ms with no touch) before advancing.
    Touch_Read_Data();
    int pts = touch_data.points;
    touch_data.points = 0;

    if (pts == 0)
    {
      if (release_time == 0)
      {
        release_time = millis();
      }
      else if (millis() - release_time > 300)
      {
        release_time = 0;
        advance_to_next_step();
      }
    }
    else
    {
      release_time = 0;
    }
    return;
  }

  Touch_Read_Data();
  if (touch_data.points == 0)
  {
    // Not touching yet, or the finger lifted before sampling finished.
    // Discard whatever was collected and wait for a fresh, continuous hold.
    settle_counter = 0;
    sum_x = 0.0f;
    sum_y = 0.0f;
    sample_n = 0;
    return;
  }

  if (settle_counter < CALIBRATION_SETTLE_FRAMES)
  {
    settle_counter++;
    return;
  }

  sum_x += touch_data.x;
  sum_y += touch_data.y;
  sample_n++;

  if (sample_n < CALIBRATION_SAMPLE_FRAMES)
  {
    return;
  }

  float avg_x = sum_x / (float)sample_n;
  float avg_y = sum_y / (float)sample_n;

  if (step == CAL_STEP_CENTER)
  {
    current_rx0 = avg_x;
    current_ry0 = avg_y;
    rebuild_matrix();
    printf("[CAL-CENTER] raw_mean=(%.1f,%.1f) -> rx0/ry0 set, matrix rebuilt\n", avg_x, avg_y);

    lv_label_set_text_static(info_label, TXT_LOCKED);
  }
  else // CAL_STEP_VERIFY
  {
    float cal_x = current_scale_x * (avg_x - current_rx0) + CENTER_X;
    float cal_y = current_scale_y * (avg_y - current_ry0) + CENTER_Y;
    float err_x = cal_x - CENTER_X;
    float err_y = cal_y - CENTER_Y;
    float err_px = sqrtf(err_x * err_x + err_y * err_y);

    printf("[CAL-VERIFY] raw_mean=(%.1f,%.1f) cal=(%.1f,%.1f) error=%.2fpx\n",
           avg_x, avg_y, cal_x, cal_y, err_px);

    char txt[64];
    snprintf(txt, sizeof(txt), "Error: %.1f px\nRelease finger", err_px);
    lv_label_set_text(info_label, txt);
  }

  step_is_locked = true;
  settle_counter = 0;
  sum_x = 0.0f;
  sum_y = 0.0f;
  sample_n = 0;
}

static void process_calibration_logic(lv_timer_t *timer)
{
  static uint32_t last_touch_time = 0;
  static uint32_t summary_touch_time = 0;

  if (current_step == CAL_STEP_CENTER || current_step == CAL_STEP_VERIFY)
  {
    process_measurement_step(current_step);
    return;
  }

  if (current_step == CAL_STEP_SUMMARY)
  {
    if (step_is_locked)
    {
      Touch_Read_Data();
      int pts = touch_data.points;
      touch_data.points = 0;

      if (pts == 0)
      {
        if (summary_touch_time == 0)
        {
          summary_touch_time = millis();
        }
        else if (millis() - summary_touch_time > 300)
        {
          summary_touch_time = 0;
          advance_to_next_step();
        }
      }
      else
      {
        summary_touch_time = 0;
      }
      return;
    }

    Touch_Read_Data();
    if (touch_data.points > 0)
    {
      step_is_locked = true;
    }
    return;
  }

  // From here on: CAL_STEP_SCALE_X / CAL_STEP_SCALE_Y, proportional-feedback
  // loop against an edge target, same as before but without the Round 2/3
  // duplication -- the Coarse/Fine pass is enough now that offset and scale
  // are decoupled.
  if (step_is_locked)
  {
    Touch_Read_Data();
    int pts = touch_data.points;
    touch_data.points = 0;

    if (pts == 0)
    {
      if (last_touch_time == 0)
      {
        last_touch_time = millis();
      }
      else if (millis() - last_touch_time > 300)
      {
        last_touch_time = 0;
        advance_to_next_step();
      }
    }
    else
    {
      last_touch_time = 0;
    }
    return;
  }

  Touch_Read_Data();
  if (touch_data.points == 0)
  {
    lock_frame_counter = 0;
    reset_scale_samples();
    return;
  }

  int16_t x_raw = touch_data.x;
  int16_t y_raw = touch_data.y;

  float tolerance = (current_pass == CAL_PASS_COARSE) ? TOLERANCE_COARSE : TOLERANCE_FINE;
  float correction_speed = (current_pass == CAL_PASS_COARSE) ? CORRECTION_SPEED_COARSE : CORRECTION_SPEED_FINE;

  static int debug_ctr = 0;
  bool do_debug = (debug_ctr++ % 20 == 0);

  switch (current_step)
  {
  case CAL_STEP_SCALE_X:
  {
    float avg_raw_x = push_scale_sample(x_raw);
    int target_x = SCREEN_WIDTH - 20;
    float cal_x = current_scale_x * (avg_raw_x - current_rx0) + CENTER_X;
    float error_x = cal_x - target_x;

    if (do_debug)
    {
      printf("[CAL-SCALE_X] raw=%d avg_raw=%.1f cal=%.1f target=%d error=%.1f scale=%.3f\n",
             x_raw, avg_raw_x, cal_x, target_x, error_x, current_scale_x);
    }

    if (fabs(error_x) > tolerance)
    {
      float scale_correction = 1.0f - (error_x / (float)target_x) * correction_speed;
      current_scale_x *= scale_correction;

      if (current_scale_x < 0.5f)
        current_scale_x = 0.5f;
      if (current_scale_x > 2.0f)
        current_scale_x = 2.0f;

      rebuild_matrix();
      lock_frame_counter = 0;
    }
    else
    {
      lock_frame_counter++;
      if (lock_frame_counter >= CALIBRATION_LOCK_FRAMES)
      {
        printf("[CAL-SCALE_X] LOCKED at %.3f\n", current_scale_x);
        step_is_locked = true;
        if (current_pass == CAL_PASS_FINE)
        {
          lv_label_set_text_static(info_label, TXT_LOCKED);
        }
      }
    }
    break;
  }

  case CAL_STEP_SCALE_Y:
  {
    float avg_raw_y = push_scale_sample(y_raw);
    int target_y = SCREEN_HEIGHT - 20;
    float cal_y = current_scale_y * (avg_raw_y - current_ry0) + CENTER_Y;
    float error_y = cal_y - target_y;

    if (do_debug)
    {
      printf("[CAL-SCALE_Y] raw=%d avg_raw=%.1f cal=%.1f target=%d error=%.1f scale=%.3f\n",
             y_raw, avg_raw_y, cal_y, target_y, error_y, current_scale_y);
    }

    if (fabs(error_y) > tolerance)
    {
      float scale_correction = 1.0f - (error_y / (float)target_y) * correction_speed;
      current_scale_y *= scale_correction;

      if (current_scale_y < 0.5f)
        current_scale_y = 0.5f;
      if (current_scale_y > 2.0f)
        current_scale_y = 2.0f;

      rebuild_matrix();
      lock_frame_counter = 0;
    }
    else
    {
      lock_frame_counter++;
      if (lock_frame_counter >= CALIBRATION_LOCK_FRAMES)
      {
        printf("[CAL-SCALE_Y] LOCKED at %.3f\n", current_scale_y);
        step_is_locked = true;
        if (current_pass == CAL_PASS_FINE)
        {
          lv_label_set_text_static(info_label, TXT_LOCKED);
        }
      }
    }
    break;
  }

  default:
    break;
  }
}

static void save_and_exit_calibration()
{
  printf("[TouchCal] Saving final matrix...\n");

  NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)1);
  NVS.setInt(KEY_TOUCH_CAL_VALID, (int32_t)1);

  for (int i = 0; i < 7; i++)
  {
    char key[20];
    snprintf(key, sizeof(key), "touch_cal_%d", i);
    NVS.setFloat(key, temp_cal_matrix[i]);
  }

  updateTouchCalibrationMatrix(temp_cal_matrix);

  printf("[TouchCal] DONE! Centre-anchored calibration complete!\n");
  printf("  Matrix: [%.2f, %.3f, %.3f, %.2f, %.3f, %.3f, %.3f]\n",
         temp_cal_matrix[0], temp_cal_matrix[1], temp_cal_matrix[2],
         temp_cal_matrix[3], temp_cal_matrix[4], temp_cal_matrix[5], temp_cal_matrix[6]);

  renderMenu(MENU_SETTINGS);
}
