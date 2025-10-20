#include "lvgl_driver.h"
#include "hardware/system/power_management.h"
#include "ui/screens/menu/menu.h"  // For getCurrentMenu() and MENU_TOUCH_CALIBRATION
#include "config.h"  // For touch calibration defaults
#include <cstring>  // Für memcpy

static lv_display_t *display;
static lv_indev_t *indev;

// ============================================
// === ZENTRALE DEFAULT-KALIBRIERUNG ===
// ============================================
// 7-Parameter Affine Transformation Matrix
// Formel: cal_x = ([1] * raw_x) + ([2] * raw_y) + [0]
//         cal_y = ([4] * raw_x) + ([5] * raw_y) + [3]
//         Dann beide durch [6] (divisor) teilen
//
// [0] = offset_x   - Horizontaler Offset (wird zu cal_x addiert)
// [1] = scale_x    - Horizontale Skalierung (multipliziert mit raw_x)
// [2] = shear_xy   - Rotation/Scherung (raw_y Einfluss auf X-Achse)
// [3] = offset_y   - Vertikaler Offset (wird zu cal_y addiert)
// [4] = shear_yx   - Rotation/Scherung (raw_x Einfluss auf Y-Achse)
// [5] = scale_y    - Vertikale Skalierung (multipliziert mit raw_y)
// [6] = divisor    - Matrix-Divisor (normalerweise 1.0)
//
// ============================================
// === Touch Calibration Defaults from config.h ===
// ============================================
// CENTRAL default matrix - now loaded from config.h for easy editing
// To change these values, edit src/config.h (Touch Calibration section)
const float DEFAULT_CAL_MATRIX[7] = {
    TOUCH_CAL_DEFAULT_OFFSET_X,   // [0] offset_x - Horizontal offset
    TOUCH_CAL_DEFAULT_SCALE_X,    // [1] scale_x - Horizontal scaling
    TOUCH_CAL_DEFAULT_SHEAR_XY,   // [2] shear_xy - Rotation/shear
    TOUCH_CAL_DEFAULT_OFFSET_Y,   // [3] offset_y - Vertical offset
    TOUCH_CAL_DEFAULT_SHEAR_YX,   // [4] shear_yx - Rotation/shear
    TOUCH_CAL_DEFAULT_SCALE_Y,    // [5] scale_y - Vertical scaling
    TOUCH_CAL_DEFAULT_DIVISOR     // [6] divisor - Matrix divisor
};

// Global touch calibration matrix (wird automatisch mit DEFAULT_CAL_MATRIX initialisiert)
float g_cal_matrix[7];

/**
 * @brief === NEU: Update global touch calibration matrix ===
 * @param matrix Pointer to a 7-element float array with new calibration data
 */
void updateTouchCalibrationMatrix(const float* matrix) {
    if (matrix) {
        memcpy(g_cal_matrix, matrix, sizeof(g_cal_matrix));
        
        // === DEBUG: Ausgabe der neuen Matrix-Werte ===
        printf("\n========================================\n");
        printf("[CALIBRATION] g_cal_matrix updated:\n");
        printf("  [0] offset_x  = %.3f\n", g_cal_matrix[0]);
        printf("  [1] scale_x   = %.3f\n", g_cal_matrix[1]);
        printf("  [2] shear_xy  = %.3f\n", g_cal_matrix[2]);
        printf("  [3] offset_y  = %.3f\n", g_cal_matrix[3]);
        printf("  [4] shear_yx  = %.3f\n", g_cal_matrix[4]);
        printf("  [5] scale_y   = %.3f\n", g_cal_matrix[5]);
        printf("  [6] divisor   = %.3f\n", g_cal_matrix[6]);
        printf("========================================\n\n");
    }
}

/**
 * @brief Get the default calibration matrix
 * @return Pointer to the default 7-parameter matrix
 */
const float* getDefaultCalibrationMatrix() {
    return DEFAULT_CAL_MATRIX;
}



static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t), MALLOC_CAP_DMA);
static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t), MALLOC_CAP_DMA);

// LVGL v9 flush callback
void Lvgl_Display_Flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    LCD_addWindow(area->x1, area->y1, area->x2, area->y2, (uint16_t *)px_map);
    lv_display_flush_ready(disp);
}

// LVGL v9 touchpad read callback with 7-PARAMETER MATRIX CORRECTION
void Lvgl_Touchpad_Read(lv_indev_t *indev, lv_indev_data_t *data) {
    Touch_Read_Data();
    
    // Block all LVGL touch events during calibration to prevent conflicts
    if (getCurrentMenu() == MENU_TOUCH_CALIBRATION) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    
    if (touch_data.points != 0) {
        power_reset_inactivity_timer();
        
        if (power_should_ignore_touch()) {
            data->state = LV_INDEV_STATE_RELEASED;
            return;
        }
    }
    
    if (touch_data.points != 0) {
        // === NEU: Anwenden der 7-Parameter-Matrix-Transformation ===
        int raw_x = touch_data.x;
        int raw_y = touch_data.y;

        // Apply 7-parameter matrix transformation
        float cal_x = (g_cal_matrix[1] * raw_x) + (g_cal_matrix[2] * raw_y) + g_cal_matrix[0];
        float cal_y = (g_cal_matrix[4] * raw_x) + (g_cal_matrix[5] * raw_y) + g_cal_matrix[3];
        cal_x /= g_cal_matrix[6];
        cal_y /= g_cal_matrix[6];

        int corrected_x = (int)(cal_x + 0.5f);
        int corrected_y = (int)(cal_y + 0.5f);

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

    if (!buf1 || !buf2) {
        printf("[LVGL_ERROR] Buffer allocation FAILED! Not enough DMA memory!\n");
        return;
    }
    
    display = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_buffers(display, buf1, buf2, LVGL_BUF_LEN * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, Lvgl_Display_Flush);
    
    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, Lvgl_Touchpad_Read);
    
    // === DIE FINALE LÖSUNG ===
    // Weist das Eingabegerät explizit dem korrekten Display zu
    lv_indev_set_display(indev, display);

    const esp_timer_create_args_t lvgl_tick_timer_args = { .callback = &lv_tick_task, .name = "lvgl_tick" };
    esp_timer_handle_t lvgl_tick_timer;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

    printf("[LVGL_Init] COMPLETE\n");
}

void Lvgl_Loop(void) {
    lv_timer_handler();
}