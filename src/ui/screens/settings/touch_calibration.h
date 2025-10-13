#pragma once

#include <lvgl.h>

/**
 * @brief Render touch calibration screen
 * 
 * Creates a full-screen calibration interface that guides the user
 * through a 5-point calibration process with fail-safe mechanisms.
 */
void renderTouchCalibrationScreen();

/**
 * @brief Cleanup touch calibration screen
 * 
 * Properly destroys the calibration screen and frees memory.
 */
void teardownTouchCalibrationScreen();

/**
 * @brief Reset touch calibration to factory defaults
 * 
 * Emergency function to restore default touch settings if calibration
 * goes wrong and device becomes unusable.
 */
void resetTouchCalibrationToDefaults();

// Emergency BOOT button reset removed - conflicts with ESP32 download mode

/**
 * @brief Check if touch calibration needs confirmation after boot
 * 
 * Call this after UI is initialized to show confirmation dialog if needed.
 * @return true if confirmation dialog should be shown
 */
bool needsTouchCalibrationConfirmation();

/**
 * @brief Confirm the current touch calibration
 * 
 * Removes the pending confirmation flag, making calibration permanent.
 */
void confirmTouchCalibration();

/**
 * @brief Render touch calibration confirmation screen
 * 
 * Shows a 10-second countdown dialog asking user to confirm new calibration.
 * Auto-reverts to defaults if no confirmation received.
 */
void renderTouchCalibrationConfirmation();

/**
 * @brief Cleanup touch calibration confirmation screen
 */
void teardownTouchCalibrationConfirmation();

/**
 * @brief Load and apply saved touch calibration from NVS
 * 
 * Called during boot to restore previously calibrated touch settings.
 * Must be called after LVGL initialization but before UI creation.
 */
void loadTouchCalibrationFromNVS();
