#include "LVGL_Driver.h"

static lv_display_t *display;
static lv_indev_t *indev;

// *** OPTIMIERT: PSRAM für größere Buffer nutzen ***
// Use PSRAM for larger buffers to free up internal RAM
static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);
static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);

// LVGL v9 flush callback
void Lvgl_Display_Flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    LCD_addWindow(area->x1, area->y1, area->x2, area->y2, (uint16_t *)px_map);
    lv_display_flush_ready(disp);
}

// LVGL v9 touchpad read callback with OPTIMIZED SCALING
void Lvgl_Touchpad_Read(lv_indev_t *indev, lv_indev_data_t *data) {
    Touch_Read_Data();
    if (touch_data.points != 0) {
        // *** OPTIMIERT: Pre-calculated constants für bessere Performance ***
        static const float scale_x = 0.85f;
        static const float scale_y = 1.0f;
        static const int center_x = LCD_WIDTH / 2;
        static const int center_y = LCD_HEIGHT / 2;
        static const int max_x = LCD_WIDTH - 1;
        static const int max_y = LCD_HEIGHT - 1;
        
        int dx = touch_data.x - center_x;
        int dy = touch_data.y - center_y;
        
        // *** OPTIMIERT: Direkte Berechnung ohne Zwischenschritte ***
        int corrected_x = center_x + (int)(dx * scale_x);
        int corrected_y = center_y + (int)(dy * scale_y);
        
        // *** OPTIMIERT: Clamp mit ternären Operatoren ***
        corrected_x = (corrected_x < 0) ? 0 : (corrected_x > max_x) ? max_x : corrected_x;
        corrected_y = (corrected_y < 0) ? 0 : (corrected_y > max_y) ? max_y : corrected_y;
        
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
