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