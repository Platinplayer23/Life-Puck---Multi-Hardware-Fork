# Port: ESP32-S3-Knob-Touch-LCD-1.8 (Waveshare)

Target: run the existing Lifepuck firmware unchanged (feature-wise) on the Waveshare
ESP32-S3-Knob-Touch-LCD-1.8, as a third PlatformIO environment next to `board_1_85C`
and `board_1_85`.

---

## 1. Verified hardware facts

Sources: Waveshare wiki (ESP32-S3-Knob-Touch-LCD-1.8), official schematic sheets
(`1_LCD&POWER`, `2_ESP32S3-R8`, `4_OTHER`, `5_DAC`), cross-checked against
`knobby-mtg/knobby-mtg-life-counter` (`knobby/board_pins.c`) and
`KrX3D/WaveShare-Knob-Esp32S3` (ESPHome configs). All three agree.

### MCU / memory
- ESP32-S3 **R8** module → 8 MB **octal** PSRAM.
- External flash **W25Q128JVPIQ** → 16 MB, quad SPI.
- ⇒ `memory_type = qio_opi`, `flash_size = 16MB`, PSRAM `opi` — identical to the 1.85 boards.
- A **second SoC (plain ESP32)** sits on the board for BT-audio/HID. It is connected to
  the S3 only via UART (`GPIO48 = ESP32S3_RX`, `GPIO38 = ESP32S3_TX`) and via the audio
  mux. It is irrelevant for this port, but **the USB-C plug orientation selects which chip
  you flash** (both chips share the connector). If the port doesn't show up: unplug, flip
  the Type-C plug 180°, re-plug.

### Pin map (from schematic sheet 2, net-by-net)

| Function | GPIO | Notes |
|---|---|---|
| LCD QSPI SCK | 13 | `LCD_QSPI_SCL` |
| LCD QSPI CS | 14 | |
| LCD QSPI D0 | 15 | |
| LCD QSPI D1 | 16 | |
| LCD QSPI D2 | 17 | |
| LCD QSPI D3 | 18 | |
| LCD RST | 21 | **direct GPIO**, no expander |
| LCD TE | — | **not connected** (marked X on the schematic) → must be `-1` |
| LCD backlight | 47 | `LCD_BLK` → AO3400A MOSFET gate, **active HIGH**, 10 k pulldown |
| Touch I2C SDA | 11 | `TP_SDA`, 5.1 k pull-up to 3V3 |
| Touch I2C SCL | 12 | `TP_SCL`, 5.1 k pull-up to 3V3 |
| Touch INT | 9 | `TP_INT`, active low |
| Touch RST | 10 | `TP_RST`, **direct GPIO** |
| Encoder A | 8 | `EC1_A`, 10 k pull-up |
| Encoder B | 7 | `EC1_B`, 10 k pull-up |
| Battery/system ADC | 1 | `BATT_ADC`, ADC1_CH0, 10k/10k divider |
| GPIO0 | 0 | `I2S_SWITCH_IN` **and** boot strap; 10 k pull-up |
| I2S BCK | 39 | `S3_I2S_DAC_BCK` |
| I2S LRCK/WS | 40 | `S3_I2S_DAC_LRCK` |
| I2S DIN | 41 | `S3_I2S_DAC_DIN` |
| PDM mic SCK / DATA | 45 / 46 | unused by Lifepuck |
| SD (SDMMC 4-bit) CLK/CMD/D0..D3 | 4 / 3 / 5, 6, 42, 2 | unused by Lifepuck |
| UART to 2nd ESP32 RX/TX | 48 / 38 | unused by Lifepuck |
| USB D-/D+ | 19 / 20 | |

### Peripherals
- **Display:** ST77916, 360×360, QSPI — *same controller and same resolution as the
  1.85 boards*. The existing `esp_lcd_st77916.c` driver and the init-command table can be
  reused 1:1; only the pins change.
- **Touch:** CST816, I2C addr `0x15` — same driver, different pins/bus.
- **Haptics:** DRV2605L (LRA motor) at I2C addr `0x5A`, **on the same bus as the touch
  controller** (GPIO11/12). `HAPTIC_EN` hard-tied to 3V3, `IN/TRIG` tied to GND, so it is
  I2C-triggered only. Not used by Lifepuck; ignore for now.
- **Audio:** PCM5100A I2S DAC → 3.5 mm jack. The I2S lines run through a **CH445P 2:1
  analog mux (U18)** that selects between the S3 and the 2nd ESP32. Select input is
  `GPIO0`; it has a 10 k pull-up, so the **default (high) state routes the S3's I2S to the
  DAC**. Leaving GPIO0 alone therefore works; explicitly driving it HIGH after boot is the
  safe belt-and-braces move.
- **No TCA9554 GPIO expander.** ⇒ `HAS_GPIO_EXTENDER 0`, all resets via direct GPIO.
- **No power button and no power-control MOSFET.** The two SSCM110100 parts on sheet 1 are
  the two rotary encoders (EC1 → S3, EC2 → 2nd ESP32); the SSCM110100 has **no push
  switch**. There is no `PWR_KEY` and no `PWR_Control` equivalent.

### ⚠️ Battery measurement is NOT a cell measurement
Sheet 4 shows the divider as **5V → R62 10k → BATT_ADC → R63 10k → GND**, i.e. the ADC
sees half of the *5 V system rail*, not the LiPo cell. The Waveshare demo calls this
"system voltage" too. Consequences:
- scale factor is **×2.0**, not the ×3.0/0.990476 used on the 1.85 boards;
- the reading is ≈5.0 V regardless of charge state, so **percentage, low-battery dimming
  and critical-battery shutdown are meaningless on this board** and must be defaulted off;
- the charger/battery front-end lives on a daughterboard reached through connector CN1 and
  is not in the published schematic sheets, so this cannot be resolved on paper — the boot
  log must print the raw mV so it can be checked on real hardware.

---

## 2. Diagnosis of the Reddit report

Reported: `The I2C transmission fails. - I2C Read` plus
`[esp32-hal-i2c-ng.c:275] i2cWrite(): i2c_master_transmit failed: [259] ESP_ERR_INVALID_STATE`.

- `esp32-hal-i2c-ng.c` is the *new* I2C HAL (Arduino-ESP32 ≥ 3.2 / IDF 5.4
  `i2c_master`), which is what this project's pioarduino platform pulls in — so nothing
  unusual there.
- `The I2C transmission fails. - I2C Read` is Lifepuck's own string, from
  `i2c_Driver.cpp:19` and `touch_cst816.cpp:20/33`.
- `259 = 0x103 = ESP_ERR_INVALID_STATE` from `i2c_master_transmit` is what you get when the
  bus never comes up / no device ACKs on the configured pins.

**Most likely cause: a `board_1_85C` build was flashed onto the Knob board.** That build
drives I2C on **SDA 11 / SCL 10**. On the Knob board GPIO11 *is* `TP_SDA`, but **GPIO10 is
`TP_RST`** — so SCL is wired to the touch controller's reset line. No clock reaches the
CST816, nothing ACKs, every transfer fails. On top of that, `TCA9554PWR_Init()` talks to an
expander at `0x20` that does not exist on this board at all. Symptom matches exactly.

This is a pinout mismatch, not a driver bug — the port below fixes it. Two robustness
issues are worth fixing while we're in there (they turn a wrong-pins situation into a
confusing failure instead of a clear one):
1. `I2C_Read`/`I2C_Write` are declared `bool` but `return -1` on error and `0` on success —
   i.e. **inverted** truthiness. `I2C_Read_Touch` returns `true` on *both* paths.
2. `TCA9554PWR_Init()` and `Touch_Init()` are called unconditionally and their failures are
   only printed, never surfaced. A boot-time I2C scan + explicit "device present?" probe
   makes a mis-flash obvious in one line of serial output.

---

## 3. Implementation plan

**Guiding rule: no behaviour changes for `board_1_85C` / `board_1_85`.** Every existing
pin value must survive byte-for-byte; the Knob board is added as a third branch.

### Step 1 — `platformio.ini`
Add:
```ini
[env:board_knob_1_8]
extends = env:common
build_flags =
    ${env:common.build_flags}
    -DBOARD_KNOB_1_8
```
Leave `default_envs = board_1_85C` untouched. `common` already has the right
memory/flash/PSRAM settings for this board.

### Step 2 — make `include/board_config.h` the single source of truth
Today pins are scattered across `board_config.h`, `display_st77916.h`, `touch_cst816.h`,
`battery_state.h`, `power_key.h`, `simple_audio.h`. Consolidate:

Add a `BOARD_KNOB_1_8` branch and extend the *common* section so every board defines the
full set. New/renamed macros to introduce (with the existing 1.85 values kept as-is for the
existing boards):

| Macro | 1.85C | 1.85 | Knob 1.8 |
|---|---|---|---|
| `HAS_GPIO_EXTENDER` | 1 | 1 | **0** |
| `HAS_POWER_KEY` *(new)* | 1 | 1 | **0** |
| `HAS_ENCODER` *(new)* | 0 | 0 | **1** |
| `LCD_PIN_SCK / D0..D3 / CS` | 40/46/45/42/41/21 | same | **13/15/16/17/18/14** |
| `LCD_PIN_RST` | -1 | 14 | **21** |
| `LCD_PIN_TE` | 18 | 18 | **-1** |
| `LCD_PIN_BL` | 5 | 5 | **47** |
| `I2C_SDA_PIN / I2C_SCL_PIN` | 11 / 10 | 11 / 10 | **11 / 12** |
| `TOUCH_PIN_SDA / SCL / INT / RST` | 11/10/4/-1 | 1/3/4/-1 | **11/12/9/10** |
| `BAT_ADC_PIN` *(moved here)* | 8 | 8 | **1** |
| `BAT_ADC_SCALE` *(new)* | 3.0f | 3.0f | **2.0f** |
| `BAT_ADC_OFFSET` *(new)* | 0.990476f | 0.990476f | **1.0f** |
| `PWR_KEY_Input_PIN / PWR_Control_PIN` | 6 / 7 | 6 / 7 | **not defined** |
| `I2S_BCLK / I2S_LRC / I2S_DOUT` *(moved here)* | 48/38/47 | same | **39/40/41** |
| `I2S_SWITCH_PIN` *(new, Knob only)* | — | — | **0** |
| `ENCODER_PIN_A / _B` *(new, Knob only)* | — | — | **8 / 7** |
| `BOARD_NAME` | … | … | `"ESP32-S3-Knob-Touch-LCD-1.8"` |

Then change the other headers to `#include <board_config.h>` and alias to these macros
(`#define ESP_PANEL_LCD_SPI_IO_SCK LCD_PIN_SCK`, `#define LCD_Backlight_PIN LCD_PIN_BL`,
`#define CST816_INT_PIN TOUCH_PIN_INT`, …) instead of hardcoding numbers. Verify the 1.85
builds still produce identical pin values.

### Step 3 — `display_st77916.cpp`
- `ST7701_Reset()`: the `#if HAS_GPIO_EXTENDER` / `#else` split already does the right
  thing; the `#else` branch now applies and uses `LCD_PIN_RST = 21`.
- `EXAMPLE_LCD_PIN_NUM_RST` must stay `-1` (the code resets the panel manually before
  `esp_lcd_panel_reset()`), OR be set to `LCD_PIN_RST` — pick one, don't do both. Keep it
  `-1` and rely on the existing manual reset, so the code path stays identical to the
  working 1.85C build.
- `ST77916_Init()` currently does `pinMode(ESP_PANEL_LCD_SPI_IO_TE, OUTPUT)`. On the Knob
  board TE is `-1` → **guard this with `#if LCD_PIN_TE >= 0`**, otherwise it configures
  GPIO -1.
- Backlight: `ledcAttach(LCD_Backlight_PIN, …)` on GPIO47, active high — unchanged logic.
- Leave the 0x04-register probe and the two vendor init tables alone; the Knob board uses
  the same ST77916, and the probe picks the right table at runtime.

### Step 4 — `i2c_Driver.cpp` / `touch_cst816.cpp`
- `I2C_Init()`: `Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ)`. The clock rate is a
  **per-board** macro on purpose: `100000` for 1.85C/1.85 (that is Arduino's implicit
  default, i.e. what those boards were validated at — hardcoding 400 kHz for everyone would
  be a silent behaviour change) and `400000` for the Knob board, which has 5.1 k pull-ups
  and a CST816 rated for it.
- `TCA9554PWR_Init()` in `main.cpp` must be wrapped in `#if HAS_GPIO_EXTENDER`. Calling it
  on the Knob board only produces "Data Transfer Failure" spam for a chip that isn't there.
- `I2C_Read()` must check the return value of `Wire.requestFrom()`. On a NACKed read it
  returns fewer bytes than requested and `Wire.read()` then hands back `-1`/`0xFF` garbage —
  which is exactly how a wrong-pins setup turns into plausible-looking nonsense instead of a
  visible error.
- `touch_cst816.cpp`: the `#if defined(BOARD_1_85C)` / `#else` split hardcodes "C ⇒ Wire,
  everything else ⇒ Wire1". Replace with a `USE_SEPARATE_TOUCH_BUS` macro
  (1 only for `BOARD_1_85`) so the Knob board uses the main `Wire` bus (touch and DRV2605
  share GPIO11/12 with the rest of the system).
- `CST816_Touch_Reset()`: the `#else` branch applies; drives `TOUCH_PIN_RST = 10`.
- `Touch_Init()`: the RST-pin `gpio_config` is currently guarded by
  `#if HAS_GPIO_EXTENDER == 0 && TOUCH_PIN_RST != -1` — that now fires correctly for the
  Knob board. Confirm the pin is driven high before the first I2C transfer.
- Fix the inverted `bool` returns in `I2C_Read`/`I2C_Write`/`I2C_Read_Touch` (`true` =
  success everywhere) and update every call site accordingly.

### Step 5 — boot-time I2C diagnostics
`src/hardware/peripherals/i2c_scanner.cpp` is an empty file. Fill it with:
- `void I2C_Scan(void)` — scan 0x08..0x77 on `Wire` (and `Wire1` where applicable), print
  every address found;
- `bool I2C_ProbeDevice(uint8_t addr)`;
called from `setup()` right after `I2C_Init()`, behind a `#if I2C_DEBUG_SCAN` flag in
`config.h` (default **on** for the Knob env for now). Expected output on a healthy Knob
board: `0x15` (CST816) and `0x5A` (DRV2605). If the log shows nothing, it is a wiring/pin
problem, and the Reddit-style failure becomes self-diagnosing.

### Step 6 — power key / deep sleep (behaviour change, unavoidable)
The Knob board has neither `PWR_KEY_Input_PIN` (GPIO6 = SDMMC_D1) nor `PWR_Control_PIN`
(GPIO7 = **encoder B**). Driving GPIO7 as an output would fight the encoder's pull-up.
Therefore, under `#if !HAS_POWER_KEY`:
- `power_init()` / `wake_up()` / `power_loop()` → no-ops (keep the symbols so nothing else
  has to change);
- `fall_asleep()` → backlight off + `power_sleep_display()`, **no `esp_deep_sleep_start()`**
  (there is no wake source we can rely on; the CST816 INT after deep sleep is untested).
  Log a clear "[Power] deep sleep not supported on this board" line.
- Everything else in `power_management.cpp` (auto-dim, display sleep, brightness) works
  unchanged and is driven by touch activity.

### Step 7 — battery
- `battery_state.cpp`: use `BAT_ADC_SCALE` / `BAT_ADC_OFFSET` from `board_config.h`
  instead of the hardcoded `3.0` / `0.990476`.
- Print raw `analogReadMilliVolts()` alongside the computed voltage in `battery_init()` so
  the ×2.0 assumption can be verified on hardware.
- In `config.h`, for `BOARD_KNOB_1_8` only: `LOW_BATTERY_DIM_DEFAULT = 0` (battery saver
  off) so the bogus reading can never trigger dimming or the critical-battery path.
- Do **not** rip out the battery UI — leave it; it will just read ~100 %. Note it in the
  README as a known limitation.

### Step 8 — audio
- I2S pins from `board_config.h` (39/40/41).
- In `simple_audio_init()`, under `#ifdef I2S_SWITCH_PIN`: `pinMode(I2S_SWITCH_PIN, OUTPUT);
  digitalWrite(I2S_SWITCH_PIN, HIGH);` to pin the CH445P mux to the S3 source. Do this
  *after* boot (GPIO0 is a strapping pin — never touch it before `setup()`).
- Keep `driver/i2s.h` (legacy) as-is; it still compiles on Arduino-ESP32 3.x, and swapping
  to `i2s_std` is out of scope for a straight port.

### Step 9 — encoder
Out of scope for "port as-is". Only define `ENCODER_PIN_A/B` in `board_config.h` and add a
`// TODO: knob input` comment. **Do not wire it into the UI.**

### Step 10 — touch calibration
`TOUCH_CAL_DEFAULT_*` in `config.h` were tuned for the 1.85C panel. The Knob panel is a
different 360×360 module, so the offsets almost certainly differ. Move the seven defaults
behind a `#if defined(BOARD_KNOB_1_8)` block seeded with the identity transform
(`offset 0/0, scale 1.0/1.0, shear 0, divisor 1.0`) and rely on the in-app calibration
screen. Also check whether `mirror_x/mirror_y` are needed — the knobby project reports
`false/false` for this panel, i.e. no mirroring, same as the 1.85.

### Step 11 — docs
- Update `README.md`: new board in the supported list, new env name, flashing note about
  flipping the Type-C plug, and the known limitations (no power button / no deep sleep,
  battery % not meaningful, encoder unused).
- Keep this file as the porting reference.

---

## 4. Definition of done
1. `pio run -e board_knob_1_8` compiles clean (no new warnings from touched files).
2. `pio run -e board_1_85C` and `pio run -e board_1_85` still compile, and a diff of the
   effective pin macros vs. the current HEAD build is empty.
3. All code, comments and log strings in English.
4. `docs/PORT_KNOB_1_8.md` + `README.md` updated.

---

## 5. Status

All steps implemented. Verified:

- All three environments build successfully (`board_knob_1_8`, `board_1_85C`, `board_1_85`).
  The only linker warning is the pre-existing toolchain note
  `_floatdidf.o: missing .note.GNU-stack section`, unrelated to this change.
- Pin equivalence for the two existing boards proven by preprocessing `board_config.h` with
  `xtensa-esp-elf-gcc -E -dM` against the pre-change version from git: **22/22 pin macros
  byte-identical** for both `BOARD_1_85C` and `BOARD_1_85`. The macros that moved in from
  other headers (`BAT_ADC_*`, `PWR_*`, `I2S_*`) reproduce their old values exactly.
- No `-1` GPIO reaches `pinMode()`/`gpio_config()`/`ledcAttach()` on the Knob target;
  `LCD_PIN_TE` is guarded, and `PWR_*` is not even defined there.

**Untested on real hardware.** Everything here is correctness-by-construction from the
schematic. First-boot checklist on a physical Knob board:

1. Serial should show `[I2C] Bus initialized: SDA=11, SCL=12, 400000 Hz` followed by an
   `I2C_Scan()` listing **0x15** (CST816) and **0x5A** (DRV2605). If the scan is empty, stop
   — everything downstream is meaningless.
2. `[Battery] Raw ADC: … mV` — confirm the ×2.0 assumption (expect roughly 2500 mV raw).
3. Touch coordinates will be uncalibrated (identity transform); run the in-app calibration.
