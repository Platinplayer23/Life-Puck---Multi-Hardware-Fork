#include "lvgl_driver.h"
#include "hardware/system/power_management.h"

static lv_display_t *display;
static lv_indev_t *indev;

// *** TOUCH CALIBRATION GLOBALS ***
// Global touch calibration values that can be updated from NVS
float g_touch_scale_x = 0.85f;  // Default value from original working calibration
float g_touch_scale_y = 1.0f;   // Default value from original working calibration
float g_touch_offset_x = 0.0f;  // NEW: X offset for calibration  
float g_touch_offset_y = 0.0f;  // NEW: Y offset for calibration

/**
 * @brief Update global touch calibration values (including offsets)
 * @param scale_x X scaling factor 
 * @param scale_y Y scaling factor
 * @param offset_x X offset in pixels
 * @param offset_y Y offset in pixels  
 */
void updateTouchCalibration(float scale_x, float scale_y, float offset_x, float offset_y) {
    g_touch_scale_x = scale_x;
    g_touch_scale_y = scale_y;
    g_touch_offset_x = offset_x;
    g_touch_offset_y = offset_y;
    printf("[TouchCal] Updated: scale(%.3f,%.3f) offset(%.3f,%.3f)\n", 
           scale_x, scale_y, offset_x, offset_y);
}

// *** OPTIMIERT: GRÖßERER BUFFER (von 32 auf 64 Zeilen) ***
// Use DMA capable memory for the buffer
static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t), MALLOC_CAP_DMA);
static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t), MALLOC_CAP_DMA);

// LVGL v9 flush callback
void Lvgl_Display_Flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    LCD_addWindow(area->x1, area->y1, area->x2, area->y2, (uint16_t *)px_map);
    lv_display_flush_ready(disp);
}

// LVGL v9 touchpad read callback with SCALING CORRECTION
void Lvgl_Touchpad_Read(lv_indev_t *indev, lv_indev_data_t *data) {
    Touch_Read_Data();
    
    // Reset inactivity timer on touch (after Touch_Read_Data to avoid I2C conflicts)
    if (touch_data.points != 0) {
        power_reset_inactivity_timer();
        
        // Ignore touch events for a short time after waking from sleep/dim
        if (power_should_ignore_touch()) {
            // Block touch completely by clearing touch data and marking as released
            touch_data.points = 0;
            touch_data.gesture = NONE;
            data->state = LV_INDEV_STATE_RELEASED;
            data->point.x = 0;
            data->point.y = 0;
            return;
        }
    }
    
    if (touch_data.points != 0) {
        // TOUCH CALIBRATION - Apply both scaling AND offset
        float scale_x = g_touch_scale_x;   // From calibration or default 0.85f
        float scale_y = g_touch_scale_y;   // From calibration or default 1.0f
        float offset_x = g_touch_offset_x; // From calibration or default 0.0f
        float offset_y = g_touch_offset_y; // From calibration or default 0.0f
        
        // Apply scaling first (around center)
        int center_x = LCD_WIDTH / 2;
        int center_y = LCD_HEIGHT / 2;
        
        int dx = touch_data.x - center_x;
        int dy = touch_data.y - center_y;
        
        int scaled_x = center_x + (int)(dx * scale_x);
        int scaled_y = center_y + (int)(dy * scale_y);
        
        // Then apply offset correction
        int corrected_x = scaled_x + (int)offset_x;
        int corrected_y = scaled_y + (int)offset_y;
        
        // Clamp to screen bounds
        if (corrected_x < 0) corrected_x = 0;
        if (corrected_y < 0) corrected_y = 0;
        if (corrected_x >= LCD_WIDTH) corrected_x = LCD_WIDTH - 1;
        if (corrected_y >= LCD_HEIGHT) corrected_y = LCD_HEIGHT - 1;
        
        data->point.x = corrected_x;
        data->point.y = corrected_y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    
    touch_data.points = 0;
    touch_data.gesture = NONE;
}

static void lv_tick_task(void *arg) {
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void Lvgl_Init(void) {
    lv_init();

    // *** CHECK BUFFER ALLOCATION ***
    if (!buf1 || !buf2) {
        printf("[LVGL_ERROR] Buffer allocation FAILED! Not enough DMA memory!\n");
        return;
    }
    printf("[LVGL_Init] Buffer allocated: %d bytes each (Total: %d KB)\n", 
           LVGL_BUF_LEN * sizeof(lv_color_t), 
           (LVGL_BUF_LEN * sizeof(lv_color_t) * 2) / 1024);

    // Create a display
    display = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    
    // *** OPTIMIERT: FULL REFRESH für bessere Performance ***
    lv_display_set_buffers(display, buf1, buf2, LVGL_BUF_LEN * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Set the flush callback
    lv_display_set_flush_cb(display, Lvgl_Display_Flush);

    // Clear display to black before UI creation to prevent static flash
    lv_color_t black_color = lv_color_black();
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, black_color, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // Create an input device
    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, Lvgl_Touchpad_Read);

    // LVGL Tick timer setup
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lv_tick_task,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

    // Create a simple label to show it's working
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "LVGL v9 Driver OK");
    lv_obj_center(label);
    
    printf("[LVGL_Init] COMPLETE - Display: %dx%d, Tick: %dms\n", 
           LCD_WIDTH, LCD_HEIGHT, EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void Lvgl_Loop(void) {
    lv_timer_handler();
}