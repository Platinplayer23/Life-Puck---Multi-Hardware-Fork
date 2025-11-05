# Life Puck - Multi-Hardware Fork

[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![LVGL](https://img.shields.io/badge/LVGL-9.3.0-green.svg)](https://lvgl.io/)
[![License](https://img.shields.io/github/license/jontiritilli/life-puck)](LICENSE)

**A digital life counter for Magic: The Gathering, Yu-Gi-Oh!, Pokémon TCG, and other tabletop games.**

This is a fork of [jontiritilli/life-puck](https://github.com/jontiritilli/life-puck) with extended hardware support, comprehensive power management, and enhanced user interface.

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
- [Power Management](#-power-management)
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
- **Battery:** Built-in LiPo battery support with voltage monitoring

---

## 🆕 New Features in This Fork

### Hardware & Build System
- ✅ **Dual board support**: ESP32-S3-Touch-LCD-1.85C and 1.85
- ✅ **GPIO Expander handling**: Automatic detection and configuration for TCA9554
- ✅ **Unified codebase**: Single project for both hardware variants
- ✅ **Arduino 3.x support**: Uses latest ESP-IDF 5.3 based framework via [pioarduino](https://github.com/pioarduino/platform-espressif32)
- ✅ **Modern LVGL 9.3.0**: Updated graphics library with performance improvements
- ✅ **Custom boot logo**: 1.5-second branded startup screen

### Power Management System
- ✅ **Auto-Dim**: Automatic brightness reduction after configurable inactivity
- ✅ **Display Sleep**: Turn off display after extended inactivity
- ✅ **Low Battery Dimming**: Aggressive dimming at ≤15% battery
- ✅ **Critical Battery Protection**: Automatic deep sleep at ≤2% to prevent battery damage
- ✅ **Smart USB Detection**: Prevents false shutdowns when charging
- ✅ **Touch Wake Protection**: 300ms grace period to prevent accidental input on wake

### Enhanced UI & UX
- ✅ **Organized Settings Menu**: Power, Audio, and Timer submenus
- ✅ **Swipe-to-Close**: Instant gesture control for menus with real-time toggle
- ✅ **Professional Touch Calibration**: 9-point Least Squares Fit calibration with BOOT-button confirmation
- ✅ **International Symbols**: D4-D20 dice notation, "C" for coin flip
- ✅ **Full-Width Buttons**: 280px buttons in all submenus for better usability
- ✅ **Consistent Typography**: Unified font sizes across all menus
- ✅ **Visual State Feedback**: Grayed-out disabled options, clear ON/OFF indicators

### Game Features
- ✅ **🎲 Dice Roller**: Roll D4, D6, D8, D10, D12, D20, and D100
- ✅ **🪙 Coin Flip**: Random heads/tails with clear display
- ✅ **⚙️ Game Presets**: Pre-configured for MTG, Yu-Gi-Oh!, Pokémon, and more
- ✅ **✏️ Preset Editor**: Create and save custom game configurations
- ✅ **⏱️ Dual Timer Modes**: Stopwatch and countdown with audio alerts
- ✅ **🔊 Audio System**: Configurable beeps and timer alerts (4 sound options)
- ✅ **📊 Simple Counters**: 4 configurable auxiliary counters for tracking additional game metrics

### Quality of Life
- ✅ **Automatic Life Reset**: Reset when changing presets or max life
- ✅ **Persistent Settings**: All configurations saved to NVS
- ✅ **Session Recovery**: Resume game state after restart
- ✅ **History Tracking**: Persistent life change log
- ✅ **Input Validation**: Proper capping at maximum values (9999 for life)

---

## 📱 Software Features

### Core Life Tracking
- **One and Two Player Modes**: Track life totals for solo or head-to-head games
- **Configurable Base Life**: Set starting life from 1-9999
- **Increment/Decrement Steps**: Adjust life in small (tap) or large (swipe) steps
- **Life Change History**: Track all life changes with undo/redo support
- **Automatic Reset**: Life counters reset when changing presets or max life

### Game Utilities

#### 🎲 Dice Roller
- Roll standard RPG/TCG dice (D4, D6, D8, D10, D12, D20, D100)
- Clear result display with international "D" notation
- Accessible from contextual menu (swipe left from center)

#### 🪙 Coin Flip
- 50/50 heads/tails outcomes
- Perfect for tiebreakers and random decisions
- Accessible from contextual menu

#### ⏱️ Timer System
**Stopwatch Mode:**
- Count up from 00:00 to track game duration
- Persistent across screen changes
- Tap to start/pause, long-press to reset

**Countdown Mode:**
- Set round time (1-999 minutes) in Timer Settings
- Visual progress bar
- Audio alert when time expires
- Automatic pause at 00:00

**Timer Controls:**
- Quick access from main screen
- Configurable in Timer Settings submenu
- Toggle visibility ON/OFF
- Four audio alert sounds to choose from

#### ⚙️ Game Presets
Pre-configured settings for popular games:
- **MTG Standard** - 20 life, ±1/±5 steps
- **MTG Commander (EDH)** - 40 life, ±1/±10 steps
- **Pokémon TCG** - 60 life, ±10/±30 steps
- **Yu-Gi-Oh!** - 8000 LP, ±50/±500 steps
- **Flesh and Blood** - 40 life, ±1/±5 steps
- **Lorcana** - 20 lore, ±1/±5 steps
- **One Piece** - 5 life, ±1/±2 steps
- **Custom Slots 8-10** - Fully customizable

#### 📊 Simple Counters System
**New Feature**: 4 configurable auxiliary counters for tracking additional game metrics:

**Counter Features:**
- **4 Corner Positions**: Top-Left, Top-Right, Bottom-Left, Bottom-Right
- **Individual Enable/Disable**: Each counter can be turned on/off independently
- **Touch Interaction**: 
  - **Short Tap**: Increment by 1
  - **Medium Hold**: Decrement by 1
  - **Long Hold**: Reset to 0
- **Visual Feedback**: Color changes during press (yellow for decrement, red for reset)
- **Theme Integration**: Counters adapt to selected theme colors
- **Semi-Transparent**: 80% opacity with gradient effects for better visual integration
- **Persistent Storage**: Counter states and values saved to NVS

**Configuration:**
- **Settings → Counters**: Enable/disable individual counters
- **Centralized Settings**: All counter parameters configurable in `config.h`
- **Positioning**: Adjustable X/Y coordinates for each corner
- **Visual Settings**: Size, opacity, colors, and timing thresholds
- **Reset Integration**: Counters reset when main life counter is reset

**Common Uses:**
- **MTG**: Commander damage, poison counters, storm count, energy counters
- **Pokémon**: Energy counters, damage counters, special conditions
- **Yu-Gi-Oh!**: Life Point modifiers, special counters
- **General**: Any auxiliary tracking needed during gameplay

#### ✏️ Preset Editor
- Create custom game configurations
- Set starting life (1-9999), step sizes, and timer defaults
- Name your presets (up to 15 characters)
- Save to non-volatile storage
- Quick-select from contextual menu
- Automatic life counter reset when preset applied

#### 🔊 Audio Settings
**Dedicated Audio submenu with:**
- **Audio Toggle**: Enable/disable all sounds
- **Volume Control**: 0-100% (displayed as percentage, 9 internal steps)
- **Sound Selection**: Choose from 4 distinct alert sounds
- Visual feedback with grayed-out disabled states

**Sound Options:**
1. **Sound 1**: Standard beep
2. **Sound 2**: High-pitched tone
3. **Sound 3**: Melodic chime
4. **Sound 4**: Low buzz

### Power Management Features

#### 🔋 Power Settings Submenu
Comprehensive power management in a dedicated submenu:

**Brightness Control:**
- Manual adjustment (0-100%)
- Immediate visual feedback
- Saved to NVS

**Auto-Dim:**
- Automatically reduces brightness to 25% after inactivity
- Default: 60 seconds
- Configurable timeout (0 = disabled)
- Restores full brightness on touch
- 300ms touch-blocking to prevent accidental input

**Display Sleep:**
- Turns off display completely after extended inactivity
- Default: 300 seconds (5 minutes)
- Configurable timeout (0 = disabled)
- Wakes on touch with 300ms grace period
- Prevents life counter changes during wake

**Low Battery Dimming:**
- Activates automatically at ≤15% battery
- Forces display to 5% brightness
- Prevents dimming when USB charging detected (voltage < 1V)
- Automatically restores brightness when battery > 15%
- Works independently from auto-dim and sleep

**Critical Battery Protection:**
- **Automatic deep sleep at ≤2% battery**
- **Two-stage verification system:**
  1. Battery drops to ≤2% → Start 2-second timer
  2. After 2 seconds → Re-check voltage
  3. Deep sleep only if voltage between 1V and 3.3V
- **Smart USB detection:**
  - Voltage < 1V → USB charging detected, no shutdown
  - 30-second timeout for switch toggling protection
  - 10-second boot grace period for voltage stabilization
- **Prevents battery damage from deep discharge**
- **Protects against false triggers during charging**

> **⚠️ Important:** The original Life Puck project has no automatic low-battery shutdown. Without this feature, the battery can be completely drained, causing permanent damage. This protection system is essential for battery longevity.

### User Interface

#### Settings Menu Structure
**Main Settings Menu:**
- **Start Life**: Configure starting life and step sizes
- **Power Settings**: Complete power management submenu
- **Audio Settings**: Sound configuration submenu
- **Timer Settings**: Timer mode and round time submenu
- **Edit Presets**: Customize game presets
- **Touch Calibration**: On-device touch calibration
- **Swipe to Close**: Toggle swipe gestures for menus
- **Restart**: Reboot device
- **Battery Indicator**: Real-time battery percentage and icon

**Power Settings Submenu:**
1. Brightness (opens brightness adjustment screen)
2. Auto-Dim (ON/OFF, configurable timeout)
3. Sleep (ON/OFF, configurable timeout)
4. Low Battery Dimming (ON/OFF, automatic at ≤15%)

**Audio Settings Submenu:**
1. Audio (ON/OFF toggle)
2. Volume (0-100% display)
3. Sound (4 sound options)

**Timer Settings Submenu:**
1. Timer (ON/OFF toggle)
2. Timer Mode (Stopwatch/Round)
3. Round Time (keyboard input, 1-999 minutes)

#### Visual Design
- **LVGL-based UI**: Hardware-accelerated animations
- **Responsive Touch**: Tap, swipe, and long-press gestures
- **Visual Feedback**: Animated responses to all interactions
- **Consistent Styling**: Unified colors, fonts, and button sizes
- **State Indicators**: Clear ON/OFF displays, grayed-out disabled options
- **International Symbols**: Universal dice (D) and coin (C) notation

### State Management
- **Robust NVS Persistence**: All settings auto-saved
- **Session Recovery**: Resume game state after restart
- **History Tracking**: Persistent life change log
- **Undo/Redo Support**: Step back through life changes
- **Automatic Resets**: Smart life counter resets when needed

---

## 🚀 Getting Started

### Prerequisites

- **Operating System:** Windows, macOS, or Linux
- **IDE:** Visual Studio Code (recommended)
- **Knowledge:** Basic understanding of C++ and embedded systems
- **Hardware:** One of the supported Waveshare ESP32-S3 boards
- **Battery:** 500mAh+ LiPo battery (usually included in bundles)

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

```bash
git clone https://github.com/Platinplayer23/Life-Puck---Multi-Hardware-Fork.git
cd Life-Puck---Multi-Hardware-Fork
```

**Option B: Download ZIP**
1. Click the green **"Code"** button on GitHub
2. Select **"Download ZIP"**
3. Extract the ZIP file

### Step 4: Open Project in VS Code

1. Open VS Code
2. File → Open Folder
3. Select the project folder
4. PlatformIO will automatically detect the project

### Step 5: Configure for Your Board

Switch between board environments in PlatformIO:
- **board_1_85C**: For ESP32-S3-Touch-LCD-1.85C (with TCA9554)
- **board_1_85**: For ESP32-S3-Touch-LCD-1.85 (no GPIO expander)

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

The device features an advanced **3-Round Calibration System** that provides superior touch accuracy through multiple refinement passes.

### What is the 3-Round Calibration System?

An innovative calibration technique that evolved from the original "Suthe's Method" to provide even better accuracy:

- ❌ **No** complex multi-point sampling
- ❌ **No** pixel-perfect alignment required
- ❌ **No** heavy computational overhead

Instead:
- ✅ **Fully automatic** - System knows if you're left/right/up/down
- ✅ **Zone-based detection** - Touch target, system auto-corrects
- ✅ **Real-time convergence** - Parameters adjust while you touch
- ✅ **3-round refinement** - Progressive accuracy improvement
- ✅ **13 total steps** - Complete calibration with multiple passes
- ✅ **Fallback system** - Confirmation dialog with auto-revert

📖 **[Read full documentation](docs/Suthes_Method_Touch_Calibration.md)**

### On-Device Calibration (Recommended)

#### How the 3-Round System Works

**Round 1: Full Calibration (5 steps)**
1. **Offset X** - Touch red dot in center, system auto-adjusts horizontal offset
2. **Offset Y** - Touch red dot in center, system auto-adjusts vertical offset  
3. **Rotation** - Touch red dot on vertical line, system auto-adjusts rotation
4. **Scale X** - Touch red dot on right edge, system auto-adjusts horizontal scale
5. **Scale Y** - Touch red dot on bottom edge, system auto-adjusts vertical scale

**Round 2: Refinement (5 steps)**
- **Refine Offset X** - Ultra-fine horizontal adjustment
- **Refine Offset Y** - Ultra-fine vertical adjustment
- **Refine Rotation** - Ultra-fine angle adjustment
- **Refine Scale X** - Ultra-fine horizontal scale
- **Refine Scale Y** - Ultra-fine vertical scale

**Round 3: Final Offset (2 steps)**
- **Final Offset X** - Ultimate horizontal precision
- **Final Offset Y** - Ultimate vertical precision

**Summary Step**
- **Calibration complete!** - Touch to exit and return to settings

#### Step-by-Step Guide

1. **Access Calibration:**
   - Go to **Settings → Touch Calibration**
   - Read on-screen instructions

2. **Follow the 3-Round Process:**
   - **Round 1**: Complete calibration (5 steps, each with Coarse → Fine)
   - **Round 2**: Refinement of all parameters (5 ultra-fine steps)
   - **Round 3**: Final offset adjustment (2 maximum precision steps)
   - **Just touch and hold** the red dot or line as shown
   - System adjusts parameters **automatically**
   - Release when text says "OK! Release finger"

3. **Confirmation:**
   - Calibration is **immediately applied** for testing
   - On next boot, a **confirmation dialog** appears
   - Choose **Keep** or **Revert** (10-second auto-revert if no action)
   - Once confirmed, calibration is permanent

#### 🎯 How the 3-Round System Works

The 3-Round Calibration System uses **progressive refinement** to achieve optimal accuracy:

```
Round 1: Full Calibration (Coarse → Fine)
┌─────────────────────────────────────────┐
│ Step 1: Offset X    (Coarse → Fine)    │
│ Step 2: Offset Y    (Coarse → Fine)    │
│ Step 3: Rotation    (Coarse → Fine)    │
│ Step 4: Scale X     (Coarse → Fine)    │
│ Step 5: Scale Y     (Coarse → Fine)    │
└─────────────────────────────────────────┘

Round 2: Refinement (Ultra-Fine)
┌─────────────────────────────────────────┐
│ Step 6:  Refine Offset X  (Ultra-Fine) │
│ Step 7:  Refine Offset Y  (Ultra-Fine) │
│ Step 8:  Refine Rotation  (Ultra-Fine) │
│ Step 9:  Refine Scale X   (Ultra-Fine) │
│ Step 10: Refine Scale Y   (Ultra-Fine) │
└─────────────────────────────────────────┘

Round 3: Final Offset (Maximum Precision)
┌─────────────────────────────────────────┐
│ Step 11: Final Offset X (Max Precision)│
│ Step 12: Final Offset Y (Max Precision)│
└─────────────────────────────────────────┘
```

**The 3-round calibration process:**
1. **Round 1**: Establishes baseline calibration with coarse + fine passes
2. **Round 2**: Refines all parameters with ultra-fine adjustments
3. **Round 3**: Achieves maximum precision with final offset adjustments
4. **Progressive accuracy**: Each round builds upon the previous for optimal results

This creates a **multi-pass refinement system** that guarantees superior calibration accuracy!

#### Testing Calibration

**Best method:** Use the **Preset Editor** keyboard

1. Go to **Settings → Edit Presets**
2. Select any preset
3. Try typing on the on-screen keyboard
4. Test all screen regions (center, edges, corners)
5. If touch feels off → Just re-run calibration (takes 45-65 seconds for full 3-round process!)

**Why the keyboard?**
- Small keys in all screen regions
- Immediate visual feedback
- Most challenging UI element - if keyboard works, everything works!

**Calibration Timing:**
- **Round 1**: 20-30 seconds (5 steps × 4-6 seconds each)
- **Round 2**: 15-25 seconds (5 refinement steps × 3-5 seconds each)  
- **Round 3**: 6-10 seconds (2 final offset steps × 3-5 seconds each)
- **Total**: 45-65 seconds for complete 3-round calibration

### Factory Reset to Defaults

If your device becomes unusable due to bad calibration, you can reset to factory defaults:

#### Option 1: Via Settings (if touch still works)
1. Go to **Settings → Touch Calibration**
2. The calibration will use factory defaults during the process
3. Complete the 3-round calibration process (13 steps total)

#### Option 2: Emergency Code Reset
If touch is completely broken:

1. Open `src/main.cpp` around line 125
2. **Uncomment** the emergency reset line:
   ```cpp
   // === NOTFALL: Touch Calibration Reset ===
   resetTouchCalibrationToDefaults();  // ← Remove the //
   ```
3. Flash to device
4. **Re-comment** the line and flash again

### Manual Default Adjustment (Advanced)

To change factory default values, edit **`src/hardware/display/lvgl_driver.cpp`** line 27-34:

```cpp
static const float DEFAULT_CAL_MATRIX[7] = {
    0.0f,    // [0] offset_x
    0.85f,   // [1] scale_x - Try 0.80-1.05
    0.0f,    // [2] shear_xy - Usually 0.0
    0.0f,    // [3] offset_y
    0.0f,    // [4] shear_yx - Usually 0.0
    1.0f,    // [5] scale_y - Try 0.80-1.05
    1.0f     // [6] divisor - Leave at 1.0
};
```

**Tested Defaults:**
- **ESP32-S3-Touch-LCD-1.85C:** `scale_x = 0.85f, scale_y = 0.90f`
- **ESP32-S3-Touch-LCD-1.85:** `scale_x = 1.0f, scale_y = 1.0f` (untested)

> **Recommendation:** Use the automatic 3-round calibration instead of manual adjustment. It provides optimal, device-specific calibration in 45-65 seconds with superior accuracy through progressive refinement!

---

## ⚙️ Adding Custom Presets

You can edit presets on-device via **Settings → Edit Presets**, but the keyboard can be challenging to use. For easier customization, edit presets before compilation.

### Step 1: Locate Preset File

Edit **`src/data/tcg_presets.cpp`**

### Step 2: Add Your Custom Game

**Example: Customizing Slot 8**

```cpp
// ---- SLOT 8: Custom ----
strncpy(TCG_PRESETS[7].name, "Vanguard", sizeof(TCG_PRESETS[7].name) - 1);
TCG_PRESETS[7].starting_life = 5;
TCG_PRESETS[7].small_step = 1;
TCG_PRESETS[7].large_step = 2;
```

**Field Explanation:**
- `name`: Display name (max 15 characters)
- `starting_life`: Initial life total (1-9999)
- `small_step`: Small increment (tap)
- `large_step`: Large increment (swipe)

### Common TCG Presets

**Cardfight!! Vanguard:**
```cpp
strncpy(TCG_PRESETS[7].name, "Vanguard", sizeof(TCG_PRESETS[7].name) - 1);
TCG_PRESETS[7].starting_life = 5;
TCG_PRESETS[7].small_step = 1;
TCG_PRESETS[7].large_step = 2;
```

**Digimon Card Game:**
```cpp
strncpy(TCG_PRESETS[8].name, "Digimon", sizeof(TCG_PRESETS[8].name) - 1);
TCG_PRESETS[8].starting_life = 5;  // Security cards
TCG_PRESETS[8].small_step = 1;
TCG_PRESETS[8].large_step = 2;
```

**Star Wars Unlimited:**
```cpp
strncpy(TCG_PRESETS[9].name, "Star Wars", sizeof(TCG_PRESETS[9].name) - 1);
TCG_PRESETS[9].starting_life = 30;
TCG_PRESETS[9].small_step = 1;
TCG_PRESETS[9].large_step = 5;
```

### Step 3: Rebuild and Upload

1. Save the file (`src/data/tcg_presets.cpp`)
2. Run **PlatformIO: Upload**
3. Your custom presets will appear in the game selection menu

### Notes

- Maximum 10 presets (indices 0-9)
- Slots 0-6 are pre-configured with popular games
- All slots are fully customizable
- Preset names limited to 15 characters
- Changes require firmware re-upload (unless using on-device editor)

---

## 📖 Usage Guide

### Basic Controls

#### Main Screen (Single Player)

**Life Adjustment:**
- **Tap Top Half:** Increase life by small step
- **Tap Bottom Half:** Decrease life by small step
- **Swipe Bottom to Top:** Increase by large step
- **Swipe Top to Bottom:** Decrease by large step

**Menu Access:**
- **Long press Center:** Open contextual menu
- **Swipe Right:** Previous page in contextual menu
- **Swipe Left:** Next page in contextual menu

**Timer:**
- **Tap Timer:** Start/pause
- **Long Press Timer:** Reset

**AMP Counter (if enabled):**
- **Tap AMP Counter:** Increment by 1
- **Long Press AMP Counter:** Reset to 0

#### Main Screen (Two Player)

**Life Adjustment:**
- **Tap Your Half:** Small adjustment
- **Swipe Your Half:** Large adjustment

**Menu Access:**
- **Long press Center:** Open contextual menu

**Timer:**
- **Tap Timer:** Start/pause
- **Long Press Timer:** Reset

### Menu Navigation

#### Contextual Menu (Page 1)

- **⚙️ Settings Icon:** Open settings menu
- **1P/2P Toggle:** Switch between one and two player modes
- **🔄 Reset Icon:** Reset all life counters to starting values
- **📋 Presets Icon:** Quick-select game presets

#### Contextual Menu (Page 2)

- **🎲 Dice Icon (D):** Open dice selection menu
- **🪙 Coin Icon (C):** Instant coin flip
- **⏱️ Timer Icon:** Open timer settings

#### Settings Menu

**Main Settings:**
- **Start Life:** Adjust starting life and step sizes for current session
- **Power Settings →** Power management submenu
- **Audio Settings →** Sound configuration submenu
- **Timer Settings →** Timer mode and round time submenu
- **Counters →** Simple counters configuration submenu
- **Edit Presets:** Customize game presets
- **Touch Calibration:** On-device touch calibration
- **Swipe to Close:** Enable/disable swipe gestures for menus
- **Restart:** Reboot device
- **Battery Display:** Real-time battery percentage and icon

**Power Settings Submenu:**
1. **Brightness:** Adjust display brightness (0-100%)
2. **Auto-Dim:** Auto-dim after inactivity (ON/OFF, configurable timeout)
3. **Sleep:** Display sleep after inactivity (ON/OFF, configurable timeout)
4. **Low Battery Dimming:** Automatic dimming at ≤15% battery (ON/OFF)

**Audio Settings Submenu:**
1. **Audio:** Master audio toggle (ON/OFF)
2. **Volume:** Adjust volume (0-100%)
3. **Sound:** Select alert sound (4 options)

**Timer Settings Submenu:**
1. **Timer:** Show/hide timer on main screen (ON/OFF)
2. **Timer Mode:** Stopwatch or Countdown
3. **Round Time:** Set countdown duration (1-999 minutes, Countdown mode only)

**Counters Settings Submenu:**
1. **Top-Left Counter:** Enable/disable counter in top-left corner
2. **Top-Right Counter:** Enable/disable counter in top-right corner
3. **Bottom-Left Counter:** Enable/disable counter in bottom-left corner
4. **Bottom-Right Counter:** Enable/disable counter in bottom-right corner

### Advanced Features

#### Using the Dice Roller

1. **Open Contextual Menu**: Long press center of screen
2. **Navigate to Page 2**: Swipe left
3. **Select Dice Icon**: Tap the "D" symbol
4. **Choose Dice Type**: Select D4, D6, D8, D10, D12, D20, or D100
5. **View Result**: Number displayed immediately
6. **Close**: Tap outside or swipe down (if enabled)

#### Using the Coin Flip

1. **Open Contextual Menu**: Long press center
2. **Navigate to Page 2**: Swipe left
3. **Select Coin Icon**: Tap the "C" symbol
4. **View Result**: Heads or Tails displayed immediately

#### Configuring Timer Modes

**Stopwatch Mode:**
1. Settings → Timer Settings → Timer Mode → Stopwatch
2. Timer counts up from 00:00
3. Tracks total game duration
4. No time limit

**Countdown Mode:**
1. Settings → Timer Settings → Timer Mode → Countdown
2. Settings → Timer Settings → Round Time → Enter duration (1-999 minutes)
3. Timer counts down to 00:00
4. Audio alert when time expires
5. Automatic pause at 00:00

#### Using the Simple Counters System

The Simple Counters system provides 4 configurable auxiliary counters for tracking additional game metrics:

**Enable Counters:**
1. Settings → Counters → Enable desired counters (Top-Left, Top-Right, Bottom-Left, Bottom-Right)
2. Counters appear in their respective corners

**Usage:**
- **Short Tap:** Increment by 1
- **Medium Hold (0.5-1.5s):** Decrement by 1 (visual feedback: yellow text)
- **Long Hold (>1.5s):** Reset to 0 (visual feedback: red text)
- **Theme Integration:** Counters adapt to selected theme colors
- **Semi-Transparent:** 80% opacity with gradient effects for better integration

**Common Uses:**
- **MTG**: Commander damage, poison counters, storm count, energy counters
- **Pokémon**: Energy counters, damage counters, special conditions
- **Yu-Gi-Oh!**: Life Point modifiers, special counters
- **General**: Any auxiliary tracking needed during gameplay

**Configuration:**
- All counter settings are centralized in `config.h`
- Adjustable positioning, size, opacity, colors, and timing
- Counters reset automatically when main life counter is reset


#### Editing Custom Presets

**Using Preset Editor (On-Device):**
1. Settings → Edit Presets
2. Select any preset (Custom 8, Custom 9, Custom 10 recommended)
3. Enter preset name (max 15 characters, on-screen keyboard)
4. Set starting life (1-9999)
5. Set small step (tap increment)
6. Set large step (swipe increment)
7. Save preset (automatically saved to NVS)

**Adding Presets Before Compilation** (Recommended):

See [Adding Custom Presets](#-adding-custom-presets) section for editing `tcg_presets.cpp` before building.

#### Power Management Tips

**Extend Battery Life:**
1. Enable Auto-Dim (Settings → Power Settings → Auto-Dim → ON)
2. Enable Sleep (Settings → Power Settings → Sleep → ON)
3. Reduce brightness (Settings → Power Settings → Brightness → 30-50%)
4. Disable Simple Counters if not needed (Settings → Counters)
5. Reduce volume or disable audio

**Battery Protection:**
- Low Battery Dimming activates automatically at ≤15%
- Critical Battery Protection triggers at ≤2%
- Device enters deep sleep to prevent battery damage
- Wake device by pressing power button

**Charging:**
- Connect USB-C cable
- Device can operate while charging
- Low Battery Dimming disabled during charging (voltage < 1V detected)
- Battery percentage may show 0% while charging (normal behavior)

---

## 🔋 Power Management

### Overview

The Life Puck includes comprehensive power management to extend battery life and protect the battery from damage.

### Features

#### Auto-Dim
- **Function:** Reduces display to 25% brightness after inactivity
- **Default:** 60 seconds
- **Configuration:** Power Settings → Auto-Dim
- **Benefits:** Extends battery life during pauses in gameplay

#### Display Sleep
- **Function:** Turns off display completely after extended inactivity
- **Default:** 300 seconds (5 minutes)
- **Configuration:** Power Settings → Sleep
- **Wake:** Touch screen anywhere
- **Protection:** 300ms grace period prevents accidental life changes on wake

#### Low Battery Dimming
- **Function:** Forces display to 5% brightness at ≤15% battery
- **Automatic:** Activates when battery drops to critical levels
- **Restoration:** Automatically restores brightness when battery > 15%
- **USB Detection:** Disabled when charging (voltage < 1V)
- **Configuration:** Power Settings → Low Battery Dimming

#### Critical Battery Protection

**Purpose:** Prevents permanent battery damage from deep discharge

**How It Works:**
1. Battery drops to ≤2%
2. 2-second verification timer starts
3. After 2 seconds, voltage is re-checked
4. If voltage is between 1V and 3.3V → Deep sleep activated
5. If voltage < 1V (USB charging) or > 3.3V → Protection canceled

**Smart USB Detection:**
- Voltage < 1V → USB charging detected, no shutdown
- 30-second timeout handles power switch toggling during USB charging
- 10-second boot grace period allows voltage readings to stabilize

**Why This Matters:**
- The original Life Puck has no automatic battery protection
- Without this feature, battery can drain to 0V, causing permanent damage
- This system ensures battery longevity and prevents costly replacements

**Wake from Deep Sleep:**
- Press and hold power button for 2 seconds
- Or connect USB-C cable

### Best Practices

**For Maximum Battery Life:**
1. Enable Auto-Dim and Sleep
2. Set brightness to 30-50%
3. Enable Low Battery Dimming
4. Charge battery before it reaches 15%

**For Maximum Performance:**
1. Set brightness to 70-100%
2. Disable Auto-Dim
3. Set Sleep timeout to 0 (disabled)
4. Keep Low Battery Dimming enabled (battery protection)

**Never Disable:**
- Low Battery Dimming (protects battery)
- Critical Battery Protection (built-in, cannot be disabled)

---

## 🐛 Troubleshooting

### Build Errors

**Error: `platform-espressif32` not found**

Solution: Delete `.pio` folder and rebuild

```bash
rm -rf .pio
pio run
```

**Error: LVGL version mismatch**

Solution: Clean build directory

```bash
pio run -t clean
pio run
```

### Upload Issues

**Device Not Detected**
1. Install CH340 drivers: [Download](http://www.wch-ic.com/downloads/CH341SER_EXE.html)
2. Try a different USB cable (must support data transfer)
3. Hold BOOT button while connecting USB

**Permission Denied (Linux)**

```bash
sudo usermod -a -G dialout $USER
# Logout and login again
```

### Display Issues

**Display Shows Garbage**
- Check board variant in `platformio.ini` matches your hardware
- Rebuild with `pio run -t clean && pio run`

**Colors Inverted**
- Edit `src/hardware/display/display_st77916.cpp`
- Toggle `LCD_CMD_INVON` / `LCD_CMD_INVOFF`

**Touch Not Working**
- Verify touch controller is CST816
- Use on-device calibration (Settings → Touch Calibration)
- Or adjust manual calibration (see [Touch Calibration](#-touch-calibration))

**Display Too Dim**
- Settings → Power Settings → Brightness → Increase
- Check if Low Battery Dimming is active (battery ≤15%)
- Disable Auto-Dim if enabled

### Audio Issues

**No Sound**
- Settings → Audio Settings → Audio → Toggle ON
- Settings → Audio Settings → Volume → Increase to 50%+
- Test with timer countdown

**Audio Distorted**
- Reduce volume (Settings → Audio Settings → Volume)
- Try different sound (Settings → Audio Settings → Sound)

### Power Management Issues

**Device Won't Wake from Sleep**
- Tap screen firmly
- Connect USB-C cable
- Press and hold power button for 2 seconds

**Device Shuts Down Unexpectedly**
- Battery may be critically low (≤2%)
- Charge via USB-C
- Check battery health (replace if old/damaged)

**Display Dims Constantly**
- Auto-Dim may be enabled with short timeout
- Settings → Power Settings → Auto-Dim → Adjust timeout or disable
- Or Low Battery Dimming is active (battery ≤15%)

**False Low Battery Warnings**
- Recalibrate battery by fully charging and discharging once
- Check battery connections
- Battery may need replacement

### Runtime Issues

**Device Crashes on Startup**

Check serial monitor for error messages:

```bash
pio device monitor -b 115200
```

**Settings Not Saving**
- NVS (Non-Volatile Storage) may be corrupted
- Erase flash: `pio run -t erase`
- Re-upload firmware: `pio run -t upload`

**Battery Drains Quickly**
- Reduce display brightness
- Enable Auto-Dim and Sleep
- Check battery capacity (recommended: ≥ 500mAh)
- Battery may need replacement

**Life Counters Not Resetting**
- Manually reset via Contextual Menu → Reset Icon
- Or change preset (triggers automatic reset)

**Touch Unresponsive After Wake**
- This is normal - 300ms grace period prevents accidental input
- Wait briefly and touch again
- Or adjust `TOUCH_IGNORE_DELAY` in `power_management.cpp`

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

### Development Guidelines

1. Follow existing code style
2. Test on both hardware variants (1.85C and 1.85)
3. Update documentation for new features
4. Ensure backward compatibility with saved settings

### Testing Checklist

- [ ] Compiles without errors for both board variants
- [ ] Touch calibration works accurately
- [ ] All menus navigable with swipe gestures
- [ ] Settings persist after reboot
- [ ] Power management functions correctly
- [ ] Audio alerts work properly
- [ ] Serial monitor shows no critical errors

---

## 👏 Credits

### Original Project
- **jontiritilli** - Original Life Puck project: [github.com/jontiritilli/life-puck](https://github.com/jontiritilli/life-puck)

### This Fork
- **Platinplayer23** - Multi-hardware support, power management system, enhanced UI/UX, audio system, comprehensive bug fixes

### Libraries Used
- **LVGL** - Graphics library (v9.3.0)
- **TFT_eSPI** - Display driver
- **ArduinoNvs** - Non-volatile storage
- **pioarduino** - Arduino 3.x ESP32 platform
- **ESP32-audioI2S** - Audio playback

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
- [AliExpress Bundle (1.85C + Case + Battery)](https://de.aliexpress.com/item/1005009857920438.html) - **Recommended**

---

## 📞 Support

- **Issues:** [GitHub Issues](https://github.com/Platinplayer23/Life-Puck---Multi-Hardware-Fork/issues)
- **Discussions:** [GitHub Discussions](https://github.com/Platinplayer23/Life-Puck---Multi-Hardware-Fork/discussions)
- **Original Project:** [jontiritilli/life-puck](https://github.com/jontiritilli/life-puck)

---

## 🔋 Battery Safety Notice

**⚠️ IMPORTANT: This device includes critical battery protection features not present in the original project.**

Without the Critical Battery Protection system:
- Battery can drain to 0V
- Permanent battery damage may occur
- Battery capacity and lifespan drastically reduced
- Potential safety hazards

**Always keep Low Battery Dimming enabled for your safety and battery health.**

---

**Made with ❤️ for the tabletop gaming community**

