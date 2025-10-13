#pragma once

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>
#include <esp_heap_caps.h>

// ============================================
// Config
// ============================================
#include "config/lv_conf.h"
#include <board_config.h> 

// ============================================
// Hardware Drivers (same level/nearby)
// ============================================
#include "display_st77916.h"
#include "../touch/touch_cst816.h"


#ifndef LCD_WIDTH
    #define LCD_WIDTH     360  // Fallback
#endif

#ifndef LCD_HEIGHT
    #define LCD_HEIGHT    360  // Fallback
#endif

// *** OPTIMIERT: GRÖßERER BUFFER (von 32 auf 64 Zeilen) ***
// Bei 480x480: (480 * 64 * 2 bytes) = 61.440 bytes = ~60 KB pro Buffer
#define LVGL_BUF_LEN  (LCD_WIDTH * 80)  // War: 32 -> JETZT: 64 Lines!

// *** SCHNELLERER TICK (von 10ms auf 5ms für 200Hz) ***
#define EXAMPLE_LVGL_TICK_PERIOD_MS  10  // War: 10ms -> JETZT: 5ms!

// Corrected function signatures for LVGL v9
void Lvgl_Display_Flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void Lvgl_Touchpad_Read(lv_indev_t *indev, lv_indev_data_t *data);

void Lvgl_Init(void);
void Lvgl_Loop(void);

// *** TOUCH CALIBRATION GLOBALS ***
// External access to touch calibration values for NVS loading
extern float g_touch_scale_x;
extern float g_touch_scale_y;
extern float g_touch_offset_x;  // NEW: Offset support
extern float g_touch_offset_y;  // NEW: Offset support

// *** TOUCH CALIBRATION API ***
void updateTouchCalibration(float scale_x, float scale_y, float offset_x, float offset_y);
