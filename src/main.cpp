#include <Arduino.h>
#include <lvgl.h>
#include "ArduinoNvs.h"
#include "gui_main.h"
#include "power_key/power_key.h"
#include "constants/constants.h"
#include "battery/battery_state.h"
#include <life/life_counter.h>
#include <life/life_counter2P.h>
#include <helpers/event_grouper.h>
#include "main.h"
#include "state/state_store.h"
#include "tcg_presets.h"
#include "dice_coin.h"

#include "I2C_Driver.h"
#include "TCA9554PWR.h"
#include "Display_ST77916.h"
#include "Touch_CST816.h"
#include "LVGL_Driver.h"

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
    // *** OPTIMIERT: Reduzierte Verzögerung für bessere Responsivität ***
    Lvgl_Loop();
    Touch_Loop();
    power_loop();
    
    if (life_counter_mode == PLAYER_MODE_ONE_PLAYER) {
        life_counter_loop();
    } else if (life_counter_mode == PLAYER_MODE_TWO_PLAYER) {
        life_counter2p_loop();
    }
    
    // *** OPTIMIERT: Touch-Interrupts nur bei Bedarf verarbeiten ***
    if (Touch_interrupts) {
        Touch_interrupts = 0;
    }
    
    lv_timer_handler();
    // *** OPTIMIERT: Reduzierte Verzögerung von 5ms auf 2ms ***
    vTaskDelay(2 / portTICK_PERIOD_MS);
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
