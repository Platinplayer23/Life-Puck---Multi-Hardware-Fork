#pragma once

#include <lvgl.h>
#include "lv_conf.h"
#include <esp_heap_caps.h>
#include "Display_ST77916.h"
#include "Touch_CST816.h"

#define LCD_WIDTH     EXAMPLE_LCD_WIDTH
#define LCD_HEIGHT    EXAMPLE_LCD_HEIGHT

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
