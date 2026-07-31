// ============================================
// Own Header (first!)
// ============================================
#include "power_key.h"

// ============================================
// System & Framework Headers
// ============================================
#include <Arduino.h>
#include <lvgl.h>
#include <esp_sleep.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// Hardware (related modules)
// ============================================
#include "../display/display_st77916.h"
#include "../system/shutdown.h"
#include "../system/power_management.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

// ============================================
// Board Configuration
// ============================================
#include "board_config.h"


#if HAS_POWER_KEY
// ============================================================================
// Boards with a power button / power-control MOSFET (1.85C, 1.85)
// ============================================================================

// ### CORRECTION: Remove old, invalid declaration ###
// extern esp_panel::board::Board *board;

static BatteryState BAT_State = BAT_OFF;
static uint32_t button_press_start = 0;

// Helper Functions
bool is_button_pressed(void)
{
    return (digitalRead(PWR_KEY_Input_PIN) == ButtonState::BUTTON_PRESSED);
}

// Helper function: Wait for button to be held for a specified duration (in ms)
bool wait_for_button_hold(uint16_t hold_ms)
{
    uint16_t wake_btn_hold_duration = 0;
    uint16_t required_count = hold_ms / 10;
    while (is_button_pressed())
    {
        vTaskDelay(10);
        wake_btn_hold_duration++;
        if (wake_btn_hold_duration >= required_count)
        {
            return true; // Button held long enough
        }
    }
    return false; // Released before required hold
}

void wake_up(void)
{
    pinMode(PWR_KEY_Input_PIN, INPUT);
    pinMode(PWR_Control_PIN, OUTPUT);
    
#ifdef BOARD_1_85C
    // C model: has a switch - power on immediately, no delay needed
    // (switch stays in position, no check required)
    digitalWrite(PWR_Control_PIN, HIGH);
    BAT_State = BAT_ON;
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    printf("[wake_up] Wakeup reason: %d (C model with switch - immediate)\n", wakeup_reason);
#else
    // Non-C model: has a button - 100ms safety delay
    // Prevents accidental power-on (e.g. in a backpack)
    digitalWrite(PWR_Control_PIN, LOW);
    vTaskDelay(100);
    if (!digitalRead(PWR_KEY_Input_PIN)) {
        BAT_State = BAT_ON;
        digitalWrite(PWR_Control_PIN, HIGH);
        esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
        printf("[wake_up] Wakeup reason: %d (non-C model with button - 100ms check)\n", wakeup_reason);
    }
#endif
}

void fall_asleep(void)
{
    // Power down display and touch
    Set_Backlight(0); // Turn off the backlight
    printf("[fall_asleep] Backlight OFF\n");
    
    digitalWrite(PWR_Control_PIN, LOW);
    printf("[fall_asleep] Display/touch power OFF\n");

    // Disable internal pullups/pulldowns on wake pin to reduce leakage
    pinMode(PWR_KEY_Input_PIN, INPUT);
    gpio_pulldown_dis((gpio_num_t)PWR_KEY_Input_PIN);
    gpio_pullup_dis((gpio_num_t)PWR_KEY_Input_PIN);

    // Enable wakeup on external pin
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PWR_KEY_Input_PIN, HIGH);
    printf("[fall_asleep] Entering deep sleep NOW\n");
    esp_deep_sleep_start();
}

void power_loop(void)
{
    if (BAT_State != BAT_OFF)
    {
        if (!digitalRead(PWR_KEY_Input_PIN))
        {
            if (BAT_State == BAT_READY_FOR_SLEEP)
            {
                if (button_press_start == 0)
                {
                    button_press_start = millis();
                }
                uint32_t held_time = millis() - button_press_start;
                if (held_time >= Device_Sleep_Time)
                {
                    fall_asleep();
                }
            }
        }
        else
        {
            if (BAT_State == BAT_ON)
                BAT_State = BAT_READY_FOR_SLEEP;
            button_press_start = 0;
        }
    }
}

void power_init(void)
{
    wake_up();
}

#else
// ============================================================================
// Boards without a power button / power-control MOSFET (Knob 1.8)
// ============================================================================
// The Knob board has neither PWR_KEY_Input_PIN nor PWR_Control_PIN (GPIO7 on
// that board is the encoder's B line - driving it as an output would fight
// the encoder's pull-up). power_init()/wake_up()/power_loop() are no-ops so
// nothing else has to change; fall_asleep() only turns the display off,
// since there is no reliable wake source on this board (deep sleep is not
// supported here). See docs/PORT_KNOB_1_8.md Step 6.

void wake_up(void)
{
    // No power key / power-control MOSFET on this board - nothing to do.
}

void power_init(void)
{
    // No power key on this board - nothing to do.
}

void power_loop(void)
{
    // No power key on this board - nothing to do.
}

void fall_asleep(void)
{
    printf("[Power] deep sleep not supported on this board\n");
    Set_Backlight(0);
    power_sleep_display();
}

#endif // HAS_POWER_KEY