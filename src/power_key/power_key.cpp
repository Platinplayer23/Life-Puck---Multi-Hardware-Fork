#include "power_key.h"
#include "shutdown/shutdown.h"
#include <lvgl.h>
#include <esp_sleep.h>
#include <Arduino.h>
#include <state/state_store.h>
#include <constants/constants.h>

// ### KORREKTUR: Include für den neuen Treiber hinzufügen ###
#include "Display_ST77916.h"

// ### KORREKTUR: Alte, ungültige Deklaration entfernen ###
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
    digitalWrite(PWR_Control_PIN, LOW);
    vTaskDelay(100);
    if (!digitalRead(PWR_KEY_Input_PIN))
    {
        BAT_State = BAT_ON;
        digitalWrite(PWR_Control_PIN, HIGH);
        esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
        printf("[wake_up] Wakeup reason: %d\n", wakeup_reason);
        vTaskDelay(300);
    }
}

void fall_asleep(void)
{
    // Power down display and touch
    // ### KORREKTUR: Alten Code durch neuen Treiber-Aufruf ersetzen ###
    Set_Backlight(0); // Schaltet die Hintergrundbeleuchtung aus
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