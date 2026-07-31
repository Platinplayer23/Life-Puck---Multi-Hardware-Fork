/**
 * @file i2c_scanner.cpp
 * @brief Boot-time I2C bus diagnostics
 *
 * Scans the I2C bus/buses for responding devices and prints every address
 * found. This turns a wiring/pin mismatch (e.g. a board flashed with the
 * wrong board target) into an obvious one-line serial log entry instead of
 * a cryptic "ESP_ERR_INVALID_STATE" deep inside the I2C driver.
 */

#include "i2c_scanner.h"
#include "board_config.h"

static void I2C_ScanBus(TwoWire &bus, const char *bus_name)
{
  printf("[I2C_Scan] Scanning %s...\n", bus_name);
  int found = 0;
  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
    bus.beginTransmission(addr);
    uint8_t result = bus.endTransmission();
    if (result == 0) {
      printf("[I2C_Scan]   Device found at 0x%02X\n", addr);
      found++;
    }
  }
  if (found == 0) {
    printf("[I2C_Scan]   No devices found on %s\n", bus_name);
  }
}

void I2C_Scan(void)
{
  I2C_ScanBus(Wire, "Wire");
#if USE_SEPARATE_TOUCH_BUS
  I2C_ScanBus(Wire1, "Wire1");
#endif
}

bool I2C_ProbeDevice(uint8_t addr)
{
  Wire.beginTransmission(addr);
  uint8_t result = Wire.endTransmission();
  return (result == 0);
}
