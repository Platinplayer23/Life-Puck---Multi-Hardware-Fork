#pragma once

/**
 * @file touch_calibration.h
 * @brief Interactive touch calibration system
 *
 * Since I couldn't find any similar approach in existing implementations,
 * I'm calling this "Suthe's Method" - a centre-anchored interactive
 * calibration technique with automatic parameter adjustment.
 *
 * The screen centre is measured directly (average raw touch while the
 * user holds a dot there), and horizontal/vertical scale are then tuned
 * with a proportional-feedback loop against edge targets. Anchoring the
 * scale at the measured centre - instead of at the raw touch-panel origin -
 * means changing scale no longer moves the centre, so offset and scale
 * cannot fight each other the way they used to.
 *
 * See docs/Suthes_Method_Touch_Calibration.md for full documentation.
 */

/**
 * @brief Start the interactive touch calibration process
 *
 * Launches the centre-anchored calibration interface: measures the raw
 * touch position at the screen centre directly, then automatically tunes
 * horizontal and vertical scale with a proportional-feedback loop against
 * edge targets, and finally shows a residual-error readout for the user
 * to judge the result before saving.
 */
void renderTouchCalibrationScreen();

/**
 * @brief Cleanup touch calibration screen
 */
void teardownTouchCalibrationScreen();

/**
 * @brief Reset touch calibration to factory defaults
 * 
 * Emergency function to restore default touch settings.
 */
void resetTouchCalibrationToDefaults();

/**
 * @brief Load and apply saved touch calibration from NVS
 * 
 * Called during boot to restore previously calibrated touch settings.
 */
void loadTouchCalibrationFromNVS();

/**
 * @brief Check if touch calibration needs confirmation after boot
 * 
 * @return true if confirmation dialog should be shown
 */
bool needsTouchCalibrationConfirmation();

/**
 * @brief Render touch calibration confirmation screen
 */
void renderTouchCalibrationConfirmation();

/**
 * @brief Cleanup touch calibration confirmation screen
 */
void teardownTouchCalibrationConfirmation();

/**
 * @brief Confirm the current touch calibration
 */
void confirmTouchCalibration();
