# Life Puck - Multi-Hardware Fork

[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![LVGL](https://img.shields.io/badge/LVGL-9.3.0-green.svg)](https://lvgl.io/)
[![License](https://img.shields.io/github/license/jontiritilli/life-puck)](LICENSE)

**A digital life counter for Magic: The Gathering, Yu-Gi-Oh!, Pokémon TCG, and other tabletop games.**

This is a fork of [jontiritilli/life-puck](https://github.com/jontiritilli/life-puck) with extended hardware support for multiple Waveshare ESP32-S3 display variants.

---

## 📋 Table of Contents

- [Hardware Support](#-hardware-support)
- [New Features](#-new-features-in-this-fork)
- [Software Features](#-software-features)
- [Getting Started](#-getting-started)
- [Installation Guide](#-installation-guide)
- [Touch Calibration](#-touch-calibration)
- [Adding Custom Presets](#-adding-custom-presets)
- [Usage Guide](#-usage-guide)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [Credits](#-credits)
- [License](#-license)

---

## 🔧 Hardware Support

This fork supports **two** Waveshare ESP32-S3 display variants:

### Supported Boards

| Board | Display | Touch | GPIO Expander | AliExpress Bundle |
|-------|---------|-------|---------------|-------------------|
| **[ESP32-S3-Touch-LCD-1.85C](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85C)** | ST77916 (QSPI) | CST816 | ✅ TCA9554 | ✅ **Recommended** (includes case + battery) |
| **[ESP32-S3-Touch-LCD-1.85](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)** | ST77916 (QSPI) | CST816 | ❌ None | ❌ More expensive |

> **💡 Cost Tip:** The **1.85C variant** (at least in Europe) is significantly cheaper on AliExpress when purchased as a bundle with case and battery!

### Hardware Specifications

- **MCU:** ESP32-S3-WROOM-1 (N16R8) - 16MB Flash, 8MB PSRAM
- **Display:** 1.85" Round LCD, 360x360px, ST77916 driver (QSPI mode)
- **Touch:** Capacitive touch screen, CST816 controller
- **Connectivity:** USB-C, Wi-Fi, Bluetooth

---

## 🆕 New Features in This Fork

### Hardware Support
- ✅ **Dual board support**: ESP32-S3-Touch-LCD-1.85C and 1.85
- ✅ **GPIO Expander handling**: Automatic detection and configuration for TCA9554
- ✅ **Unified codebase**: Single project for both hardware variants

### Build System Improvements
- ✅ **Arduino 3.x support**: Uses latest ESP-IDF 5.3 based framework via [pioarduino](https://github.com/pioarduino/platform-espressif32)
- ✅ **Multi-environment builds**: Switch between boards with `platformio.ini` environments
- ✅ **Modern LVGL 9.3.0**: Updated graphics library with performance improvements

### New Game Features
- ✅ **🎲 Dice Roller**: Roll D4, D6, D8, D10, D12, and D20
- ✅ **🪙 Coin Flip**: Coin flip for random heads/tails decisions
- ✅ **⚙️ Game Presets**: Pre-configured settings for popular TCGs (MTG, Yu-Gi-Oh!, Pokémon, FaB, Lorcana, One Piece)
- ✅ **✏️ Preset Editor**: Create and save custom game configurations
- ✅ **⏱️ Round Timer**: Track round duration with stopwatch and countdown modes
- ✅ **📊 Enhanced Timer**: Switchable timer modes (stopwatch/countdown) with configurable round times

### User Experience
- ✅ **Comprehensive documentation**: Detailed setup and configuration guides
- ✅ **Preset system**: Pre-compile custom game presets to avoid on-device keyboard


---

## 📱Software Features

### Core Life Tracking
- **One and Two Player Modes**: Track life totals for solo or head-to-head games
- **Configurable Base Life**: Set starting life from 1-999999
- **Increment/Decrement Steps**: Adjust life in small (tap) or large (swipe) steps
- **Life Change History**: Track all life changes

### Game Utilities

#### 🎲 Dice Roller
- Roll standard RPG/TCG dice (D4, D6, D8, D10, D12, D20)
- Clear result display
- Accessible from main menu

#### 🪙 Coin Flip
- 50/50 heads/tails outcomes
- Perfect for tiebreakers and random decisions

#### ⏱️ Timer Modes
- **Stopwatch Mode**: Count up from 00:00 to track game duration
- **Countdown Mode**: Set a round time and count down with visual progress
- **Persistent Timer**: Continues running across screen changes
- **Quick Controls**: Tap to start/pause, long-press to reset

#### ⚙️ Game Presets
Pre-configured settings for popular games:
- **MTG Standard** - 20 life, ±1/±5 steps
- **MTG Commander (EDH)** - 40 life, ±1/±10 steps
- **Pokémon TCG** - 60 life, ±10/±30 steps (Prize cards)
- **Yu-Gi-Oh!** - 8000 LP, ±50/±500 steps
- **Custom 5-10** - Customizable presets (default: 20 life, ±1/±5)

#### ✏️ Preset Editor
- Create custom game configurations
- Set starting life, step size, and timer defaults
- Name your presets (up to 15 characters)
- Save to non-volatile storage
- Quick-select from game menu

### User Interface
- **LVGL-based UI**: Hardware-accelerated animations
- **Intuitive Touch Controls**: Tap, swipe, and long-press gestures
- **In-app Configuration**: Change all settings without recompiling
- **Visual Feedback**: Animated responses to all interactions
- **Auto-brightness**: Adjustable display brightness to save battery
- **Sleep Mode**: Auto-sleep after configurable inactivity period

### State Management
- **Robust State Persistence**: All settings saved to NVS
- **History Tracking**: Persistent life change log until Reset
- **Undo/Redo Support**: Step back through life changes
- **Session Recovery**: Resume game state after restart

---

## 🚀 Getting Started

### Prerequisites

- **Operating System:** Windows, macOS, or Linux
- **IDE:** Visual Studio Code (recommended)
- **Knowledge:** Basic understanding of C++ and embedded systems
- **Hardware:** One of the supported Waveshare ESP32-S3 boards

### Required Software

1. **Visual Studio Code**: [Download](https://code.visualstudio.com/)
2. **PlatformIO Extension**: Install from VS Code marketplace
3. **USB Drivers**: CH340 (usually auto-installed)

---

## 📦 Installation Guide

### Step 1: Install Visual Studio Code

1. Download VS Code from [https://code.visualstudio.com/](https://code.visualstudio.com/)
2. Install and launch VS Code

### Step 2: Install PlatformIO

1. Open VS Code
2. Go to **Extensions** (Ctrl+Shift+X)
3. Search for **"PlatformIO IDE"**
4. Click **Install**
5. Restart VS Code when prompted

### Step 3: Clone This Repository

**Option A: Using Git (recommended)**

git clone https://github.com/Platinplayer23/Life-Puck---Multi-Hardware-Fork.git
cd Life-Puck---Multi-Hardware-Fork-master


**Option B: Download ZIP**
1. Click the green **"Code"** button on GitHub
2. Select **"Download ZIP"**
3. Extract the ZIP file

### Step 4: Open Project in VS Code

1. Open VS Code
2. File → Open Folder
3. Select the `Life-Puck---Multi-Hardware-Fork-master` folder
4. PlatformIO will automatically detect the project

### Step 5: Configure for Your Board

Just switch between the Project Environments

### Step 6: Build and Upload

1. Connect your ESP32-S3 board via USB-C
2. Press **Ctrl+Alt+U** (Cmd+Alt+U on Mac)
3. Wait for compilation and upload to complete

**Alternative: Using VS Code Upper Bar**
- Click the **→** (Upload) icon in the upper bar

### Step 7: Verify Upload

1. Press **Ctrl+Shift+P**
2. Type **"PlatformIO: Serial Monitor"**
3. You should see debug output at 115200 baud

---

## 🎯 Touch Calibration

If touch inputs are misaligned or unresponsive, adjust the calibration in `src/LVGL_Driver.cpp`.

### Adjusting Touch Scaling

Edit the `Lvgl_Touchpad_Read` function in `src/LVGL_Driver.cpp`:

void Lvgl_Touchpad_Read(lv_indev_t *indev, lv_indev_data_t *data) {  
Touch_Read_Data();  
if (touch_data.points != 0) {  
// TOUCH SCALING CORRECTION  
float scale_x = 0.85f; // Adjust: try 0.90, 0.95, 1.0, 1.05  
float scale_y = 1.0f; // Adjust: try 0.90, 0.95, 1.0, 1.05  

text
    int center_x = LCD_WIDTH / 2;
    int center_y = LCD_HEIGHT / 2;
    
    // Scale touch coordinates from center
    int dx = touch_data.x - center_x;
    int dy = touch_data.y - center_y;
    
    int corrected_x = center_x + (int)(dx * scale_x);
    int corrected_y = center_y + (int)(dy * scale_y);
    
    // Clamp to screen bounds
    if (corrected_x < 0) corrected_x = 0;
    if (corrected_y < 0) corrected_y = 0;
    if (corrected_x >= LCD_WIDTH) corrected_x = LCD_WIDTH - 1;
    if (corrected_y >= LCD_HEIGHT) corrected_y = LCD_HEIGHT - 1;
    
    data->point.x = corrected_x;
    data->point.y = corrected_y;
    data->state = LV_INDEV_STATE_PRESSED;
} else {
    data->state = LV_INDEV_STATE_RELEASED;
}

touch_data.points = 0;
touch_data.gesture = NONE;

}

text

### Calibration Values

**scale_x / scale_y:**
- `< 1.0`: Touch area shrinks toward center (fixes edge overshooting)
- `= 1.0`: No scaling (1:1 mapping)
- `> 1.0`: Touch area expands (fixes not reaching edges)

**Common Issues:**

| Problem | Solution | Example Value |
|---------|----------|---------------|
| Touch overshoots edges | Reduce scale | `scale_x = 0.85f` |
| Can't reach edges | Increase scale | `scale_x = 1.05f` |
| Left/right offset | Adjust X scale only | `scale_x = 0.90f` |
| Up/down offset | Adjust Y scale only | `scale_y = 1.02f` |

### Testing Touch Calibration

**Best method:** Use the **Preset Editor** keyboard

1. Go to **Settings → Edit Presets**
2. Tap on any Preset
3. The on-screen keyboard appears
4. Test tapping different keys (especially corners and edges)
5. If keys are unresponsive or wrong keys register, adjust scaling
6. Rebuild and re-upload firmware
7. Repeat until keyboard feels accurate

**Why the keyboard?**
- Has buttons in all screen regions (center, edges, corners)
- Provides immediate visual feedback
- Small keys reveal misalignment quickly
- Most challenging UI element to use

### Step-by-Step Calibration Process

1. **Test current calibration:**

pio run -t upload && pio device monitor


2. **Open Preset Editor keyboard:**
- Navigate: Main Screen → Settings → Edit Presets → Add Preset
- Try typing on the keyboard

3. **Identify the problem:**
- **Buttons don't respond:** Increase scale (e.g., `1.05f`)
- **Wrong buttons activate:** Decrease scale (e.g., `0.90f`)
- **Only X-axis wrong:** Adjust `scale_x` only
- **Only Y-axis wrong:** Adjust `scale_y` only

4. **Adjust values in `LVGL_Driver.cpp`:**

float scale_x = 0.85f; // Your adjusted value
float scale_y = 1.0f; // Your adjusted value


5. **Rebuild and test:**

pio run -t upload


6. **Repeat** until keyboard works reliably

### Example Calibration Values

**ESP32-S3-Touch-LCD-1.85C:**

float scale_x = 0.85f; // Tested working value
float scale_y = 1.0f;


**ESP32-S3-Touch-LCD-1.85:**

float scale_x = 1.0f; // May vary per device
float scale_y = 1.0f;


> **Note:** These values may vary between individual displays. Always test on your specific hardware.

### Alternative: Runtime Calibration (Future Feature)

On-device calibration UI will be added in a future update. For now, compile-time calibration is the recommended method.

---

## ⚙️ Adding Custom Presets
You can change the presets via the on-device Menu  
But the on-device keyboard can be difficult to use. Instead you can, add custom game presets before compilation.

### Step 1: Locate Preset File

Edit the preset initialization function in **`src/tcg_presets.cpp`**:


### Step 2: Add Your Custom Game

**Example: Custom Slot**

Just fill in the Game Data:

    // ---- SLOT 7: Custom ----
    strncpy(TCG_PRESETS[7].name, "Custom 8", sizeof(TCG_PRESETS[7].name) - 1);
    TCG_PRESETS[7].starting_life = 20;
    TCG_PRESETS[7].small_step = 1;
    TCG_PRESETS[7].large_step = 5;


First Line is just the Name as a comment keep the //  
Second Line replace "Custom 8" with "Name of Your Game"  
And for the Rest just change the numbers according to your needs
### Preset Format

TCG_PRESETS[index].name // Display name (max 15 characters)  
TCG_PRESETS[index].starting_life // Initial life total (1-999999)  
TCG_PRESETS[index].small_step // Small increment (tap)  
TCG_PRESETS[index].large_step // Large increment (swipe)  


### Common TCG Presets



**Cardfight!! Vanguard:**

strncpy(TCG_PRESETS.name, "Vanguard", sizeof(TCG_PRESETS.name) - 1);  
TCG_PRESETS.starting_life = 5;  
TCG_PRESETS.small_step = 1;​  
TCG_PRESETS.large_step = 2;  



**Digimon Card Game:**

strncpy(TCG_PRESETS.name, "Digimon", sizeof(TCG_PRESETS.name) - 1);  
TCG_PRESETS.starting_life = 5; // Security cards​  
TCG_PRESETS.small_step = 1;​  
TCG_PRESETS.large_step = 2;  



**Star Wars Unlimited:**

strncpy(TCG_PRESETS.name, "Star Wars", sizeof(TCG_PRESETS.name) - 1);  
TCG_PRESETS.starting_life = 30;​  
TCG_PRESETS.small_step = 1;​  
TCG_PRESETS.large_step = 5;  

​

### Step 3: Rebuild and Upload

1. Save the file (`src/tcg_presets.cpp`)
2. Run **PlatformIO: Upload**
3. Your custom presets will appear in the game selection menu

### Notes

- Maximum 10 presets (indices 0-9)
- First 7 slots are pre configurde with default games
- Every Slot can be changed
- Preset names are limited to 15 characters
- Changes require recompiling and re-uploading firmware except using the on-board menu

---

## 📖 Usage Guide

### Basic Controls

#### Main Screen (Single Player)

- **Tap Top Half:** Increase life by small step
- **Tap Bottom Half:** Decrease life by small step
- **Swipe Bottom to Top:** Increase by large step
- **Swipe Top to Bottom:** Decrease by large step
- **Long press Middle:** Open tools menu (dice, coin, timer)
- **Click Gear in tools menu:** Open settings menu
- **Tap Timer:** Start/pause timer
- **Long Press Timer:** Reset timer

#### Main Screen (Two Player)

- **Tap Your Side:** Adjust your life total
- **Swipe Your Side:** Large adjustment
- **Long press Middle:** Open menus
- **Tap Timer:** Start/pause timer

#### Tools Menu

- **⚙️ Settings Icon:** Settings Menu
- **1P/2P:** Switch 1 and 2 Player Mode
- **🔄 Reset Icon:** Reset game
- **Presets:** Select Presets
#### Second Page via Swipe
- **🎲 Dice Icon:** Open dice roller menu
- **🪙 Coin Icon:** Start coin flip
- **⏱️ Timer Icon:** Timer settings

#### Settings Menu

- **Start Life:** Adjust Life and Increments for this Session
- **Brightness:** Adjust display brightness (0-100%)
- **Amp:** Enable/disable auxiliary counter (tracks Commander Damage, Poison, Storm, etc.)
- **Swipe On:** Enable/disable swipe-to-dismiss for popup menus and lists
- **Timer On:** Show/hide timer on main screen
- **Timer Mode:** Stopwatch or Countdown
- **Round Time:** Set countdown duration (countdown mode only)
- **Preset Editor:** Edit game presets

### Advanced Features

#### Using the Dice Roller

1. **Open Tools Menu**: Hold in the Middle and swipe left
2. **Select Dice Icon**: Tap the W6 symbol
3. **Choose Dice Type**: Select D4, D6, D8, D10, D12, or D20
4. **View Result**: Displays final number


#### Using the Coin Flip

1. **Open Tools Menu**: Hold in the Middle and swipe left
2. **Select Coin Icon**: Tap the 🪙 coin symbol
3. **Result**: Result will be shown directly


#### Configuring Timer Modes

**Stopwatch Mode:**
1. Settings → Timer Mode → Stopwatch
2. Timer counts up from 00:00
3. Track total game duration
4. No time limit

**Countdown Mode:**
1. Settings → Timer Mode → Countdown
2. Settings → Round Time → Set duration (1-999 minutes)
3. Timer counts down to 00:00
6. Audialert when time expires in future update

#### Editing Custom Presets

**Using Preset Editor (On-Device):**
1. Settings → Edit Presets
2. Tap any Custom Preset
3. Enter preset name (max 15 characters)
4. Set starting life
5. Set small step (tap increment)
6. Set large step (Swipe increment)
7. Save preset

**Adding Presets Before Compilation** (Recommended):

See [Adding Custom Presets](#-adding-custom-presets) section for details on editing the preset file before building.



---

## 🐛 Troubleshooting

### Build Errors

**Error: `platform-espressif32` not found**

Solution: Delete .pio folder and rebuild

### Upload Issues

**Device Not Detected**
1. Install CH340 drivers: [Download](http://www.wch-ic.com/downloads/CH341SER_EXE.html)
2. Try a different USB cable (must support data transfer)
3. Hold BOOT button while connecting USB

**Permission Denied (Linux)**

sudo usermod -a -G dialout $USER
Logout and login again


### Display Issues

**Display Shows Garbage**
- Check board variant in `platformio.ini` matches your hardware
- Rebuild with `pio run -t clean && pio run`

**Colors Inverted**
- Edit `src/Display_ST77916.cpp`
- Toggle `LCD_CMD_INVON` / `LCD_CMD_INVOFF`

**Touch Not Working**
- Verify touch controller is CST816
- Check I2C address in `Touch_CST816.cpp` (default: 0x15)
- Adjust calibration (see [Touch Calibration](#-touch-calibration))

### Runtime Issues

**Device Crashes on Startup**

Check serial monitor for error messages

pio device monitor -b 115200


**Settings Not Saving**
- NVS (Non-Volatile Storage) may be corrupted
- Erase flash: `pio run -t erase`
- Re-upload firmware: `pio run -t upload`

**Battery Drains Quickly**
- Reduce display brightness in settings
- Check battery capacity (recommended: ≥ 500mAh)

---



## 👏 Credits

### Original Project
- **jontiritilli** - Original Life Puck project: [github.com/jontiritilli/life-puck](https://github.com/jontiritilli/life-puck)

### This Fork
- **Platinplayer23** - Multi-hardware support, dice roller, coin flip, enhanced timer, Arduino 3.x integration

### Libraries Used
- **LVGL** - Graphics library (v9.3.0)
- **TFT_eSPI** - Display driver
- **ArduinoNvs** - Non-volatile storage
- **pioarduino** - Arduino 3.x ESP32 platform

### Hardware
- **Waveshare** - ESP32-S3 display modules
- **Espressif** - ESP32-S3 MCU

---

## 📄 License

This project inherits the license from the original [jontiritilli/life-puck](https://github.com/jontiritilli/life-puck) repository.

See [LICENSE](LICENSE) for details.

---

## 🔗 Useful Links

### Hardware Documentation
- [ESP32-S3-Touch-LCD-1.85C Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85C)
- [ESP32-S3-Touch-LCD-1.85 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)

### Software Resources
- [PlatformIO Documentation](https://docs.platformio.org/)
- [LVGL Documentation](https://docs.lvgl.io/9.3/)
- [pioarduino Platform](https://github.com/pioarduino/platform-espressif32)

### Purchase Links
- [Waveshare Official Store](https://www.waveshare.com/)
- [AliExpress Bundle (1.85C + Case + Battery)](https://de.aliexpress.com/item/1005009857920438.html?spm=a2g0o.order_list.order_list_main.10.6a295c5fDaffaT&gatewayAdapt=glo2deu) - **Recommended**

---

## 📞 Support

- **Issues:** [GitHub Issues](https://github.com/YOUR-USERNAME/life-puck/issues)
- **Discussions:** [GitHub Discussions](https://github.com/YOUR-USERNAME/life-puck/discussions)
- **Original Project:** [jontiritilli/life-puck](https://github.com/jontiritilli/life-puck)

---

**Made with ❤️ for the tabletop gaming community**