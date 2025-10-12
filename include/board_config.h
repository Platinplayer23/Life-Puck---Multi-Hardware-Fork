#pragma once

// *** AUTO-DETECT VIA PLATFORMIO ***
// Kein manuelles Kommentieren mehr!

#if defined(BOARD_1_85C)
    #define HAS_GPIO_EXTENDER  1
    
    // TCA9554 (GPIO Extender)
    #define EXIO_PIN_SDA  8
    #define EXIO_PIN_SCL  17
    #define EXIO_PIN_INT  16
    
    // Reset Pins via EXIO
    #define LCD_PIN_RST   -1
    #define TOUCH_PIN_RST -1
    
    #define BOARD_NAME "ESP32-S3-Touch-LCD-1.85C"
    
#elif defined(BOARD_1_85)
    #define HAS_GPIO_EXTENDER  0
    
    // Reset Pins DIREKT
    #define LCD_PIN_RST   14
    #define TOUCH_PIN_RST 13
    
    #define BOARD_NAME "ESP32-S3-Touch-LCD-1.85"
    
#else
    #error "Kein Board definiert! Wähle env:board_1_85C oder env:board_1_85 in PlatformIO!"
#endif

// ========================================
// GEMEINSAME DEFINES (BEIDE GLEICH!)
// ========================================
#define LCD_WIDTH  360
#define LCD_HEIGHT 360

// LCD Pins
#define LCD_PIN_CS     21
#define LCD_PIN_SCK    40
#define LCD_PIN_DATA0  46
#define LCD_PIN_DATA1  45
#define LCD_PIN_DATA2  42
#define LCD_PIN_DATA3  41
#define LCD_PIN_TE     18
#define LCD_PIN_BL     5

// Touch Pins
#define TOUCH_PIN_SDA  11
#define TOUCH_PIN_SCL  10
#define TOUCH_PIN_INT  4
