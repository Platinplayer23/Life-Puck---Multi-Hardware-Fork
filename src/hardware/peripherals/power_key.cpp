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
#include "../system/battery_state.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

// ============================================
// Board Configuration
// ============================================
#include "board_config.h"

// Static variable to store wake-up reason (for debugging)
static esp_sleep_wakeup_cause_t stored_wakeup_reason = ESP_SLEEP_WAKEUP_UNDEFINED;
static unsigned long last_wakeup_reason_print = 0;

// Button state logging
#define BUTTON_LOG_SIZE 50
static int button_log[BUTTON_LOG_SIZE];
static unsigned long button_log_time[BUTTON_LOG_SIZE];
static int button_log_index = 0;
static bool button_log_enabled = false;
static unsigned long last_button_print = 0;

static BatteryState BAT_State = BAT_OFF;
static uint32_t button_press_start = 0;

bool is_button_pressed(void)
{
    return (digitalRead(PWR_KEY_Input_PIN) == ButtonState::BUTTON_PRESSED);
}

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
            return true;
        }
    }
    return false;
}

void wake_up(void)
{
    pinMode(PWR_KEY_Input_PIN, INPUT);
    pinMode(PWR_Control_PIN, OUTPUT);
    
#ifdef BOARD_1_85C
    // C-Model: Hardware switch connects battery to ESP
    // Simple logic: Check battery voltage only
    // Battery >= 3.2V = Switch ON → ESP power on/stay on
    // Battery < 3.2V = Switch OFF → Deep Sleep
    float battery_volts = battery_get_volts();
    bool switch_on = (battery_volts >= 3.2);
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        // Timer wake-up: Re-read battery voltage
        battery_volts = battery_get_volts();
        switch_on = (battery_volts >= 3.2);
        
        if (switch_on) {
            digitalWrite(PWR_Control_PIN, HIGH);
            BAT_State = BAT_ON;
            return;
        } else {
            BAT_State = BAT_OFF;
            digitalWrite(PWR_Control_PIN, LOW);
            gpio_pulldown_dis((gpio_num_t)PWR_KEY_Input_PIN);
            gpio_pullup_dis((gpio_num_t)PWR_KEY_Input_PIN);
            esp_sleep_enable_timer_wakeup(5 * 1000000);
            esp_deep_sleep_start();
            return;
        }
    }
    
    if (switch_on) {
        digitalWrite(PWR_Control_PIN, HIGH);
        BAT_State = BAT_ON;
    } else {
        BAT_State = BAT_OFF;
        digitalWrite(PWR_Control_PIN, LOW);
        gpio_pulldown_dis((gpio_num_t)PWR_KEY_Input_PIN);
        gpio_pullup_dis((gpio_num_t)PWR_KEY_Input_PIN);
        esp_sleep_enable_timer_wakeup(5 * 1000000);
        esp_deep_sleep_start();
    }
#else
    // Non-C-Model: ESP controls power via PWR_Control_PIN (GPIO 7)
    // ESP always runs (even with USB), display/touch controlled via PWR_Control_PIN
    // When "off", ESP is in deep sleep (not completely off like C-Model)
    // Check wake-up reason to distinguish button press from USB power
    
    // CRITICAL: Read wake-up reason IMMEDIATELY, before any GPIO operations!
    // If we configure GPIO pins first, the wake-up reason might be corrupted
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    stored_wakeup_reason = wakeup_reason;
    
    // Reset button log
    button_log_index = 0;
    button_log_enabled = true;
    
    // Now configure GPIO pins
    pinMode(PWR_KEY_Input_PIN, INPUT);
    pinMode(PWR_Control_PIN, OUTPUT);
    digitalWrite(PWR_Control_PIN, LOW);
    
    // Start logging button state immediately
    unsigned long start_time = millis();
    for (int i = 0; i < 20; i++) {
        int btn_state = digitalRead(PWR_KEY_Input_PIN);
        button_log[button_log_index] = btn_state;
        button_log_time[button_log_index] = millis() - start_time;
        button_log_index = (button_log_index + 1) % BUTTON_LOG_SIZE;
        delay(10);
    }
    
    // Wait for Serial Monitor to connect
    delay(2000);
    
    // Debug: Print wake-up reason and button log (now that Serial Monitor is connected)
    printf("\n[wake_up] Non-C: WakeReason=%d (read immediately at start)\n", wakeup_reason);
    printf("[wake_up] Non-C: Button log during wake-up (first 20 readings):\n");
    for (int i = 0; i < button_log_index && i < 20; i++) {
        printf("  [%lu ms] Button=%d\n", button_log_time[i], button_log[i]);
    }
    Serial.flush();
    delay(500);
    
    // ESP_SLEEP_WAKEUP_UNDEFINED = power-on reset (USB plugged in or first boot)
    // → Go to sleep IMMEDIATELY, don't check button (button can't be read when device is off anyway)
    if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED)
    {
        printf("[wake_up] Non-C: USB power-on detected - going to deep sleep immediately\n");
        Serial.flush();
        delay(2000); // Wait for Serial Monitor to see output
        fall_asleep();
        return; // Exit early, don't check button
    }
    
    // ESP_SLEEP_WAKEUP_EXT0 = woken by power button (LOW = button pressed)
    // → Check button state to verify it was a real button press
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
    {
        // Configure button pin - no pull-up/pull-down (external pull-up is active)
        gpio_pulldown_dis((gpio_num_t)PWR_KEY_Input_PIN);
        gpio_pullup_dis((gpio_num_t)PWR_KEY_Input_PIN);
        delay(100); // Wait for pin to stabilize

        // Read button state immediately - if HIGH, it's a false wake-up (button not pressed)
        int button_state_immediate = digitalRead(PWR_KEY_Input_PIN);
        
        // If button is HIGH immediately after EXT0 wake-up, it's a false positive
        // (EXT0 was triggered by USB power-on spike, not by button press)
        if (button_state_immediate == HIGH)
        {
            printf("[wake_up] Non-C: EXT0 wake-up but Button HIGH (not pressed) - False wake-up, going to sleep\n");
            Serial.flush();
            delay(2000);
            fall_asleep();
            return;
        }
        
        // Button is LOW (pressed) - read multiple times to debounce
        int button_state_1 = digitalRead(PWR_KEY_Input_PIN);
        delay(50);
        int button_state_2 = digitalRead(PWR_KEY_Input_PIN);
        delay(50);
        int button_state_3 = digitalRead(PWR_KEY_Input_PIN);
        
        // Button state is consistent if all readings are the same
        bool button_consistent = (button_state_1 == button_state_2) && (button_state_2 == button_state_3);
        int button_state = button_state_2; // Use middle reading
        
        // Debug: Print button state
        printf("[wake_up] Non-C: ButtonState=%d (readings: %d,%d,%d, consistent=%d, LOW=pressed)\n", 
               button_state, button_state_1, button_state_2, button_state_3, button_consistent);
        Serial.flush();
        delay(500);
        
        // Only boot if button is consistently LOW (pressed)
        // If EXT0 was triggered and button is LOW, it means button was pressed - power on!
        if (button_state == LOW && button_consistent)
        {
            // Button is pressed - power on immediately (user pressed button to wake up)
            printf("[wake_up] Non-C: EXT0 + Button pressed (LOW) - Power ON\n");
            Serial.flush();
            BAT_State = BAT_ON;
            digitalWrite(PWR_Control_PIN, HIGH);
            delay(300);
            return; // Exit, device is now on
        }
        else
        {
            // EXT0 wake-up but button not LOW or inconsistent - false wake-up, go back to sleep
            printf("[wake_up] Non-C: EXT0 but Button not LOW or inconsistent - False wake-up, Deep Sleep (Button=%d, Consistent=%d)\n", 
                   button_state, button_consistent);
            Serial.flush();
            delay(2000); // Wait for Serial Monitor to see output
            fall_asleep();
            return;
        }
    }
    
    // All other wake-up reasons (TIMER, EXT1, etc.) - go to sleep immediately
    printf("[wake_up] Non-C: Other wake-up reason (%d) - going to deep sleep\n", wakeup_reason);
    Serial.flush();
    delay(2000); // Wait for Serial Monitor to see output
    fall_asleep();
#endif
}

void fall_asleep(void)
{
    Set_Backlight(0);
    digitalWrite(PWR_Control_PIN, LOW);

#ifdef BOARD_1_85C
    pinMode(PWR_KEY_Input_PIN, INPUT);
    gpio_pulldown_dis((gpio_num_t)PWR_KEY_Input_PIN);
    gpio_pullup_dis((gpio_num_t)PWR_KEY_Input_PIN);
    // C-Model: Timer-based wake-up (check switch every 5 seconds)
    esp_sleep_enable_timer_wakeup(5 * 1000000);
#else
    // Non-C-Model: Wake on button press
    // Button pin has external pull-up (pin is HIGH when button not pressed)
    // Configure button pin - disable pull-up/pull-down (external pull-up is active)
    pinMode(PWR_KEY_Input_PIN, INPUT);
    gpio_pulldown_dis((gpio_num_t)PWR_KEY_Input_PIN);
    gpio_pullup_dis((gpio_num_t)PWR_KEY_Input_PIN);
    
    // Debug: Print IMMEDIATELY (before any delays or operations)
    printf("\n[fall_asleep] Non-C: Entering deep sleep...\n");
    Serial.flush();
    
    // Wait a bit to ensure button is released
    delay(200);
    
    // Check button state before configuring wake-up
    int button_before_sleep = digitalRead(PWR_KEY_Input_PIN);
    printf("[fall_asleep] Non-C: Button state before sleep: %d (HIGH=not pressed, LOW=pressed)\n", button_before_sleep);
    Serial.flush();
    delay(500);
    
    // Disable all wakeup sources first (including any timer wake-ups)
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    
    // Enable wakeup on external pin (power button only)
    // Wake on LOW = button is pressed (pin goes from HIGH to LOW)
    // Button has external pull-up, so pin is HIGH when button not pressed
    // When button is pressed, pin goes LOW, triggering wake-up
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PWR_KEY_Input_PIN, LOW);
    printf("[fall_asleep] Non-C: Configured EXT0 wake-up on pin %d (LOW = button pressed)\n", PWR_KEY_Input_PIN);
    Serial.flush();
    delay(2000); // Wait for Serial Monitor to see output
    
    // Configure power domains for minimum consumption during sleep
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
#endif
    esp_deep_sleep_start();
}

void power_loop(void)
{
#ifdef BOARD_1_85C
    // C-Model: Hardware switch, check battery voltage only
    // Battery >= 3.2V = Switch ON → ESP runs/stays on
    // Battery < 3.2V = Switch OFF → Deep Sleep
    float battery_volts = battery_get_volts();
    bool switch_on = (battery_volts >= 3.2);
    
    if (BAT_State == BAT_ON || BAT_State == BAT_READY_FOR_SLEEP) {
        if (!switch_on) {
            fall_asleep();
        } else {
            if (BAT_State == BAT_ON)
                BAT_State = BAT_READY_FOR_SLEEP;
        }
    } else if (BAT_State == BAT_OFF) {
        // Should have gone to deep sleep in wake_up() if switch OFF
        // If not, check again
        if (!switch_on) {
            fall_asleep();
        }
    }
#else
    // Non-C-Model: ESP controls power via PWR_Control_PIN
    // Print stored wake-up reason every 5 seconds (so Serial Monitor can see it)
    unsigned long current_time = millis();
    if (current_time - last_wakeup_reason_print > 5000) {
        printf("[power_loop] Non-C: Stored WakeReason=%d\n", stored_wakeup_reason);
        Serial.flush();
        last_wakeup_reason_print = current_time;
    }
    
    // Continuously log button state (every 100ms)
    if (current_time - last_button_print > 100) {
        int btn_state = digitalRead(PWR_KEY_Input_PIN);
        if (button_log_enabled) {
            button_log[button_log_index] = btn_state;
            button_log_time[button_log_index] = current_time;
            button_log_index = (button_log_index + 1) % BUTTON_LOG_SIZE;
        }
        printf("[power_loop] Non-C: Button=%d (HIGH=not pressed, LOW=pressed), BAT_State=%d\n", btn_state, BAT_State);
        Serial.flush();
        last_button_print = current_time;
    }
    
    // Only check button if device is on
    if (BAT_State != BAT_OFF) {
        if (!digitalRead(PWR_KEY_Input_PIN)) {
            if (BAT_State == BAT_READY_FOR_SLEEP) {
                if (button_press_start == 0) {
                    button_press_start = millis();
                    printf("[power_loop] Non-C: Button pressed, starting sleep timer\n");
                    Serial.flush();
                }
                uint32_t held_time = millis() - button_press_start;
                if (held_time >= Device_Sleep_Time) {
                    printf("[power_loop] Non-C: Button held for %d ms, calling fall_asleep()\n", held_time);
                    Serial.flush();
                    delay(500); // Give time for output
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
#endif
}

void power_init(void)
{
    wake_up();
}