/**
 * @file i2c_scanner.h
 * @brief Boot-time I2C bus diagnostics
 *
 * Scans the I2C bus/buses for responding devices. Intended to make a
 * pinout mismatch (e.g. the wrong board target flashed onto a device)
 * self-diagnosing in the serial log instead of surfacing only as a
 * cryptic low-level I2C error.
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * @brief Scan the I2C bus/buses (0x08..0x77) and print every address found.
 *
 * Scans the main I2C bus (Wire), and also Wire1 when the board uses a
 * separate touch I2C bus (USE_SEPARATE_TOUCH_BUS, see board_config.h).
 */
void I2C_Scan(void);

/**
 * @brief Probe a single I2C address on the main bus (Wire) for an ACK.
 * @param addr 7-bit I2C address to probe
 * @return true if a device acknowledged the address, false otherwise
 */
bool I2C_ProbeDevice(uint8_t addr);
