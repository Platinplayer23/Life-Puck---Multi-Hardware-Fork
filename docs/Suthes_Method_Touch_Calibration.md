# Touch Calibration System - Centre-Anchored Calibration

## Introduction

This document describes the **centre-anchored calibration system**, the touch-calibration technique implemented in `src/ui/screens/settings/touch_calibration.cpp`.

Since I couldn't find any similar approach in existing implementations, I originally called this "Suthe's Method": an interactive calibration technique where the user holds a finger on an on-screen target and the system adjusts the calibration parameters automatically, without requiring pixel-perfect touches.

The version described here is a redesign of that idea. The interactive, no-precision-required user experience is unchanged, but the underlying model and the step sequence are different, for reasons explained below. If you are looking for the earlier zone/hit-test-based implementation (`lv_obj_hit_test()`, `zone1`/`zone2`/`zone_target`), it has been removed; this document describes only the current code.

---

## Why the previous version was reworked

The runtime transform (applied in `Lvgl_Touchpad_Read()`, `src/hardware/display/LVGL_Driver.cpp`) is, and remains, a 7-parameter affine transformation stored in `g_cal_matrix`:

```cpp
cal_x = m[1]*raw_x + m[2]*raw_y + m[0]   // A*x + B*y + C
cal_y = m[4]*raw_x + m[5]*raw_y + m[3]   // D*x + E*y + F
both  /= m[6]                             // divisor, always 1.0
```

Three problems with the previous calibration procedure motivated this rework:

### 1. Scaling was anchored at the origin

Because `cal_x = sx*raw_x + C`, changing `sx` moves the screen centre by `Δsx * raw_x_centre`. With `raw_x_centre` around 170-200 (see the worked example below), a tiny scale change shifts the centre by roughly two orders of magnitude more than the change itself. The old offset step (measured at the centre) and the old scale step (measured at the edge) therefore fought each other: fixing the centre would throw off the edges, and fixing the edges would throw off the centre. The old "Round 2" and "Round 3" step groups existed purely to iterate that coupling away, at the cost of a much longer procedure.

### 2. A non-linear fudge factor lived outside the model

`Lvgl_Touchpad_Read()` used to apply, after the matrix, an extra shift of up to 20px whenever `final_x < 120` or `final_y < 120`. This correction was never applied during calibration itself (the calibration screen computed the plain affine, and `Lvgl_Touchpad_Read()` returns early while `getCurrentMenu() == MENU_TOUCH_CALIBRATION`). So the system was calibrated against one function and used against a different one. The correction's location - exactly the top-left corner - is consistent with it being a patch for problem 1: origin-anchored scaling puts its largest residual error there.

### 3. Rotation was not identifiable from the available signal

The old rotation step derived `current_theta` from the x-error at a single touch point. But an x-error at one point can come from an offset error, a scale error, or a rotation error - with only one point, those three cannot be told apart. Worse, the sensitivity of `cal_x` to `theta` scales with `raw_y`, while the old code used a fixed gain, so a touch near the top of the screen would drive theta hard to "fix" what was actually a plain offset error. Since the touch panel is bonded to the display assembly, physical rotation between them is effectively zero, so this step was mostly correcting noise it had itself introduced. It has been removed.

---

## The centre-anchored model

The new model keeps scale and offset as two logically separate measurements by anchoring the scale transformation at the screen centre instead of at the raw origin:

```cpp
cal_x = CENTER_X + sx * (raw_x - rx0)
cal_y = CENTER_Y + sy * (raw_y - ry0)
```

- `CENTER_X = SCREEN_WIDTH / 2`, `CENTER_Y = SCREEN_HEIGHT / 2` - both 180 on this 360x360 display (`data/constants.h`).
- `rx0`, `ry0` are the raw touch-panel coordinates measured while the user holds the centre of the screen.
- `sx`, `sy` are the horizontal/vertical scale factors, tuned afterwards against edge targets.

Expanding the multiplication shows this is still a plain affine transform:

```cpp
cal_x = sx*raw_x + (CENTER_X - sx*rx0)
cal_y = sy*raw_y + (CENTER_Y - sy*ry0)
```

so it fits the **same** 7-parameter matrix used by `Lvgl_Touchpad_Read()`, with:

```cpp
m[0] = CENTER_X - sx*rx0     // C
m[1] = sx                    // A
m[2] = 0                     // B (shear/rotation term, unused)
m[3] = CENTER_Y - sy*ry0     // F
m[4] = 0                     // D (shear/rotation term, unused)
m[5] = sy                    // E
m[6] = 1.0f                  // divisor
```

Nothing downstream changes: `Lvgl_Touchpad_Read()` still applies a generic 7-parameter affine plus clamping, unaware that the parameters were derived this way. `m[2]`/`m[4]` are kept in the matrix (fixed at 0) purely so the stored format and NVS keys stay compatible with older saved calibrations.

The calibration code has a single helper, `rebuild_matrix()`, that recomputes `temp_cal_matrix[0..6]` from `(sx, sy, rx0, ry0)`. It is the only function that writes the matrix, and it is called every time any of those four values changes.

### Why this decouples offset from scale

With scaling anchored at `(rx0, ry0)` instead of at `(0, 0)`, changing `sx` or `sy` no longer moves the measured centre point: plugging `raw_x = rx0` into the formula gives `cal_x = CENTER_X` regardless of `sx`. So the centre measurement (step 1) can be trusted once and never has to be revisited after the scale steps run. This is what removes the need for the old multi-round iteration - the parameters genuinely stop interacting once `rx0`/`ry0` are fixed.

### Worked example: interpreting the shipped defaults

The default matrix in `src/config.h` (`TOUCH_CAL_DEFAULT_*`) predates this rework and is stored in the old origin-anchored form: `offset_x = 23.5`, `scale_x = 0.90`, `offset_y = -7.0`, `scale_y = 0.95`. These numbers were derived with the old calibration procedure and validated across roughly ten physical boards - they are not placeholders, and they have not been changed by this rework.

Even so, any origin-anchored matrix can be re-read as an equivalent centre-anchored one by solving `CENTER - C = sx * rx0` for `rx0`:

```
rx0 = (CENTER_X - offset_x) / scale_x = (180 - 23.5) / 0.90  ≈ 174
ry0 = (CENTER_Y - offset_y) / scale_y = (180 - (-7.0)) / 0.95 ≈ 197
```

In other words, the shipped defaults imply this particular touch panel reports roughly `(174, 197)` for a touch at the physical screen centre. This is exactly the derivation `renderTouchCalibrationScreen()` uses to seed `current_rx0`/`current_ry0` from whatever matrix is currently stored, before `CAL_STEP_CENTER` immediately re-measures it directly. It is also a convenient way to sanity-check any stored matrix by hand.

### An open question: `TOUCH_EDGE_CORRECTION` and the shipped defaults

The shipped defaults were validated on real hardware as `affine(defaults) + edge correction` - the old procedure that produced them did not model the top-left residual described above, and the edge-correction block was compensating for it at runtime. The centre-anchored model in this document is intended to remove the need for that compensation, but only for a matrix produced by the *new* calibration procedure.

This creates a real interaction that has not been tested on a device as of this writing:

- **Freshly calibrated with the new centre-anchored procedure:** the top-left residual should not arise in the first place, so `TOUCH_EDGE_CORRECTION` should stay at its default of `0`. Leaving it on would add a correction on top of a matrix that does not need one, over-correcting the top-left.
- **Running on the shipped `TOUCH_CAL_DEFAULT_*` values without recalibrating:** the old residual is still baked into those numbers (since they came from the old procedure), so `TOUCH_EDGE_CORRECTION = 1` may still be doing useful work there.

Nobody has measured this on a device yet - anyone testing on hardware that is still running the stock defaults should try both settings and compare, rather than assuming `0` is strictly better. Once a fresh calibration has been run and saved, `0` is the expected setting going forward.

---

## The calibration procedure

The interactive screen (`renderTouchCalibrationScreen()` / `process_calibration_logic()`) now runs five steps in a fixed order:

### 1. `CAL_STEP_CENTER`

A red dot is shown at the screen centre. While the user holds it:

1. The first `CALIBRATION_SETTLE_FRAMES` touch frames are discarded (finger settling).
2. The next `CALIBRATION_SAMPLE_FRAMES` frames are averaged.
3. `rx0`/`ry0` are set from that mean and `rebuild_matrix()` is called.
4. The label switches to "OK! Release finger"; releasing advances to the next step.

This is a direct measurement, not a feedback loop - there is nothing to converge, so there is no coarse/fine pass here.

### 2. `CAL_STEP_SCALE_X`

A red dot is shown at `x = SCREEN_WIDTH - 20` (screen-centre height). This step keeps the proportional-feedback loop from the previous implementation:

```cpp
scale_correction = 1 - (error / target) * speed
current_scale_x *= scale_correction;
rebuild_matrix();
```

run over a Coarse pass (`TOLERANCE_COARSE = 5px`, `CORRECTION_SPEED_COARSE = 0.3`) followed by a Fine pass (`TOLERANCE_FINE = 2px`, `CORRECTION_SPEED_FINE = 0.1`), each locking after `CALIBRATION_LOCK_FRAMES` consecutive in-tolerance frames. The two-pass structure is retained because it is cheap and still helps convergence speed; what has been removed is the old Round 2/Round 3 repetition of the *entire* offset+scale sequence, which is no longer needed now that offset and scale don't interact.

The error driving the loop is computed from a short moving average of raw touch samples (`CALIBRATION_SAMPLE_FRAMES` wide), not from a single 50ms reading. `TOLERANCE_FINE` (2px) is finer than a finger reproducibly hits in one sample, so averaging - not just the consecutive-frame lock counter - is what keeps the loop from chasing sensor noise.

### 3. `CAL_STEP_SCALE_Y`

Identical to `CAL_STEP_SCALE_X`, mirrored: dot at `y = SCREEN_HEIGHT - 20`, tuning `current_scale_y`.

### 4. `CAL_STEP_VERIFY`

The red dot returns to the centre. This step makes no parameter changes - it is purely informational. It uses the same settle-then-average sampling as `CAL_STEP_CENTER`, transforms the averaged raw sample through the now-final matrix, and displays the residual distance from the centre, e.g. `"Error: 1.4 px"`. This gives the user a concrete number to judge the calibration by before it is saved. Touching and releasing advances to the summary.

### 5. `CAL_STEP_SUMMARY`

Unchanged from before: shows the final centre raw coordinates and scale factors, and saves on touch. See "Save and confirmation flow" below.

### Step count and timing

| | Old (3-round) | New (centre-anchored) |
|---|---|---|
| Steps | 13 (5 + 5 + 2 + 1 summary, each of the first 5 with a Coarse/Fine sub-pass = up to 13 hold/release cycles) | 5 (1 + 2 with Coarse/Fine + 1 + 1 summary = 4 hold/release cycles) |
| Parameters tuned | offset_x, offset_y, theta, scale_x, scale_y (3 rounds each) | rx0, ry0 (1 direct measurement), scale_x, scale_y (1 feedback loop each) |
| Typical duration | 45-65 seconds | Well under half that - one direct measurement (a few hundred ms of holding) plus two feedback loops that no longer have to re-converge against each other in later rounds, plus one purely informational touch |

The old document's 45-65 second estimate was itself a consequence of the coupling problem: most of that time was spent re-running Round 2 and Round 3 to claw back accuracy that the previous round's scale adjustment had disturbed. Removing that coupling removes most of the procedure, not just the rotation step.

---

## Why rotation was removed entirely

As explained above, the old rotation step could not distinguish rotation from offset error using a single touch point, and its fixed angular gain made it oversensitive to touches away from the vertical centre. Combined with the fact that the touch panel is physically bonded to the display (so true rotation is approximately zero), the step was doing more harm than good. `current_theta`, `update_rotation_matrix()`, and the associated UI strings have been deleted. `m[2]` and `m[4]` remain in the matrix, always written as `0` by `rebuild_matrix()`, so a shear/rotation term could be reintroduced later without another NVS format change.

---

## Why the edge correction is now opt-out

The `final_x < 120` / `final_y < 120` correction block in `Lvgl_Touchpad_Read()` (`src/hardware/display/LVGL_Driver.cpp`) is now compiled in only when `TOUCH_EDGE_CORRECTION` is defined to `1` in `src/config.h` (default `0`). It was almost certainly a workaround for problem 1 above - the centre-anchored model should not need it once a fresh calibration has been run - but this cannot be verified without a physical device, and the shipped `TOUCH_CAL_DEFAULT_*` values were validated *with* the correction active (see "An open question" above). Keeping it behind a single flag makes it a one-line change to A/B test on real hardware and to fall back to the old behaviour if the new model turns out not to fully eliminate the residual.

---

## Save and confirmation flow (unchanged)

This part of the system was not touched by this rework and remains the safety net it always was:

1. On completing `CAL_STEP_SUMMARY`, `save_and_exit_calibration()` writes the 7 matrix values to NVS under `touch_cal_0` .. `touch_cal_6`, sets `touch_valid = 1` and `touch_pending = 1`, and applies the matrix immediately via `updateTouchCalibrationMatrix()`.
2. On the next boot, `needsTouchCalibrationConfirmation()` (checking `touch_pending`) triggers a confirmation screen with a 10-second countdown (`CALIBRATION_TIMEOUT_MS`).
3. The user can **Keep** (clears `touch_pending`, calibration becomes permanent) or **Revert** (restores `TOUCH_CAL_DEFAULT_*` via `resetTouchCalibrationToDefaults()`). Letting the countdown expire reverts automatically.

This ensures a bad calibration - however it was produced - can never permanently strand the user with an unusable touchscreen.

---

## Manual calibration (alternative)

For advanced users, or when the automatic calibration doesn't provide sufficient accuracy:

1. **Access Manual Mode**: Edit `src/main.cpp` around line 125, uncomment `resetTouchCalibrationToDefaults();`, flash, then re-comment and flash again to force a reset.
2. **Adjust the factory defaults**: Edit `TOUCH_CAL_DEFAULT_*` in `src/config.h` directly. These are consumed by `DEFAULT_CAL_MATRIX` in `src/hardware/display/LVGL_Driver.cpp`.

> As with the previous version of this document: try the automatic calibration first. Manual adjustment is for debugging or hardware that the automatic procedure genuinely cannot handle.

---

## Conclusion

The centre-anchored model removes the parameter coupling that made the previous calibration procedure need three repeated rounds, by measuring the screen centre directly and anchoring subsequent scale adjustments there instead of at the touch panel's raw origin. Rotation calibration is gone because it was never separable from offset error with the available signal, and the panel does not physically rotate relative to the display anyway. The empirical top-left edge correction is preserved but now opt-in, pending a real A/B test against the new model on hardware.

---

**Author:** Suthe (Pascal)
**Status:** Awaiting hardware validation
**License:** Inherited from original Life Puck project
