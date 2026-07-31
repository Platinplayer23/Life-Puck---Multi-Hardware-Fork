#pragma once

// *** BOARD IS AUTO-SELECTED VIA THE PLATFORMIO ENVIRONMENT ***
// No manual editing needed - pick env:board_1_85C, env:board_1_85 or
// env:board_knob_1_8 in PlatformIO.
//
// This file is the single source of truth for every board-specific pin and
// capability macro. Other headers (display_st77916.h, touch_cst816.h,
// battery_state.h, power_key.h, simple_audio.h) alias their own macro names
// to the values defined here instead of hardcoding numbers.
//
// See docs/PORT_KNOB_1_8.md for the schematic-derived pin map and the
// reasoning behind the BOARD_KNOB_1_8 values.

#if defined(BOARD_1_85C)
    #define HAS_GPIO_EXTENDER  1
    #define HAS_POWER_KEY      1
    #define HAS_ENCODER        0

    // TCA9554 (GPIO expander)
    #define EXIO_PIN_SDA  8
    #define EXIO_PIN_SCL  17
    #define EXIO_PIN_INT  16

    // Reset pins via EXIO
    #define LCD_PIN_RST   -1
    #define TOUCH_PIN_RST -1

    // Touch: shares the main I2C bus (Wire)
    #define USE_SEPARATE_TOUCH_BUS 0

    // LCD QSPI pins
    #define LCD_PIN_SCK    40
    #define LCD_PIN_DATA0  46
    #define LCD_PIN_DATA1  45
    #define LCD_PIN_DATA2  42
    #define LCD_PIN_DATA3  41
    #define LCD_PIN_CS     21
    #define LCD_PIN_TE     18
    #define LCD_PIN_BL     5

    // Main I2C bus
    #define I2C_SDA_PIN    11
    #define I2C_SCL_PIN    10
    #define I2C_FREQ_HZ    100000  // Arduino's default - do not change, this
                                   // board is validated at 100kHz

    // Power key / power-control MOSFET
    #define PWR_KEY_Input_PIN 6
    #define PWR_Control_PIN   7

    // Battery ADC (LiPo cell via voltage divider)
    #define BAT_ADC_PIN     8
    #define BAT_ADC_SCALE   3.0f
    #define BAT_ADC_OFFSET  0.990476f

    // I2S audio (PCM5101 DAC)
    #define I2S_BCLK  48
    #define I2S_LRC   38
    #define I2S_DOUT  47

    #define BOARD_NAME "ESP32-S3-Touch-LCD-1.85C"

#elif defined(BOARD_1_85)
    #define HAS_GPIO_EXTENDER  1  // Demo shows: non-C variant ALSO has a GPIO expander!
    #define HAS_POWER_KEY      1
    #define HAS_ENCODER        0

    // TCA9554 (GPIO expander) - same as the C variant
    #define EXIO_PIN_SDA  8
    #define EXIO_PIN_SCL  17
    #define EXIO_PIN_INT  16

    // Reset pins (as shown by the vendor demo)
    #define LCD_PIN_RST   14  // Can still be driven directly
    #define TOUCH_PIN_RST -1  // Demo: touch reset goes through the EXIO

    // Touch: separate I2C bus (Wire1), as shown by the vendor demo
    #define USE_SEPARATE_TOUCH_BUS 1

    // LCD QSPI pins
    #define LCD_PIN_SCK    40
    #define LCD_PIN_DATA0  46
    #define LCD_PIN_DATA1  45
    #define LCD_PIN_DATA2  42
    #define LCD_PIN_DATA3  41
    #define LCD_PIN_CS     21
    #define LCD_PIN_TE     18
    #define LCD_PIN_BL     5

    // Main I2C bus
    #define I2C_SDA_PIN    11
    #define I2C_SCL_PIN    10
    #define I2C_FREQ_HZ    100000  // Arduino's default - do not change, this
                                   // board is validated at 100kHz

    // Power key / power-control MOSFET
    #define PWR_KEY_Input_PIN 6
    #define PWR_Control_PIN   7

    // Battery ADC (LiPo cell via voltage divider)
    #define BAT_ADC_PIN     8
    #define BAT_ADC_SCALE   3.0f
    #define BAT_ADC_OFFSET  0.990476f

    // I2S audio (PCM5101 DAC)
    #define I2S_BCLK  48
    #define I2S_LRC   38
    #define I2S_DOUT  47

    #define BOARD_NAME "ESP32-S3-Touch-LCD-1.85"

#elif defined(BOARD_KNOB_1_8)
    // Waveshare ESP32-S3-Knob-Touch-LCD-1.8
    #define HAS_GPIO_EXTENDER  0  // No TCA9554 on this board - all resets are direct GPIO
    #define HAS_POWER_KEY      0  // No power button, no power-control MOSFET
    #define HAS_ENCODER        1  // EC1 rotary encoder (not wired into the UI, see Step 9)

    // Reset pins: direct GPIO, no expander
    #define LCD_PIN_RST    21
    #define TOUCH_PIN_RST  10

    // Touch (CST816) and haptics (DRV2605L) share the main I2C bus (Wire)
    #define USE_SEPARATE_TOUCH_BUS 0

    // LCD QSPI pins
    #define LCD_PIN_SCK    13
    #define LCD_PIN_DATA0  15
    #define LCD_PIN_DATA1  16
    #define LCD_PIN_DATA2  17
    #define LCD_PIN_DATA3  18
    #define LCD_PIN_CS     14
    #define LCD_PIN_TE     -1  // Not connected on this board (marked X on the schematic)
    #define LCD_PIN_BL     47

    // Main I2C bus (touch CST816 @ 0x15, haptics DRV2605L @ 0x5A)
    #define I2C_SDA_PIN    11
    #define I2C_SCL_PIN    12
    #define I2C_FREQ_HZ    400000  // 5.1k pull-ups to 3V3, CST816 is rated for 400kHz

    // Battery ADC measures half of the 5V system rail, NOT the LiPo cell -
    // see docs/PORT_KNOB_1_8.md section 1 for why percentage/dimming/
    // critical-battery shutdown are meaningless on this board.
    #define BAT_ADC_PIN     1
    #define BAT_ADC_SCALE   2.0f
    #define BAT_ADC_OFFSET  1.0f

    // I2S audio (PCM5100A DAC), routed through the CH445P 2:1 analog mux
    #define I2S_BCLK  39
    #define I2S_LRC   40
    #define I2S_DOUT  41
    #define I2S_SWITCH_PIN 0  // CH445P mux select; HIGH routes the S3's I2S to the DAC

    // Rotary encoder EC1 (out of scope for this port, see Step 9)
    #define ENCODER_PIN_A  8
    #define ENCODER_PIN_B  7

    // PWR_KEY_Input_PIN / PWR_Control_PIN intentionally NOT defined:
    // this board has neither a power button nor a power-control MOSFET.

    #define BOARD_NAME "ESP32-S3-Knob-Touch-LCD-1.8"

#else
    #error "No board defined! Select env:board_1_85C, env:board_1_85 or env:board_knob_1_8 in PlatformIO!"
#endif

// ========================================
// SHARED DEFINES (SAME FOR ALL BOARDS)
// ========================================
#define LCD_WIDTH  360
#define LCD_HEIGHT 360

// ========================================
// Touch pins (differ per board)
// ========================================
#if defined(BOARD_1_85C)
    // C variant: touch on the main I2C bus
    #define TOUCH_PIN_SDA  I2C_SDA_PIN  // 11
    #define TOUCH_PIN_SCL  I2C_SCL_PIN  // 10
    #define TOUCH_PIN_INT  4
#elif defined(BOARD_1_85)
    // Non-C variant: touch has a separate I2C bus (vendor demo values!)
    #define TOUCH_PIN_SDA  1   // Demo: CST816_SDA_PIN = 1
    #define TOUCH_PIN_SCL  3   // Demo: CST816_SCL_PIN = 3
    #define TOUCH_PIN_INT  4
#elif defined(BOARD_KNOB_1_8)
    // Touch on the main I2C bus - same pins as I2C_SDA_PIN / I2C_SCL_PIN
    #define TOUCH_PIN_SDA  I2C_SDA_PIN  // 11
    #define TOUCH_PIN_SCL  I2C_SCL_PIN  // 12
    #define TOUCH_PIN_INT  9
#endif
