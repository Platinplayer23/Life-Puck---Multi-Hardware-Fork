#pragma once
#include <Arduino.h>
#include "board_config.h"

// BAT_ADC_PIN, BAT_ADC_SCALE and BAT_ADC_OFFSET are board-specific and
// defined in board_config.h. On the Knob 1.8 board the ADC reads half of
// the 5V system rail rather than the LiPo cell - see
// docs/PORT_KNOB_1_8.md section 1 for details.

/// Current battery voltage reading (updated by battery functions)
extern float BAT_analogVolts;

/**
 * @brief Initialize battery monitoring system
 * 
 * Configures ADC pin and sets up initial battery state.
 * Call once during system initialization.
 */
void battery_init(void);

/**
 * @brief Get current battery voltage
 * @return Battery voltage in volts (float)
 */
float battery_get_volts(void);

/**
 * @brief Get current battery charge percentage
 * @return Battery charge level (0.0-100.0%)
 */
float battery_get_percent(void);