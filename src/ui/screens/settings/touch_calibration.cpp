// =========================================================================
// TOUCH_CALIBRATION.CPP
// Interactive Touch Calibration System
// 
// Since I couldn't find any similar approach in existing implementations,
// I'm taking the liberty of calling this "Suthe's Method".
// 
// Features:
// - 2-Pass System: Coarse → Fine adjustment (seamless transition)
// - Zone-based interactive calibration
// - Automatic parameter adjustment based on touch position
// - Optional rotation calibration
// - Fallback confirmation system with auto-revert
// =========================================================================

#include "touch_calibration.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>
#include <ArduinoNvs.h>
#include <math.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// Hardware
// ============================================
#include "hardware/display/lvgl_driver.h"
#include "hardware/touch/touch_cst816.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/menu/menu.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

// ============================================
// EXTERNAL TOUCH DATA
// ============================================
extern struct CST816_Touch touch_data;

// ============================================
// NVS STORAGE KEYS
// ============================================
#define KEY_TOUCH_CAL_VALID "touch_valid"
#define KEY_TOUCH_PENDING_CONFIRM "touch_pending"

// ============================================
// CONFIRMATION TIMEOUT
// ============================================
#define CONFIRMATION_TIMEOUT_MS 10000

// ============================================
// CALIBRATION PARAMETERS
// ============================================
// PASS 1: Coarse adjustment (fast approach to target zone)
#define OFFSET_STEP_COARSE 2.0f       // 2 pixels per frame (safe and responsive)
#define SCALE_STEP_COARSE 0.015f      // 1.5% per frame
#define ROTATION_STEP_COARSE 0.02f    // ~1.15 degrees per frame

// PASS 2: Fine adjustment (precision tuning)
#define OFFSET_STEP_FINE 0.25f        // 0.25 pixels per frame (high precision)
#define SCALE_STEP_FINE 0.001f        // 0.1% per frame (high precision)
#define ROTATION_STEP_FINE 0.002f     // ~0.11 degrees per frame (high precision)

#define LOCK_FRAMES 10                // Number of consecutive frames in target zone to lock

// Refinement: Backtrack distance when switching from coarse to fine
#define REFINEMENT_OFFSET_BACKTRACK 5.0f    // 5 pixels back from coarse target
#define REFINEMENT_SCALE_BACKTRACK 0.03f    // 3% back from coarse target
#define REFINEMENT_ROTATION_BACKTRACK 0.04f // ~2.3 degrees back from coarse target

// ============================================
// CALIBRATION STATE MACHINE
// ============================================
typedef enum {
    CAL_STEP_OFFSET_X,
    CAL_STEP_OFFSET_Y,
    CAL_STEP_ROTATION,
    CAL_STEP_SCALE_X,
    CAL_STEP_SCALE_Y,
    CAL_STEP_SUMMARY,
    CAL_STEP_FINISHED
} calibration_step_t;

typedef enum {
    CAL_PASS_COARSE,
    CAL_PASS_FINE
} calibration_pass_t;

// ============================================
// STATIC VARIABLES - CALIBRATION STATE
// ============================================
static lv_obj_t* cal_screen = nullptr;
static lv_timer_t* cal_timer = nullptr;
static calibration_step_t current_step;
static calibration_pass_t current_pass;
static float temp_cal_matrix[7];
static lv_obj_t* info_label = nullptr;
static lv_obj_t* target_visual = nullptr;
static lv_obj_t* zone1 = nullptr;
static lv_obj_t* zone2 = nullptr;
static lv_obj_t* zone_target = nullptr;
static bool step_is_locked = false;
static int lock_frame_counter = 0;

// Dynamic step sizes (adjusted based on current pass)
static float current_offset_step = OFFSET_STEP_COARSE;
static float current_scale_step = SCALE_STEP_COARSE;
static float current_rotation_step = ROTATION_STEP_COARSE;

// Rotation angle (in radians, for optional rotation step)
static float current_theta = 0.0f;

// ============================================
// STATIC VARIABLES - CONFIRMATION SCREEN
// ============================================
static lv_obj_t* confirmation_screen = nullptr;
static lv_obj_t* countdown_label = nullptr;
static uint32_t confirmation_start_time = 0;
static lv_timer_t* confirmation_timer = nullptr;

// ============================================
// UI TEXT STRINGS
// ============================================
static const char* TXT_OFFSET_X_COARSE = "Hold red line\n(Fast)";
static const char* TXT_OFFSET_X_FINE = "Hold red line\n(Precise)";
static const char* TXT_OFFSET_Y_COARSE = "Great!\nAgain (Fast)";
static const char* TXT_OFFSET_Y_FINE = "Great!\nAgain (Precise)";
static const char* TXT_ROTATION_COARSE = "Hold red dot\non line (Fast)";
static const char* TXT_ROTATION_FINE = "Hold red dot\non line (Precise)";
static const char* TXT_SCALE_X_COARSE = "Almost done!\nTouch right (Fast)";
static const char* TXT_SCALE_X_FINE = "Almost done!\nTouch right (Precise)";
static const char* TXT_SCALE_Y_COARSE = "Last step!\nTouch bottom (Fast)";
static const char* TXT_SCALE_Y_FINE = "Last step!\nTouch bottom (Precise)";
static const char* TXT_LOCKED = "OK!\nRelease finger";
static const char* TXT_SUMMARY = "Calibration complete!\n\nTouch to exit";

// ============================================
// FUNCTION PROTOTYPES
// ============================================
static void setup_ui_for_step(calibration_step_t step);
static void process_calibration_logic(lv_timer_t* timer);
static void save_and_exit_calibration();
static void advance_to_next_step();
static void update_rotation_matrix(float theta);

// ============================================
// NVS STORAGE FUNCTIONS
// ============================================

void resetTouchCalibrationToDefaults() {
    printf("[TouchCal] Resetting to factory defaults\n");
    
    const float* defaults = getDefaultCalibrationMatrix();
    
    NVS.setInt(KEY_TOUCH_CAL_VALID, (int32_t)1);
    NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)0);
    
    for (int i = 0; i < 7; i++) {
        char key[20];
        snprintf(key, sizeof(key), "touch_cal_%d", i);
        NVS.setFloat(key, defaults[i]);
    }
    
    updateTouchCalibrationMatrix(defaults);
    printf("[TouchCal] Factory defaults applied\n");
}

void loadTouchCalibrationFromNVS() {
    if (NVS.getInt(KEY_TOUCH_CAL_VALID, 0) == 1) {
        float matrix[7];
        for (int i = 0; i < 7; i++) {
            char key[20];
            snprintf(key, sizeof(key), "touch_cal_%d", i);
            matrix[i] = NVS.getFloat(key, 0.0f);
        }
        updateTouchCalibrationMatrix(matrix);
        printf("[TouchCal] Loaded from NVS\n");
    } else {
        printf("[TouchCal] No valid calibration, using defaults\n");
        resetTouchCalibrationToDefaults();
    }
}

bool needsTouchCalibrationConfirmation() {
    bool needs_confirm = NVS.getInt(KEY_TOUCH_PENDING_CONFIRM, 0) == 1;
    printf("[TouchCal] needsTouchCalibrationConfirmation() = %d\n", needs_confirm);
    return needs_confirm;
}

void confirmTouchCalibration() {
    NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)0);
    printf("[TouchCal] Calibration confirmed and saved permanently\n");
}

// ============================================
// ROTATION MATRIX UPDATE
// ============================================

static void update_rotation_matrix(float theta) {
    temp_cal_matrix[1] = cosf(theta);
    temp_cal_matrix[2] = sinf(theta);
    temp_cal_matrix[4] = -sinf(theta);
    temp_cal_matrix[5] = cosf(theta);
}

// ============================================
// CONFIRMATION SCREEN FUNCTIONS (MIT UI-FIXES!)
// ============================================

void renderTouchCalibrationConfirmation() {
    teardownTouchCalibrationConfirmation();
    
    confirmation_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(confirmation_screen, 320, 240);
    lv_obj_align(confirmation_screen, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(confirmation_screen, lv_color_hex(0x2C2C2C), 0);
    lv_obj_set_style_border_width(confirmation_screen, 2, 0);
    lv_obj_set_style_border_color(confirmation_screen, lv_color_hex(0x00FF00), 0);
    lv_obj_move_foreground(confirmation_screen);
    
    // Title
    lv_obj_t* title_label = lv_label_create(confirmation_screen);
    lv_label_set_text(title_label, "Touch Calibration");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10);
    
    // Message
    lv_obj_t* message_label = lv_label_create(confirmation_screen);
    lv_label_set_text(message_label, "New calibration applied!\nKeep these settings?");
    lv_obj_set_style_text_color(message_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(message_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(message_label, 300);
    lv_obj_align(message_label, LV_ALIGN_TOP_MID, 0, 45);
    
    // Countdown - OBERHALB der Buttons!
    countdown_label = lv_label_create(confirmation_screen);
    lv_label_set_text(countdown_label, "Auto-revert in: 10s");
    lv_obj_set_style_text_color(countdown_label, lv_color_hex(0xFFAA00), 0);
    lv_obj_set_style_text_font(countdown_label, &lv_font_montserrat_14, 0);
    lv_obj_set_width(countdown_label, 300);
    lv_obj_set_style_text_align(countdown_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(countdown_label, LV_ALIGN_CENTER, 0, 30);
    
    // Keep Button (GRÜN) - SCHMALER!
    lv_obj_t* keep_btn = lv_btn_create(confirmation_screen);
    lv_obj_set_size(keep_btn, 120, 50);
    lv_obj_align(keep_btn, LV_ALIGN_BOTTOM_LEFT, 20, -10);
    lv_obj_set_style_bg_color(keep_btn, lv_color_hex(0x00AA00), 0);
    
    lv_obj_t* keep_label = lv_label_create(keep_btn);
    lv_label_set_text(keep_label, "Keep");
    lv_obj_set_style_text_color(keep_label, lv_color_white(), 0);
    lv_obj_center(keep_label);
    
    lv_obj_add_event_cb(keep_btn, [](lv_event_t* e) {
        confirmTouchCalibration();
        teardownTouchCalibrationConfirmation();
        renderMenu(MENU_SETTINGS);
    }, LV_EVENT_CLICKED, NULL);
    
    // Revert Button (ROT) - SCHMALER!
    lv_obj_t* revert_btn = lv_btn_create(confirmation_screen);
    lv_obj_set_size(revert_btn, 120, 50);
    lv_obj_align(revert_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
    lv_obj_set_style_bg_color(revert_btn, lv_color_hex(0xAA0000), 0);
    
    lv_obj_t* revert_label = lv_label_create(revert_btn);
    lv_label_set_text(revert_label, "Revert");
    lv_obj_set_style_text_color(revert_label, lv_color_white(), 0);
    lv_obj_center(revert_label);
    
    lv_obj_add_event_cb(revert_btn, [](lv_event_t* e) {
        resetTouchCalibrationToDefaults();
        teardownTouchCalibrationConfirmation();
        renderMenu(MENU_SETTINGS);
    }, LV_EVENT_CLICKED, NULL);
    
    confirmation_start_time = millis();
    confirmation_timer = lv_timer_create([](lv_timer_t* timer) {
        long remaining_ms = (long)CONFIRMATION_TIMEOUT_MS - (long)(millis() - confirmation_start_time);
        if (remaining_ms > 0) {
            char txt[50];
            snprintf(txt, sizeof(txt), "Auto-revert in: %lds", (remaining_ms / 1000) + 1);
            if (countdown_label) lv_label_set_text(countdown_label, txt);
        } else {
            printf("[TouchCal] Confirmation timeout, reverting.\n");
            resetTouchCalibrationToDefaults();
            teardownTouchCalibrationConfirmation();
            renderMenu(MENU_SETTINGS);
        }
    }, 500, NULL);
}

void teardownTouchCalibrationConfirmation() {
    if (confirmation_timer) {
        lv_timer_del(confirmation_timer);
        confirmation_timer = nullptr;
    }
    if (confirmation_screen) {
        lv_obj_del(confirmation_screen);
        confirmation_screen = nullptr;
    }
}

// ============================================
// MAIN CALIBRATION FUNCTION
// ============================================

void renderTouchCalibrationScreen() {
    printf("[TouchCal] Starting calibration\n");
    
    teardownTouchCalibrationScreen();
    
    vTaskDelay(pdMS_TO_TICKS(50));
    
    cal_screen = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(cal_screen);
    lv_obj_set_style_bg_color(cal_screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(cal_screen, LV_OPA_COVER, 0);
    lv_obj_set_size(cal_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_center(cal_screen);
    
    temp_cal_matrix[0] = 0.0f;
    temp_cal_matrix[1] = 1.0f;
    temp_cal_matrix[2] = 0.0f;
    temp_cal_matrix[3] = 0.0f;
    temp_cal_matrix[4] = 0.0f;
    temp_cal_matrix[5] = 1.0f;
    temp_cal_matrix[6] = 1.0f;
    
    current_step = CAL_STEP_OFFSET_X;
    current_pass = CAL_PASS_COARSE;
    step_is_locked = false;
    lock_frame_counter = 0;
    
    current_offset_step = OFFSET_STEP_COARSE;
    current_scale_step = SCALE_STEP_COARSE;
    current_rotation_step = ROTATION_STEP_COARSE;
    current_theta = 0.0f;
    
    setup_ui_for_step(CAL_STEP_OFFSET_X);
    
    cal_timer = lv_timer_create(process_calibration_logic, 50, NULL);
    
    printf("[TouchCal] Calibration screen ready\n");
}

void teardownTouchCalibrationScreen() {
    printf("[TouchCal] Teardown\n");
    
    if (cal_timer) {
        lv_timer_del(cal_timer);
        cal_timer = nullptr;
    }
    
    if (cal_screen) {
        lv_obj_del(cal_screen);
        cal_screen = nullptr;
    }
    
    info_label = nullptr;
    target_visual = nullptr;
    zone1 = nullptr;
    zone2 = nullptr;
    zone_target = nullptr;
}

// ============================================
// STATE TRANSITION LOGIC
// ============================================

static void advance_to_next_step() {
    printf("[TouchCal] Advancing from step %d (pass %d)\n", current_step, current_pass);
    
    if (current_pass == CAL_PASS_COARSE) {
        switch (current_step) {
            case CAL_STEP_OFFSET_X:
                if (temp_cal_matrix[0] > 0) {
                    temp_cal_matrix[0] -= REFINEMENT_OFFSET_BACKTRACK;
                } else {
                    temp_cal_matrix[0] += REFINEMENT_OFFSET_BACKTRACK;
                }
                current_offset_step = OFFSET_STEP_FINE;
                printf("[TouchCal] Refinement: offset_x=%.2f, step=%.2f\n", 
                       temp_cal_matrix[0], current_offset_step);
                break;
                
            case CAL_STEP_OFFSET_Y:
                if (temp_cal_matrix[3] > 0) {
                    temp_cal_matrix[3] -= REFINEMENT_OFFSET_BACKTRACK;
                } else {
                    temp_cal_matrix[3] += REFINEMENT_OFFSET_BACKTRACK;
                }
                current_offset_step = OFFSET_STEP_FINE;
                printf("[TouchCal] Refinement: offset_y=%.2f, step=%.2f\n", 
                       temp_cal_matrix[3], current_offset_step);
                break;
                
            case CAL_STEP_ROTATION:
                if (current_theta > 0) {
                    current_theta -= REFINEMENT_ROTATION_BACKTRACK;
                } else {
                    current_theta += REFINEMENT_ROTATION_BACKTRACK;
                }
                update_rotation_matrix(current_theta);
                current_rotation_step = ROTATION_STEP_FINE;
                printf("[TouchCal] Refinement: theta=%.3f, step=%.3f\n", 
                       current_theta, current_rotation_step);
                break;
                
            case CAL_STEP_SCALE_X: {
                float cos_t = cosf(current_theta);
                float sin_t = sinf(current_theta);
                temp_cal_matrix[1] -= REFINEMENT_SCALE_BACKTRACK * cos_t;
                temp_cal_matrix[2] -= REFINEMENT_SCALE_BACKTRACK * sin_t;
                current_scale_step = SCALE_STEP_FINE;
                printf("[TouchCal] Refinement: scale_x A=%.3f, B=%.3f\n", 
                       temp_cal_matrix[1], temp_cal_matrix[2]);
                break;
            }
                
            case CAL_STEP_SCALE_Y: {
                float cos_t = cosf(current_theta);
                float sin_t = sinf(current_theta);
                temp_cal_matrix[4] += REFINEMENT_SCALE_BACKTRACK * sin_t;
                temp_cal_matrix[5] -= REFINEMENT_SCALE_BACKTRACK * cos_t;
                current_scale_step = SCALE_STEP_FINE;
                printf("[TouchCal] Refinement: scale_y D=%.3f, E=%.3f\n", 
                       temp_cal_matrix[4], temp_cal_matrix[5]);
                break;
            }
                
            default:
                break;
        }
        
        current_pass = CAL_PASS_FINE;
        step_is_locked = false;
        lock_frame_counter = 0;
        
        setup_ui_for_step(current_step);
        return;
    }
    
    step_is_locked = false;
    lock_frame_counter = 0;
    current_pass = CAL_PASS_COARSE;
    
    current_offset_step = OFFSET_STEP_COARSE;
    current_scale_step = SCALE_STEP_COARSE;
    current_rotation_step = ROTATION_STEP_COARSE;
    
    current_step = (calibration_step_t)(current_step + 1);
    
    if (current_step == CAL_STEP_FINISHED) {
        save_and_exit_calibration();
    } else {
        setup_ui_for_step(current_step);
    }
}

// ============================================
// SUTHE'S METHOD - UI SETUP (MIT ROTER-PUNKT-FIX!)
// ============================================

static void setup_ui_for_step(calibration_step_t step) {
    printf("[TouchCal] Setup UI for step %d (pass %d)\n", step, current_pass);
    
    // WICHTIG: ALLE Kinder des cal_screen löschen (inkl. roter Punkt!)
    lv_obj_clean(cal_screen);
    
    // Pointer zurücksetzen
    target_visual = nullptr;
    zone1 = nullptr;
    zone2 = nullptr;
    zone_target = nullptr;
    info_label = nullptr;
    
    if (step == CAL_STEP_SUMMARY) {
        info_label = lv_label_create(cal_screen);
        lv_obj_set_style_text_color(info_label, lv_color_white(), 0);
        lv_obj_set_style_text_font(info_label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(info_label, SCREEN_WIDTH - 40);
        
        float rotation_degrees = current_theta * 180.0f / M_PI;
        
        char summary_text[300];
        snprintf(summary_text, sizeof(summary_text),
                 "%s\n\n"
                 "Offset X: %.1f\n"
                 "Offset Y: %.1f\n"
                 "Rotation: %.2f°\n"
                 "Scale X: %.3f\n"
                 "Scale Y: %.3f",
                 TXT_SUMMARY,
                 temp_cal_matrix[0], temp_cal_matrix[3],
                 rotation_degrees,
                 temp_cal_matrix[1], temp_cal_matrix[5]);
        
        lv_label_set_text(info_label, summary_text);
        lv_obj_center(info_label);
        return;
    }
    
    target_visual = lv_obj_create(cal_screen);
    lv_obj_remove_style_all(target_visual);
    lv_obj_set_style_bg_color(target_visual, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(target_visual, LV_OPA_COVER, 0);
    lv_obj_clear_flag(target_visual, LV_OBJ_FLAG_CLICKABLE);
    
    zone1 = lv_btn_create(cal_screen);
    lv_obj_set_style_bg_opa(zone1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(zone1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_opa(zone1, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(zone1, LV_OBJ_FLAG_CLICKABLE);
    
    zone2 = lv_btn_create(cal_screen);
    lv_obj_set_style_bg_opa(zone2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(zone2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_opa(zone2, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(zone2, LV_OBJ_FLAG_CLICKABLE);
    
    zone_target = lv_btn_create(cal_screen);
    lv_obj_set_style_bg_opa(zone_target, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(zone_target, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_opa(zone_target, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(zone_target, LV_OBJ_FLAG_CLICKABLE);
    
    info_label = lv_label_create(cal_screen);
    lv_obj_set_style_text_color(info_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(info_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(info_label, SCREEN_WIDTH - 40);
    lv_obj_align(info_label, LV_ALIGN_TOP_MID, 0, 20);
    
    switch (step) {
        case CAL_STEP_OFFSET_X:
            lv_label_set_text_static(info_label, 
                current_pass == CAL_PASS_COARSE ? TXT_OFFSET_X_COARSE : TXT_OFFSET_X_FINE);
            
            lv_obj_set_size(target_visual, 5, SCREEN_HEIGHT);
            lv_obj_align(target_visual, LV_ALIGN_CENTER, 0, 0);
            
            lv_obj_set_size(zone1, (SCREEN_WIDTH / 2) - 3, SCREEN_HEIGHT);
            lv_obj_align(zone1, LV_ALIGN_LEFT_MID, 0, 0);
            
            lv_obj_set_size(zone2, (SCREEN_WIDTH / 2) - 3, SCREEN_HEIGHT);
            lv_obj_align(zone2, LV_ALIGN_RIGHT_MID, 0, 0);
            
            lv_obj_set_size(zone_target, 5, SCREEN_HEIGHT);
            lv_obj_align(zone_target, LV_ALIGN_CENTER, 0, 0);
            break;
            
        case CAL_STEP_OFFSET_Y:
            lv_label_set_text_static(info_label, 
                current_pass == CAL_PASS_COARSE ? TXT_OFFSET_Y_COARSE : TXT_OFFSET_Y_FINE);
            
            lv_obj_set_size(target_visual, SCREEN_WIDTH, 5);
            lv_obj_align(target_visual, LV_ALIGN_CENTER, 0, 0);
            
            lv_obj_set_size(zone1, SCREEN_WIDTH, (SCREEN_HEIGHT / 2) - 3);
            lv_obj_align(zone1, LV_ALIGN_TOP_MID, 0, 0);
            
            lv_obj_set_size(zone2, SCREEN_WIDTH, (SCREEN_HEIGHT / 2) - 3);
            lv_obj_align(zone2, LV_ALIGN_BOTTOM_MID, 0, 0);
            
            lv_obj_set_size(zone_target, SCREEN_WIDTH, 5);
            lv_obj_align(zone_target, LV_ALIGN_CENTER, 0, 0);
            break;
            
        case CAL_STEP_ROTATION: {
            lv_label_set_text_static(info_label, 
                current_pass == CAL_PASS_COARSE ? TXT_ROTATION_COARSE : TXT_ROTATION_FINE);
            
            lv_obj_set_size(target_visual, 5, SCREEN_HEIGHT);
            lv_obj_align(target_visual, LV_ALIGN_CENTER, 0, 0);
            
            lv_obj_t* target_dot = lv_obj_create(cal_screen);
            lv_obj_remove_style_all(target_dot);
            lv_obj_set_style_bg_color(target_dot, lv_color_hex(0xFF0000), 0);
            lv_obj_set_style_bg_opa(target_dot, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(target_dot, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_size(target_dot, 20, 20);
            lv_obj_set_pos(target_dot, (SCREEN_WIDTH / 2) - 10, (SCREEN_HEIGHT / 4) - 10);
            lv_obj_clear_flag(target_dot, LV_OBJ_FLAG_CLICKABLE);
            
            lv_obj_set_size(zone1, (SCREEN_WIDTH / 2) - 3, SCREEN_HEIGHT);
            lv_obj_align(zone1, LV_ALIGN_LEFT_MID, 0, 0);
            
            lv_obj_set_size(zone2, (SCREEN_WIDTH / 2) - 3, SCREEN_HEIGHT);
            lv_obj_align(zone2, LV_ALIGN_RIGHT_MID, 0, 0);
            
            lv_obj_set_size(zone_target, 5, SCREEN_HEIGHT / 2);
            lv_obj_set_pos(zone_target, (SCREEN_WIDTH / 2) - 2, 0);
            break;
        }
            
        case CAL_STEP_SCALE_X: {
            lv_label_set_text_static(info_label, 
                current_pass == CAL_PASS_COARSE ? TXT_SCALE_X_COARSE : TXT_SCALE_X_FINE);
            
            if (current_pass == CAL_PASS_COARSE) {
                float cos_theta = cosf(current_theta);
                float sin_theta = sinf(current_theta);
                temp_cal_matrix[1] = 0.5f * cos_theta;
                temp_cal_matrix[2] = 0.5f * sin_theta;
                printf("[TouchCal] Scale X initialized: A=%.3f, B=%.3f\n", 
                       temp_cal_matrix[1], temp_cal_matrix[2]);
            }
            
            lv_obj_set_size(target_visual, 20, 20);
            lv_obj_set_style_radius(target_visual, LV_RADIUS_CIRCLE, 0);
            lv_obj_align(target_visual, LV_ALIGN_RIGHT_MID, -10, 0);
            
            lv_obj_set_size(zone1, SCREEN_WIDTH - 40, SCREEN_HEIGHT);
            lv_obj_align(zone1, LV_ALIGN_LEFT_MID, 0, 0);
            
            lv_obj_set_size(zone2, 1, 1);
            
            lv_obj_set_size(zone_target, 40, 80);
            lv_obj_align(zone_target, LV_ALIGN_RIGHT_MID, 0, 0);
            break;
        }
            
        case CAL_STEP_SCALE_Y: {
            lv_label_set_text_static(info_label, 
                current_pass == CAL_PASS_COARSE ? TXT_SCALE_Y_COARSE : TXT_SCALE_Y_FINE);
            
            if (current_pass == CAL_PASS_COARSE) {
                float cos_theta = cosf(current_theta);
                float sin_theta = sinf(current_theta);
                temp_cal_matrix[4] = -0.5f * sin_theta;
                temp_cal_matrix[5] = 0.5f * cos_theta;
                printf("[TouchCal] Scale Y initialized: D=%.3f, E=%.3f\n", 
                       temp_cal_matrix[4], temp_cal_matrix[5]);
            }
            
            lv_obj_set_size(target_visual, 20, 20);
            lv_obj_set_style_radius(target_visual, LV_RADIUS_CIRCLE, 0);
            lv_obj_align(target_visual, LV_ALIGN_BOTTOM_MID, 0, -10);
            
            lv_obj_set_size(zone1, SCREEN_WIDTH, SCREEN_HEIGHT - 40);
            lv_obj_align(zone1, LV_ALIGN_TOP_MID, 0, 0);
            
            lv_obj_set_size(zone2, 1, 1);
            
            lv_obj_set_size(zone_target, 80, 40);
            lv_obj_align(zone_target, LV_ALIGN_BOTTOM_MID, 0, 0);
            break;
        }
            
        default:
            break;
    }
    
    lv_obj_update_layout(cal_screen);
}

// ============================================
// SUTHE'S METHOD - KERNLOGIK (MIT TIMEOUT + DATA RESET!)
// ============================================

static void process_calibration_logic(lv_timer_t* timer) {
    static uint32_t last_touch_time = 0;
    static uint32_t summary_touch_time = 0;
    
    if (current_step == CAL_STEP_SUMMARY) {
        if (step_is_locked) {
            Touch_Read_Data();
            int current_points = touch_data.points;
            touch_data.points = 0;
            touch_data.x = 0;
            touch_data.y = 0;
            
            if (current_points == 0) {
                if (summary_touch_time == 0) {
                    summary_touch_time = millis();
                    printf("[TouchCal] Summary release detection started\n");
                } else if (millis() - summary_touch_time > 300) {
                    printf("[TouchCal] Summary touch released, exiting\n");
                    summary_touch_time = 0;
                    advance_to_next_step();
                }
            } else {
                summary_touch_time = 0;
            }
            return;
        }
        
        Touch_Read_Data();
        if (touch_data.points > 0) {
            printf("[TouchCal] Summary screen touch detected\n");
            step_is_locked = true;
        }
        return;
    }
    
    if (step_is_locked) {
        if (current_pass == CAL_PASS_FINE) {
            Touch_Read_Data();
            int current_points = touch_data.points;
            touch_data.points = 0;
            touch_data.x = 0;
            touch_data.y = 0;
            
            if (current_points == 0) {
                if (last_touch_time == 0) {
                    last_touch_time = millis();
                    printf("[TouchCal] Release detection started (timeout in 300ms)\n");
                } else {
                    uint32_t elapsed = millis() - last_touch_time;
                    if (elapsed > 300) {
                        printf("[TouchCal] Finger released (timeout after %dms), advancing\n", elapsed);
                        last_touch_time = 0;
                        advance_to_next_step();
                    }
                }
            } else {
                if (last_touch_time != 0) {
                    printf("[TouchCal] Touch detected again, timeout cancelled\n");
                }
                last_touch_time = 0;
            }
            return;
        } else {
            printf("[TouchCal] Coarse locked, switching to fine\n");
            advance_to_next_step();
            return;
        }
    }
    
    Touch_Read_Data();
    if (touch_data.points == 0) {
        lock_frame_counter = 0;
        return;
    }
    
    int16_t x_raw = touch_data.x;
    int16_t y_raw = touch_data.y;
    int16_t x_calibrated, y_calibrated;
    
    if (current_step == CAL_STEP_ROTATION) {
        float x_centered = (float)x_raw - (SCREEN_WIDTH / 2.0f);
        float y_centered = (float)y_raw - (SCREEN_HEIGHT / 2.0f);
        float cos_t = cosf(current_theta);
        float sin_t = sinf(current_theta);
        float x_rotated = cos_t * x_centered + sin_t * y_centered;
        float y_rotated = -sin_t * x_centered + cos_t * y_centered;
        float cal_x = x_rotated + (SCREEN_WIDTH / 2.0f);
        float cal_y = y_rotated + (SCREEN_HEIGHT / 2.0f);
        x_calibrated = (int16_t)(cal_x + 0.5f);
        y_calibrated = (int16_t)(cal_y + 0.5f);
    } else {
        float cal_x = (temp_cal_matrix[1] * x_raw) + (temp_cal_matrix[2] * y_raw) + temp_cal_matrix[0];
        float cal_y = (temp_cal_matrix[4] * x_raw) + (temp_cal_matrix[5] * y_raw) + temp_cal_matrix[3];
        cal_x /= temp_cal_matrix[6];
        cal_y /= temp_cal_matrix[6];
        x_calibrated = (int16_t)(cal_x + 0.5f);
        y_calibrated = (int16_t)(cal_y + 0.5f);
    }
    
    lv_point_t p;
    p.x = x_calibrated;
    p.y = y_calibrated;
    
    switch (current_step) {
        case CAL_STEP_OFFSET_X: {
            bool hit_left = lv_obj_hit_test(zone1, &p);
            bool hit_right = lv_obj_hit_test(zone2, &p);
            bool hit_target = lv_obj_hit_test(zone_target, &p);
            if (hit_left) {
                temp_cal_matrix[0] += current_offset_step;
                lock_frame_counter = 0;
            } else if (hit_right) {
                temp_cal_matrix[0] -= current_offset_step;
                lock_frame_counter = 0;
            } else if (hit_target) {
                lock_frame_counter++;
                if (lock_frame_counter >= LOCK_FRAMES) {
                    printf("[TouchCal] Offset X locked at %.2f\n", temp_cal_matrix[0]);
                    step_is_locked = true;
                    if (current_pass == CAL_PASS_FINE) {
                        lv_label_set_text_static(info_label, TXT_LOCKED);
                    }
                }
            } else {
                lock_frame_counter = 0;
            }
            break;
        }
        case CAL_STEP_OFFSET_Y: {
            bool hit_top = lv_obj_hit_test(zone1, &p);
            bool hit_bottom = lv_obj_hit_test(zone2, &p);
            bool hit_target = lv_obj_hit_test(zone_target, &p);
            if (hit_top) {
                temp_cal_matrix[3] += current_offset_step;
                lock_frame_counter = 0;
            } else if (hit_bottom) {
                temp_cal_matrix[3] -= current_offset_step;
                lock_frame_counter = 0;
            } else if (hit_target) {
                lock_frame_counter++;
                if (lock_frame_counter >= LOCK_FRAMES) {
                    printf("[TouchCal] Offset Y locked at %.2f\n", temp_cal_matrix[3]);
                    step_is_locked = true;
                    if (current_pass == CAL_PASS_FINE) {
                        lv_label_set_text_static(info_label, TXT_LOCKED);
                    }
                }
            } else {
                lock_frame_counter = 0;
            }
            break;
        }
        case CAL_STEP_ROTATION: {
            bool hit_left = lv_obj_hit_test(zone1, &p);
            bool hit_right = lv_obj_hit_test(zone2, &p);
            bool hit_target = lv_obj_hit_test(zone_target, &p);
            static int debug_counter = 0;
            if (debug_counter++ % 20 == 0) {
                printf("[TouchCal-Rotation] Raw: (%d,%d) → Cal: (%d,%d) | theta=%.3f rad (%.1f°)\n", 
                       x_raw, y_raw, x_calibrated, y_calibrated, current_theta, current_theta * 180.0f / M_PI);
                printf("[TouchCal-Rotation] Hit: left=%d right=%d target=%d | lock=%d/%d\n", 
                       hit_left, hit_right, hit_target, lock_frame_counter, LOCK_FRAMES);
            }
            if (hit_left) {
                current_theta -= current_rotation_step;
                update_rotation_matrix(current_theta);
                lock_frame_counter = 0;
            } else if (hit_right) {
                current_theta += current_rotation_step;
                update_rotation_matrix(current_theta);
                lock_frame_counter = 0;
            } else if (hit_target) {
                lock_frame_counter++;
                if (lock_frame_counter >= LOCK_FRAMES) {
                    printf("[TouchCal-Rotation] LOCKED! Final theta=%.3f rad (%.1f°)\n",
                           current_theta, current_theta * 180.0f / M_PI);
                    step_is_locked = true;
                    if (current_pass == CAL_PASS_FINE) {
                        lv_label_set_text_static(info_label, TXT_LOCKED);
                    }
                }
            } else {
                lock_frame_counter = 0;
            }
            break;
        }
        case CAL_STEP_SCALE_X: {
            bool hit_left = lv_obj_hit_test(zone1, &p);
            bool hit_target = lv_obj_hit_test(zone_target, &p);
            if (hit_left) {
                float cos_t = cosf(current_theta);
                float sin_t = sinf(current_theta);
                temp_cal_matrix[1] += current_scale_step * cos_t;
                temp_cal_matrix[2] += current_scale_step * sin_t;
                lock_frame_counter = 0;
            } else if (hit_target) {
                lock_frame_counter++;
                if (lock_frame_counter >= LOCK_FRAMES) {
                    printf("[TouchCal] Scale X locked at A=%.3f, B=%.3f\n", 
                           temp_cal_matrix[1], temp_cal_matrix[2]);
                    step_is_locked = true;
                    if (current_pass == CAL_PASS_FINE) {
                        lv_label_set_text_static(info_label, TXT_LOCKED);
                    }
                }
            } else {
                lock_frame_counter = 0;
            }
            break;
        }
        case CAL_STEP_SCALE_Y: {
            bool hit_top = lv_obj_hit_test(zone1, &p);
            bool hit_target = lv_obj_hit_test(zone_target, &p);
            if (hit_top) {
                float cos_t = cosf(current_theta);
                float sin_t = sinf(current_theta);
                temp_cal_matrix[4] -= current_scale_step * sin_t;
                temp_cal_matrix[5] += current_scale_step * cos_t;
                lock_frame_counter = 0;
            } else if (hit_target) {
                lock_frame_counter++;
                if (lock_frame_counter >= LOCK_FRAMES) {
                    printf("[TouchCal] Scale Y locked at D=%.3f, E=%.3f\n", 
                           temp_cal_matrix[4], temp_cal_matrix[5]);
                    step_is_locked = true;
                    if (current_pass == CAL_PASS_FINE) {
                        lv_label_set_text_static(info_label, TXT_LOCKED);
                    }
                }
            } else {
                lock_frame_counter = 0;
            }
            break;
        }
        default:
            break;
    }
}

// ============================================
// SAVE CALIBRATION AND EXIT
// ============================================

static void save_and_exit_calibration() {
    printf("[TouchCal] Saving final matrix...\n");
    
    NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)1);
    NVS.setInt(KEY_TOUCH_CAL_VALID, (int32_t)1);
    
    for (int i = 0; i < 7; i++) {
        char key[20];
        snprintf(key, sizeof(key), "touch_cal_%d", i);
        NVS.setFloat(key, temp_cal_matrix[i]);
    }
    
    updateTouchCalibrationMatrix(temp_cal_matrix);
    
    float rotation_degrees = current_theta * 180.0f / M_PI;
    printf("[TouchCal] COMPLETE! Matrix: C=%.2f A=%.3f F=%.2f E=%.3f Rotation=%.2f°\n",
           temp_cal_matrix[0], temp_cal_matrix[1],
           temp_cal_matrix[3], temp_cal_matrix[5], rotation_degrees);
    printf("[TouchCal] Pending confirmation flag set. Will show confirmation on next boot.\n");
    
    renderMenu(MENU_SETTINGS);
    printf("[TouchCal] Returned to settings\n");
}
