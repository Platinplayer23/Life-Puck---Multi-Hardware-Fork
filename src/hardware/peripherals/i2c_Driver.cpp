/**
 * @file i2c_driver.cpp
 * @brief I2C bus driver for peripheral communication
 */

#include "I2C_Driver.h"

void I2C_Init(void) {
  // Initialize I2C bus with the pins and clock rate defined for this board.
  // I2C_FREQ_HZ is board-specific on purpose: the 1.85/1.85C boards keep
  // Arduino's 100kHz default they were validated at, the Knob 1.8 runs at
  // 400kHz (see board_config.h).
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);
  printf("[I2C] Bus initialized: SDA=%d, SCL=%d, %d Hz\n",
         I2C_SDA_PIN, I2C_SCL_PIN, (int)I2C_FREQ_HZ);
}


bool I2C_Read(uint8_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
  Wire.beginTransmission(Driver_addr);
  Wire.write(Reg_addr);
  if ( Wire.endTransmission(true)){
    printf("The I2C transmission fails. - I2C Read\r\n");
    return false;
  }
  // If the device NACKs the read phase, requestFrom() returns fewer bytes than
  // asked for and Wire.read() would silently hand back 0xFF/-1 garbage. Bail
  // out instead, so a wrong-pins situation surfaces as an error rather than as
  // plausible-looking nonsense.
  if (Wire.requestFrom(Driver_addr, Length) != Length) {
    printf("The I2C transmission fails. - I2C Read (short read from 0x%02X)\r\n", Driver_addr);
    return false;
  }
  for (int i = 0; i < Length; i++) {
    *Reg_data++ = Wire.read();
  }
  return true;
}
bool I2C_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
  Wire.beginTransmission(Driver_addr);
  Wire.write(Reg_addr);
  for (int i = 0; i < Length; i++) {
    Wire.write(*Reg_data++);
  }
  if ( Wire.endTransmission(true))
  {
    printf("The I2C transmission fails. - I2C Write\r\n");
    return false;
  }
  return true;
}