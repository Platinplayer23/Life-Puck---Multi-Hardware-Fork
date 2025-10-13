// ============================================
// System & Framework Headers
// ============================================
#include <Arduino.h>
#include <lvgl.h>
#include <ArduinoNvs.h>

// ============================================
// Config Headers
// ============================================
#include <board_config.h>

// ============================================
// Core System
// ============================================
#include "core/gui_main.h"
#include "core/main.h"
#include "core/state_manager.h"

// ============================================
// Hardware Layer
// ============================================
#include "hardware/peripherals/i2c_Driver.h"
#include "hardware/peripherals/TCA9554PWR.h"
#include "hardware/peripherals/power_key.h"
#include "hardware/display/Display_ST77916.h"
#include "hardware/display/LVGL_Driver.h"
#include "hardware/touch/Touch_CST816.h"
#include "hardware/system/battery_state.h"

// ============================================
// UI Layer
// ============================================
#include "ui/screens/life/life_counter.h"
#include "ui/screens/life/life_counter2P.h"
#include "ui/screens/tools/dice_coin.h"
#include "ui/helpers/event_grouper.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "data/tcg_presets.h"


extern uint8_t Touch_interrupts;

PlayerMode life_counter_mode = PLAYER_MODE_ONE_PLAYER;
bool is_two_player_mode = false;
int player1_life = 20;
int player2_life = 20;

void update_player1_display() {
}

void update_player2_display() {
}

void setup()
{
    Serial.begin(115200);
    Serial.println("--- Starting Life-Puck with Custom Demo Drivers ---");

    I2C_Init();
    TCA9554PWR_Init();
    LCD_Init();
    Backlight_Init();
    Set_Backlight(100);

    Lvgl_Init();

    battery_init();
    power_init();

    // Presets initialisieren BEVOR ui_init()!
    init_presets();
    load_preset();
    
    TCGPreset preset = get_preset();
    player1_life = preset.starting_life;
    player2_life = preset.starting_life;

    ui_init(); 
    if (psramFound()) {
    printf("PSRAM: %d bytes\n", ESP.getPsramSize());
    heap_caps_malloc_extmem_enable(4096);  // Use PSRAM for large allocations
  }
}

void loop()
{
    Lvgl_Loop();
    Touch_Loop();
    power_loop();
    
    if (life_counter_mode == PLAYER_MODE_ONE_PLAYER) {
        life_counter_loop();
    } else if (life_counter_mode == PLAYER_MODE_TWO_PLAYER) {
        life_counter2p_loop();
    }
    
    if (Touch_interrupts) {
        Touch_interrupts = 0;
    }
    
    lv_timer_handler();
    vTaskDelay(5 / portTICK_PERIOD_MS);
}

void apply_preset_to_game(int preset_index) {
    TCGPreset preset = get_preset();
    
    player1_life = preset.starting_life;
    player2_life = preset.starting_life;
    
    update_player1_display();
    if (is_two_player_mode) {
        update_player2_display();
    }
}
