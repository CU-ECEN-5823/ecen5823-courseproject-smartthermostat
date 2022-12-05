/*******************************************************************************
 * @file    i2c.c
 * @brief   Has functions to send and receive data over I2C0 and interrupt based
 *          approach is used. See irq.c on how to handle when I2C0 interrupt
 *          occurs.
 *
 * @author  Ajay Kandagal, ajka9053@colorado.edu
 * @date    Nov 27, 2022
 *
 * @editor  Dec 2, 2022, Ajay Kandagal
 * @change  Rewrote I2C read to do the register address writing and reading
 *          from it in a single transaction.
 *
 ******************************************************************************/
#include <sl_i2cspm.h>

#include "i2c.h"
#include "common.h"


#define I2C0_SCL_port gpioPortC
#define I2C0_SCL_pin 10
#define I2C0_SCL_portLocation 14
#define I2C0_SDA_port gpioPortC
#define I2C0_SDA_pin 11
#define I2C0_SDA_portLocation 16


I2C_TransferSeq_TypeDef transferSequence;


/*******************************************************************************
 * Initializes the I2C0 with proper PORT and PIN values.
 ******************************************************************************/
void I2C0_init()
{
  I2CSPM_Init_TypeDef I2C_config = {
      .port = I2C0,
      .sclPort = I2C0_SCL_port,
      .sclPin = I2C0_SCL_pin,
      .sdaPort = I2C0_SDA_port,
      .sdaPin = I2C0_SDA_pin,
      .portLocationScl = I2C0_SCL_portLocation,
      .portLocationSda = I2C0_SDA_portLocation,
      .i2cRefFreq = 0,
      .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
      .i2cClhr = i2cClockHLRStandard
  };

  I2CSPM_Init(&I2C_config);
}


/*******************************************************************************
 * Writes data on I2C of given length. On I2C write complete event, the interrupt
 * is generated and is handled in irq.c
 *
 * @param     dev_addr  I2C peripheral device address
 * @param     *data     Data to be written
 * @param     data_len  Length of *data array
 *
 * @return    Returns non-zero value on fail and 0 on success.
 *
 ******************************************************************************/
int I2C0_write(uint16_t dev_addr, uint8_t *data, uint8_t data_len)
{
  I2C_TransferReturn_TypeDef transferStatus;

  I2C0_init();

  transferSequence.addr = dev_addr << 1;
  transferSequence.buf[0].data = data;
  transferSequence.buf[0].len = data_len;
  transferSequence.flags = I2C_FLAG_WRITE;
  NVIC_EnableIRQ(I2C0_IRQn);
  transferStatus = I2C_TransferInit(I2C0, &transferSequence);

  if (transferStatus < 0) {
      LOG_ERROR("I2CSPM_Transfer: I2C bus write failed\n");
      return -1;
  }

  return 0;
}


/*******************************************************************************
 * Reads data over I2C of given length. On I2C read complete, the interrupt is
 * generated and is handled in irq.c
 *
 * @param     dev_addr  I2C peripheral device address
 * @param     *data     Data to be written
 * @param     data_len  Length of *data array
 *
 * @return    Returns non-zero value on fail and 0 on success.
 *
 ******************************************************************************/
int I2C0_read(uint16_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t data_len)
{
  I2C_TransferReturn_TypeDef transferStatus;

  I2C0_init();

  transferSequence.addr = dev_addr << 1;
  transferSequence.buf[0].data = &reg_addr;
  transferSequence.buf[0].len = 1;
  transferSequence.buf[1].data = data;
  transferSequence.buf[1].len = data_len;
  transferSequence.flags = I2C_FLAG_WRITE_READ;
  NVIC_EnableIRQ(I2C0_IRQn);
  transferStatus = I2C_TransferInit (I2C0, &transferSequence);

  if (transferStatus < 0) {
      LOG_ERROR("I2CSPM_Transfer: I2C bus read failed\n");
      return -1;
  }

  return 0;
}
