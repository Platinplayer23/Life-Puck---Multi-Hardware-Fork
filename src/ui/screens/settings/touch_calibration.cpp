// ============================================
// Own Header (first!)
// ============================================
#include "touch_calibration.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>
#include <ArduinoNvs.h>
#include "hardware/display/lvgl_driver.h"  // For global touch calibration variables

// ============================================
// Hardware
// ============================================
#include "hardware/touch/touch_cst816.h"
#include "hardware/peripherals/power_key.h"
#include "hardware/display/lvgl_driver.h" // For touch calibration globals

// ============================================
// UI Screens
// ============================================
#include "ui/screens/menu/menu.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

// ============================================
// CALIBRATION CONSTANTS
// ============================================
#define CALIBRATION_POINTS 5
#define CALIBRATION_TIMEOUT_MS 30000  // 30 seconds
#define CONFIRMATION_TIMEOUT_MS 10000 // 10 seconds
// Emergency BOOT button reset removed - conflicts with ESP32 download mode

// NVS Keys for calibration data
#define KEY_TOUCH_CAL_VALID "touch_valid"
#define KEY_TOUCH_OFFSET_X "touch_off_x" 
#define KEY_TOUCH_OFFSET_Y "touch_off_y"
#define KEY_TOUCH_SCALE_X "touch_scale_x"
#define KEY_TOUCH_SCALE_Y "touch_scale_y"
#define KEY_TOUCH_PENDING_CONFIRM "touch_pending"  // NEW: Pending confirmation flag

// ============================================
// STATIC VARIABLES
// ============================================
static lv_obj_t *calibration_screen = nullptr;
static lv_obj_t *instruction_label = nullptr;
static lv_obj_t *target_point = nullptr;
static lv_obj_t *progress_bar = nullptr;

// Confirmation screen objects
static lv_obj_t *confirmation_screen = nullptr;
static lv_obj_t *countdown_label = nullptr;
static uint32_t confirmation_start_time = 0;
static lv_timer_t *confirmation_timer = nullptr;

static int current_point = 0;
static uint32_t calibration_start_time = 0;
static bool calibration_active = false;

// Calibration points (screen coordinates) - User-optimized positioning for better edge detection
static lv_point_t calibration_targets[CALIBRATION_POINTS] = {
    {60, 60},           // Point 1 - Top-Left (good as is)
    {280, 60},          // Point 2 - Top-Right (aligned with Point 5)
    {160, 180},         // Point 3 - Center (moved left from 180 to be more centered)
    {55, 240},          // Point 4 - Bottom-Left (tiny bit left from 60, same height as Point 1)  
    {280, 240}          // Point 5 - Bottom-Right (aligned with Point 2)
};

// Measured touch coordinates 
static lv_point_t measured_points[CALIBRATION_POINTS];

// ============================================
// FAIL-SAFE FUNCTIONS
// ============================================

// Emergency BOOT button reset removed - conflicts with ESP32 download mode

void resetTouchCalibrationToDefaults() {
    printf("[TouchCal] Resetting to factory defaults\n");
    
    // Clear calibration data from NVS
    NVS.setInt(KEY_TOUCH_CAL_VALID, (int32_t)0);
    NVS.setFloat(KEY_TOUCH_OFFSET_X, 0.0f);
    NVS.setFloat(KEY_TOUCH_OFFSET_Y, 0.0f); 
    NVS.setFloat(KEY_TOUCH_SCALE_X, 1.0f);
    NVS.setFloat(KEY_TOUCH_SCALE_Y, 1.0f);
    NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)0);  // Clear pending flag
    
    // Apply defaults immediately using API function
    updateTouchCalibration(1.0f, 1.0f, 0.0f, 0.0f);
    
    printf("[TouchCal] Factory defaults restored and applied immediately\n");
}

/**
 * @brief Check if touch calibration needs confirmation after boot
 * @return true if confirmation dialog should be shown
 */
bool needsTouchCalibrationConfirmation() {
    return NVS.getInt(KEY_TOUCH_PENDING_CONFIRM, 0) == 1;
}

/**
 * @brief Confirm the current touch calibration (remove pending flag)
 */
void confirmTouchCalibration() {
    NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)0);
    printf("[TouchCal] Calibration confirmed and saved permanently\n");
}

/**
 * @brief Load and apply saved touch calibration from NVS
 * 
 * Called during boot to restore previously calibrated touch settings.
 * If no valid calibration exists, uses factory defaults.
 */
void loadTouchCalibrationFromNVS() {
    // Check if we have valid calibration data
    int cal_valid = NVS.getInt(KEY_TOUCH_CAL_VALID, 0);
    
    if (cal_valid == 1) {
        // Load calibration values
        float scale_x = NVS.getFloat(KEY_TOUCH_SCALE_X, 1.0f);
        float scale_y = NVS.getFloat(KEY_TOUCH_SCALE_Y, 1.0f);
        float offset_x = NVS.getFloat(KEY_TOUCH_OFFSET_X, 0.0f);
        float offset_y = NVS.getFloat(KEY_TOUCH_OFFSET_Y, 0.0f);
        
        printf("[TouchCal] Loaded calibration: scale_x=%.3f, scale_y=%.3f, offset_x=%.3f, offset_y=%.3f\n", 
               scale_x, scale_y, offset_x, offset_y);
        
        // Apply to LVGL touch driver - use API function to avoid linker issues
        updateTouchCalibration(scale_x, scale_y, offset_x, offset_y);
        
        printf("[TouchCal] ALL calibration values applied (scale AND offset)\n");
    } else {
        printf("[TouchCal] No valid calibration found, using factory defaults\n");
    }
}

// ============================================
// CALIBRATION LOGIC
// ============================================

static void calculateCalibrationData() {
    // Real calibration calculation using measured touch points
    printf("[TouchCal] Calculating calibration from %d measured points\n", CALIBRATION_POINTS);
    
    // Calculate average offset (difference between target and measured)
    float total_offset_x = 0.0f;
    float total_offset_y = 0.0f;
    
    for (int i = 0; i < CALIBRATION_POINTS; i++) {
        float diff_x = calibration_targets[i].x - measured_points[i].x;
        float diff_y = calibration_targets[i].y - measured_points[i].y;
        total_offset_x += diff_x;
        total_offset_y += diff_y;
        
        printf("[TouchCal] Point %d: Target(%d,%d) Measured(%d,%d) Diff(%.1f,%.1f)\n", 
               i+1, calibration_targets[i].x, calibration_targets[i].y,
               measured_points[i].x, measured_points[i].y, diff_x, diff_y);
    }
    
    float offset_x = total_offset_x / CALIBRATION_POINTS;
    float offset_y = total_offset_y / CALIBRATION_POINTS;
    
    // For now, keep existing scale values (could be calculated from corner points)
    float scale_x = 0.85f;  // From working calibration
    float scale_y = 1.0f;   // From working calibration
    
    printf("[TouchCal] Calculated: offset_x=%.2f, offset_y=%.2f\n", offset_x, offset_y);
    
    // Store in NVS and mark as pending confirmation
    NVS.setInt(KEY_TOUCH_CAL_VALID, (int32_t)1);
    NVS.setFloat(KEY_TOUCH_SCALE_X, scale_x);
    NVS.setFloat(KEY_TOUCH_SCALE_Y, scale_y);
    NVS.setFloat(KEY_TOUCH_OFFSET_X, offset_x);
    NVS.setFloat(KEY_TOUCH_OFFSET_Y, offset_y);
    NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)1);  // NEW: Mark as pending confirmation
    
    // Apply calibration immediately using API function
    updateTouchCalibration(scale_x, scale_y, offset_x, offset_y);
    
    printf("[TouchCal] Calibration saved and applied immediately: scale_x=%.3f, scale_y=%.3f, offset_x=%.3f, offset_y=%.3f\n", 
           scale_x, scale_y, offset_x, offset_y);
    printf("[TouchCal] Confirmation will be required on next boot\n");
}

static void nextCalibrationPoint() {
    current_point++;
    
    if (current_point >= CALIBRATION_POINTS) {
        // Calibration complete - calculate and apply immediately!
        calculateCalibrationData();
        
        lv_label_set_text(instruction_label, "Calibration Complete!\nApplied immediately!\nTouch to exit");
        lv_obj_add_flag(target_point, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(progress_bar, LV_OBJ_FLAG_HIDDEN);
        
        return;
    }
    
    // Update UI for next point
    char instruction[64];
    snprintf(instruction, sizeof(instruction), "Touch the target\nPoint %d of %d", 
             current_point + 1, CALIBRATION_POINTS);
    lv_label_set_text(instruction_label, instruction);
    
    // Move target to next position
    lv_obj_set_pos(target_point, 
                   calibration_targets[current_point].x - 25,  // Center the 50x50 target (updated)
                   calibration_targets[current_point].y - 25);
                   
    // Update progress
    lv_bar_set_value(progress_bar, (current_point * 100) / CALIBRATION_POINTS, LV_ANIM_ON);
}

static void calibrationTouchHandler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        if (current_point < CALIBRATION_POINTS) {
            // Record touch point
            lv_indev_t *indev = lv_indev_get_act();
            lv_indev_get_point(indev, &measured_points[current_point]);
            
            printf("[TouchCal] Point %d: target(%d,%d) measured(%d,%d)\n",
                   current_point,
                   calibration_targets[current_point].x,
                   calibration_targets[current_point].y,
                   measured_points[current_point].x,
                   measured_points[current_point].y);
                   
            nextCalibrationPoint();
        } else {
            // Calibration complete - go back to settings (calibration is already applied!)
            printf("[TouchCal] Calibration completed and applied immediately\n");
            renderMenu(MENU_SETTINGS);
        }
    }
}

// ============================================
// UI FUNCTIONS  
// ============================================

void renderTouchCalibrationScreen() {
    teardownTouchCalibrationScreen();
    
    printf("[TouchCal] Starting touch calibration\n");
    
    // Create full-screen container
    calibration_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(calibration_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(calibration_screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(calibration_screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_opa(calibration_screen, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(calibration_screen, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    
    // Instruction label
    instruction_label = lv_label_create(calibration_screen);
    lv_label_set_text(instruction_label, "Touch Calibration\nTouch the target\nPoint 1 of 5");
    lv_obj_set_style_text_font(instruction_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(instruction_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(instruction_label, LV_ALIGN_TOP_MID, 0, 40);
    
    // Progress bar
    progress_bar = lv_bar_create(calibration_screen);
    lv_obj_set_size(progress_bar, 200, 20);
    lv_obj_align(progress_bar, LV_ALIGN_TOP_MID, 0, 120);
    lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
    
    // Target point (red circle) - Larger for better visibility and touch accuracy
    target_point = lv_obj_create(calibration_screen);
    lv_obj_set_size(target_point, 50, 50);  // Increased from 30x30 to 50x50
    lv_obj_set_style_bg_color(target_point, lv_color_make(255, 0, 0), LV_PART_MAIN);  // Red
    lv_obj_set_style_border_color(target_point, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(target_point, 3, LV_PART_MAIN);  // Thicker border
    lv_obj_set_style_radius(target_point, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_pos(target_point, 
                   calibration_targets[0].x - 25,  // Adjusted for new size (50/2 = 25)
                   calibration_targets[0].y - 25);
    
    // Add touch event handler to entire screen
    lv_obj_add_event_cb(calibration_screen, calibrationTouchHandler, LV_EVENT_CLICKED, NULL);
    
    // Initialize state
    current_point = 0;
    calibration_start_time = millis();
    calibration_active = true;
    
    printf("[TouchCal] Calibration screen ready\n");
}

void teardownTouchCalibrationScreen() {
    if (calibration_screen) {
        lv_obj_del(calibration_screen);
        calibration_screen = nullptr;
    }
    
    instruction_label = nullptr;
    target_point = nullptr;  
    progress_bar = nullptr;
    calibration_active = false;
    
    printf("[TouchCal] Screen cleaned up\n");
}

// ============================================
// CONFIRMATION DIALOG IMPLEMENTATION  
// ============================================

// Timer callback for updating confirmation dialog countdown
static void confirmation_timer_cb(lv_timer_t *timer) {
    if (confirmation_screen == nullptr) {
        lv_timer_del(timer);
        confirmation_timer = nullptr;
        return;
    }
    
    // Calculate remaining time
    uint32_t elapsed = millis() - confirmation_start_time;
    int remaining_seconds = 10 - (elapsed / 1000);
    
    // Auto-timeout: Reset to defaults if 10 seconds passed
    if (remaining_seconds <= 0) {
        printf("[TouchCal] Confirmation timeout! Resetting calibration to defaults\n");
        resetTouchCalibrationToDefaults();
        loadTouchCalibrationFromNVS(); // Reload defaults
        teardownTouchCalibrationConfirmation();
        return;
    }
    
    // Update countdown text
    if (countdown_label != nullptr) {
        char countdown_text[32];
        snprintf(countdown_text, sizeof(countdown_text), "Auto-reset in %d seconds", remaining_seconds);
        lv_label_set_text(countdown_label, countdown_text);
    }
}

void teardownTouchCalibrationConfirmation() {
    if (confirmation_timer != nullptr) {
        lv_timer_del(confirmation_timer);
        confirmation_timer = nullptr;
    }
    
    if (confirmation_screen != nullptr) {
        lv_obj_del(confirmation_screen);
        confirmation_screen = nullptr;
        countdown_label = nullptr;
        confirmation_start_time = 0;
        printf("[TouchCal] Confirmation dialog cleaned up\n");
    }
}

/**
 * @brief Render touch calibration confirmation screen
 * 
 * Shows a 10-second countdown dialog asking user to confirm new calibration.
 * Auto-reverts to defaults if no confirmation received.
 */
void renderTouchCalibrationConfirmation() {
    printf("[TouchCal] Showing calibration confirmation dialog\n");
    
    // Start countdown if not already started
    if (confirmation_start_time == 0) {
        confirmation_start_time = millis();
    }
    
    // Create confirmation screen if not exists
    if (confirmation_screen == nullptr) {
        // *** CRITICAL FIX: Create dialog as a separate layer with TOP parent ***
        confirmation_screen = lv_obj_create(lv_layer_top());  // Use TOP layer instead of lv_screen_active()
        lv_obj_set_size(confirmation_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
        lv_obj_set_style_bg_color(confirmation_screen, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(confirmation_screen, LV_OPA_90, 0); // Semi-transparent overlay
        lv_obj_center(confirmation_screen);
        
        // *** ADDITIONAL: Force to absolute foreground ***
        lv_obj_move_foreground(confirmation_screen);
        lv_obj_set_style_opa(confirmation_screen, LV_OPA_COVER, 0);  // Make sure it's fully opaque
        
        // Dialog container
        lv_obj_t *dialog = lv_obj_create(confirmation_screen);
        lv_obj_set_size(dialog, 280, 200);
        lv_obj_set_style_bg_color(dialog, lv_color_hex(0x1A1A1A), 0);
        lv_obj_set_style_border_color(dialog, lv_color_white(), 0);
        lv_obj_set_style_border_width(dialog, 2, 0);
        lv_obj_set_style_radius(dialog, 10, 0);
        lv_obj_center(dialog);
        
        // Title
        lv_obj_t *title = lv_label_create(dialog);
        lv_label_set_text(title, "Touch Calibration");
        lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(title, lv_color_white(), 0);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
        
        // Message
        lv_obj_t *message = lv_label_create(dialog);
        lv_label_set_text(message, "New calibration applied.\nConfirm to keep it?");
        lv_obj_set_style_text_font(message, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(message, lv_color_white(), 0);
        lv_obj_align(message, LV_ALIGN_TOP_MID, 0, 45);
        lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, 0);
        
        // Countdown label
        countdown_label = lv_label_create(dialog);
        lv_obj_set_style_text_font(countdown_label, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(countdown_label, lv_color_hex(0xFF6B35), 0); // Orange warning color
        lv_obj_align(countdown_label, LV_ALIGN_TOP_MID, 0, 85);
        
        // Confirm button
        lv_obj_t *btn_confirm = lv_btn_create(dialog);
        lv_obj_set_size(btn_confirm, 100, 40);
        lv_obj_set_style_bg_color(btn_confirm, lv_color_hex(0x4CAF50), 0); // Green
        lv_obj_align(btn_confirm, LV_ALIGN_BOTTOM_LEFT, 20, -15);
        lv_obj_t *lbl_confirm = lv_label_create(btn_confirm);
        lv_label_set_text(lbl_confirm, "Keep");
        lv_obj_set_style_text_color(lbl_confirm, lv_color_white(), 0);
        lv_obj_center(lbl_confirm);
        lv_obj_add_event_cb(btn_confirm, [](lv_event_t *e) {
            confirmTouchCalibration();
            teardownTouchCalibrationConfirmation();
            printf("[TouchCal] Calibration confirmed by user\n");
        }, LV_EVENT_CLICKED, NULL);
        
        // Cancel button
        lv_obj_t *btn_cancel = lv_btn_create(dialog);
        lv_obj_set_size(btn_cancel, 100, 40);
        lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0xF44336), 0); // Red
        lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_RIGHT, -20, -15);
        lv_obj_t *lbl_cancel = lv_label_create(btn_cancel);
        lv_label_set_text(lbl_cancel, "Reset");
        lv_obj_set_style_text_color(lbl_cancel, lv_color_white(), 0);
        lv_obj_center(lbl_cancel);
        lv_obj_add_event_cb(btn_cancel, [](lv_event_t *e) {
            resetTouchCalibrationToDefaults();
            loadTouchCalibrationFromNVS(); // Reload defaults
            teardownTouchCalibrationConfirmation();
            printf("[TouchCal] Calibration reset by user\n");
        }, LV_EVENT_CLICKED, NULL);
        
        // Create timer for countdown updates (1000ms = 1 second intervals)
        if (confirmation_timer == nullptr) {
            confirmation_timer = lv_timer_create(confirmation_timer_cb, 1000, NULL);
            printf("[TouchCal] Countdown timer started\n");
        }
        
        // Initial countdown display
        char countdown_text[32];
        snprintf(countdown_text, sizeof(countdown_text), "Auto-reset in 10 seconds");
        lv_label_set_text(countdown_label, countdown_text);
    }
}
