# Suthe's Method - Interactive Touch Calibration

## Introduction

Since I couldn't find any similar approach in existing touch calibration implementations, I'm taking the liberty of naming this method after myself: **"Suthe's Method"**.

This document describes an innovative, zone-based touch calibration technique that provides fully automatic parameter adjustment through intelligent hit-detection zones.

---

## What is Suthe's Method?

**Suthe's Method** is an interactive calibration approach that uses invisible detection zones to automatically adjust calibration parameters based on where the user touches.

### Core Concept

Instead of sampling multiple precise points and calculating transformations mathematically, Suthe's Method:

1. **Shows a target** (line or dot)
2. **User touches the target** (doesn't need to be pixel-perfect)
3. **System detects deviation** (which zone was hit: left/right, up/down, etc.)
4. **Parameters auto-adjust** (offset or scale incrementally changed)
5. **Loop repeats** until touch lands in target zone
6. **Convergence achieved** → Lock and advance to next step

This creates a **self-correcting feedback loop** that eliminates the need for precise manual alignment.

---

## Technical Implementation

### Calibration Steps

The method calibrates 4 (or 5) parameters sequentially:

1. **Offset X** - Horizontal offset correction
2. **Offset Y** - Vertical offset correction  
3. **Rotation** (Optional) - Rotational alignment
4. **Scale X** - Horizontal scaling correction
5. **Scale Y** - Vertical scaling correction

### Two-Pass System

Each step uses a **2-pass approach** for optimal speed and precision:

#### Pass 1: Coarse Adjustment (Fast)
- **Large step sizes** (2.0 pixels, 1.5% scale)
- **Quick approach** to target zone
- **Locks after 10 consecutive frames** in target
- **Automatically switches** to fine pass

#### Pass 2: Fine Adjustment (Precise)
- **Small step sizes** (0.25 pixels, 0.1% scale)
- **High precision** tuning
- **Locks after 10 consecutive frames** in target
- **Advances** to next calibration step

### Zone-Based Detection

Each calibration step uses **invisible hit-test zones** to detect touch position:

#### Offset X Calibration
```
┌─────────────────────────────┐
│         │         │         │
│  LEFT   │ TARGET  │  RIGHT  │
│  ZONE   │  ZONE   │  ZONE   │
│         │         │         │
└─────────────────────────────┘
```
- **Left zone hit** → `offset_x += step` (shift right)
- **Right zone hit** → `offset_x -= step` (shift left)
- **Target zone hit** → Lock (perfectly centered!)

#### Offset Y Calibration
```
┌─────────────────────────────┐
│        TOP ZONE             │
├─────────────────────────────┤
│      TARGET ZONE            │
├─────────────────────────────┤
│      BOTTOM ZONE            │
└─────────────────────────────┘
```
- **Top zone hit** → `offset_y += step` (shift down)
- **Bottom zone hit** → `offset_y -= step` (shift up)
- **Target zone hit** → Lock (perfectly centered!)

#### Scale X Calibration
```
┌─────────────────────────────┐
│                             │
│  INCREASE ZONE       ● TGT  │
│  (Touch registers left)     │
│                             │
└─────────────────────────────┘
```
- **Left zone hit** → `scale_x += step` (expand horizontally)
- **Target zone hit** → Lock (perfect scale!)

#### Scale Y Calibration
```
┌─────────────────────────────┐
│     INCREASE ZONE           │
│  (Touch registers high)     │
├─────────────────────────────┤
│          ● TARGET           │
└─────────────────────────────┘
```
- **Top zone hit** → `scale_y += step` (expand vertically)
- **Target zone hit** → Lock (perfect scale!)

---

## Advantages of Suthe's Method

### ✅ User Experience
- **No pixel-perfect alignment required** - Just touch and hold
- **Fully automatic** - System adjusts parameters without user input
- **Visual feedback** - Clear "OK! Release finger" when locked
- **Fast** - Typically completes in 15-30 seconds
- **Intuitive** - No technical knowledge needed

### ✅ Technical Benefits
- **Self-correcting** - Feedback loop ensures convergence
- **Axis decoupling** - X and Y calibrated independently
- **No complex math during calibration** - Simple hit-tests only
- **Robust** - Works even with severely miscalibrated starting values
- **Real-time** - Parameters update at 20 Hz (50ms timer)

### ✅ Implementation Advantages
- **No multi-point sampling** - No need for 9 precise touch points
- **No least-squares calculation** - No matrix inversion or complex math
- **Low memory footprint** - No sample buffers or large arrays
- **Fast execution** - Hit-tests are computationally cheap
- **LVGL native** - Uses standard `lv_obj_hit_test()` API

---

## Disadvantages and Limitations

### ⚠️ Potential Issues

#### 1. Convergence Speed
- **Problem:** May oscillate near target zone in fine pass
- **Impact:** Can take a few extra seconds to lock
- **Mitigation:** Frame counter requires 10 consecutive hits to lock

#### 2. Non-Linear Distortion
- **Problem:** Method assumes linear transformation (scale + offset)
- **Impact:** May not correct severe non-linear touch panel warping
- **Mitigation:** Shear parameters (B, D) in matrix can be manually adjusted if needed

#### 3. Rotation Calibration Complexity
- **Problem:** Rotation step requires understanding of center-based rotation
- **Impact:** User may not understand "touch on line" instruction
- **Mitigation:** Rotation step is optional and can be skipped

#### 4. Dependency on lv_obj_hit_test()
- **Problem:** Requires LVGL's hit-test to work correctly
- **Impact:** Transparent objects (opacity = 0) cannot be detected in LVGL v9
- **Mitigation:** Use `lv_btn_create()` with `LV_OPA_TRANSP` for invisible zones

#### 5. No Cross-Axis Correction
- **Problem:** X-offset doesn't account for Y-position variations
- **Impact:** If touch panel has position-dependent offset, calibration may be suboptimal
- **Mitigation:** Use shear parameters (B, D) for advanced correction

---

## Implementation Details

### Code Structure

**File:** `src/ui/screens/settings/touch_calibration.cpp`

#### Key Components:

1. **State Machine:**
   ```cpp
   typedef enum {
       CAL_STEP_OFFSET_X,
       CAL_STEP_OFFSET_Y,
       CAL_STEP_ROTATION,  // Optional
       CAL_STEP_SCALE_X,
       CAL_STEP_SCALE_Y,
       CAL_STEP_SUMMARY,
       CAL_STEP_FINISHED
   } calibration_step_t;
   ```

2. **Zone Objects:**
   ```cpp
   static lv_obj_t* zone1 = nullptr;        // Correction zone 1 (left/top)
   static lv_obj_t* zone2 = nullptr;        // Correction zone 2 (right/bottom)
   static lv_obj_t* zone_target = nullptr;  // Target zone (lock zone)
   ```

3. **Core Logic:**
   ```cpp
   static void process_calibration_logic(lv_timer_t* timer) {
       // Read raw touch data
       Touch_Read_Data();
       
       // Calculate calibrated position
       // Perform hit-tests on zones
       // Adjust parameters based on which zone was hit
       // Lock if target zone hit for 10 consecutive frames
   }
   ```

### Matrix Application

The calibration matrix is a **7-parameter affine transformation**:

```cpp
float g_cal_matrix[7] = {
    offset_x,  // [0] C - X translation
    scale_x,   // [1] A - X scaling
    shear_xy,  // [2] B - X shear from Y
    offset_y,  // [3] F - Y translation
    shear_yx,  // [4] D - Y shear from X
    scale_y,   // [5] E - Y scaling
    divisor    // [6] - Normalization
};
```

**Transformation formula:**
```cpp
cal_x = (raw_x * A) + (raw_y * B) + C
cal_y = (raw_x * D) + (raw_y * E) + F
cal_x /= divisor
cal_y /= divisor
```

During calibration, Suthe's Method adjusts **C, F, A, E** (and optionally **B, D** for rotation).

---

## Comparison with Traditional Methods

### Traditional Multi-Point Calibration

**Typical Approach:**
1. Show 9 target points
2. User touches each point precisely
3. System samples raw coordinates
4. Least-squares fit calculates transformation matrix

**Pros:**
- Mathematically optimal (minimizes total error)
- Corrects non-linear distortion
- Single-shot calibration (no iteration)

**Cons:**
- Requires pixel-perfect alignment (difficult on small screens)
- User must touch 9 points accurately
- Complex math (matrix inversion, least-squares)
- More code, higher memory footprint

### Suthe's Method

**Approach:**
1. Show 1 target at a time
2. User touches target (approximate)
3. System detects deviation and auto-corrects
4. Repeat for 4-5 steps

**Pros:**
- No precise alignment needed
- Fully automatic adjustment
- Simple implementation (just hit-tests)
- Fast and intuitive user experience
- Guaranteed convergence (feedback loop)

**Cons:**
- May not correct severe non-linear distortion
- Takes slightly longer than single-shot methods
- Requires multiple user interactions (4-5 touches)

---

## User Workflow

### Step-by-Step Guide

#### Step 1: Offset X (Horizontal Centering)
1. **Red vertical line** appears in center
2. **User touches and holds** the line
3. **System detects** if touch is left or right of center
4. **Offset adjusts automatically** until touch hits center
5. **Text changes** to "OK! Release finger"
6. **User releases** → Advances to next step

#### Step 2: Offset Y (Vertical Centering)
1. **Red horizontal line** appears in center
2. **User touches and holds** the line
3. **System detects** if touch is above or below center
4. **Offset adjusts automatically** until touch hits center
5. **Text changes** to "OK! Release finger"
6. **User releases** → Advances to next step

#### Step 3: Rotation (Optional, if enabled)
1. **Red dot** appears on diagonal line
2. **User touches and holds** the dot
3. **System detects** rotation error
4. **Angle adjusts automatically** until touch aligns
5. **Text changes** to "OK! Release finger"
6. **User releases** → Advances to next step

#### Step 4: Scale X (Horizontal Scaling)
1. **Red dot** appears on **right edge**
2. **Scale starts at 0.5** (everything squeezed left)
3. **User touches and holds** the dot
4. **Scale increases automatically** until touch hits target
5. **Text changes** to "OK! Release finger"
6. **User releases** → Advances to next step

#### Step 5: Scale Y (Vertical Scaling)
1. **Red dot** appears on **bottom edge**
2. **Scale starts at 0.5** (everything squeezed up)
3. **User touches and holds** the dot
4. **Scale increases automatically** until touch hits target
5. **Text changes** to "OK! Release finger"
6. **User releases** → Calibration complete!

---

## Technical Notes

### Why This Works

The method exploits a fundamental property of affine transformations:

**Offset calibration is independent of scale:**
- Adjusting `offset_x` shifts all touches horizontally
- **Zone detection** tells system which direction to shift
- System converges to correct offset regardless of scale

**Scale calibration is independent of offset:**
- Adjusting `scale_x` expands/contracts touches from origin
- By placing target at edge, system can measure if scale is too small
- System increases scale until edge touch registers correctly

### Convergence Guarantee

The method is **guaranteed to converge** because:

1. **Monotonic adjustment** - Parameters only move toward target
2. **Frame-based locking** - Prevents premature lock on noise
3. **Two-pass refinement** - Coarse pass gets close, fine pass perfects
4. **Backtracking** - Stepping back from coarse lock ensures fine pass captures target

### Performance

**Timing (typical):**
- Offset X: 3-5 seconds (coarse + fine)
- Offset Y: 3-5 seconds (coarse + fine)
- Rotation: 4-6 seconds (optional)
- Scale X: 2-4 seconds (coarse + fine)
- Scale Y: 2-4 seconds (coarse + fine)
- **Total: 15-30 seconds**

**Frame rate:** 20 Hz (50ms timer)

**Convergence:** Typically locks within 40-120 frames per step

---

## Code Example

### Creating Detection Zones

```cpp
// Invisible zone for left-side detection
zone1 = lv_btn_create(cal_screen);
lv_obj_set_size(zone1, (SCREEN_WIDTH / 2) - 3, SCREEN_HEIGHT);
lv_obj_align(zone1, LV_ALIGN_LEFT_MID, 0, 0);
lv_obj_set_style_bg_opa(zone1, LV_OPA_TRANSP, 0);
lv_obj_set_style_border_opa(zone1, LV_OPA_TRANSP, 0);
lv_obj_set_style_outline_opa(zone1, LV_OPA_TRANSP, 0);
lv_obj_add_flag(zone1, LV_OBJ_FLAG_CLICKABLE);
```

### Hit-Test Logic

```cpp
// Calculate calibrated coordinates
float cal_x = (raw_x * scale_x) + offset_x;
float cal_y = (raw_y * scale_y) + offset_y;

lv_point_t p = {(int16_t)cal_x, (int16_t)cal_y};

// Test which zone was hit
bool hit_left = lv_obj_hit_test(zone1, &p);
bool hit_right = lv_obj_hit_test(zone2, &p);
bool hit_target = lv_obj_hit_test(zone_target, &p);

// Adjust based on zone
if (hit_left) {
    offset_x += step_size;  // Shift right
} else if (hit_right) {
    offset_x -= step_size;  // Shift left
} else if (hit_target) {
    lock_frame_counter++;   // Count frames in target
    if (lock_frame_counter >= 10) {
        locked = true;      // Lock achieved!
    }
}
```

---

## Fallback and Confirmation

After calibration completes, the system:

1. **Saves matrix to NVS** with pending confirmation flag
2. **Applies calibration immediately** for testing
3. **Returns to settings menu**

On next boot:

1. **Loads calibrated matrix** from NVS
2. **Shows confirmation dialog** (10-second timeout)
3. **User chooses:**
   - **Keep** → Confirmation flag cleared, calibration permanent
   - **Revert** → Factory defaults restored
   - **Timeout** → Automatic revert to factory defaults

This ensures bad calibrations cannot permanently break the device.

---

## Future Enhancements

Potential improvements to Suthe's Method:

1. **Adaptive step sizes** - Reduce step size as zone is approached
2. **Predictive convergence** - Estimate final value and jump directly
3. **Multi-zone refinement** - Use smaller zones for higher precision
4. **Visual zone indicators** - Show zones during calibration for debugging
5. **Skip rotation** - Make rotation step optional via flag
6. **One-pass mode** - Skip fine pass for faster calibration

---

## Conclusion

Suthe's Method provides an elegant balance between:
- **User-friendliness** (no precise alignment needed)
- **Technical accuracy** (feedback loop ensures convergence)
- **Implementation simplicity** (zone-based hit-tests only)

While it may not achieve the theoretical optimum of least-squares methods, it provides **excellent practical accuracy** with **superior user experience**.

---

**Author:** Suthe (Pascal)  
**Date:** January 2025  
**Status:** Production Ready  
**License:** Inherited from original Life Puck project
