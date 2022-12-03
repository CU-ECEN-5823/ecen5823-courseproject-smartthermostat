/*******************************************************************************
 * @file    i2c.h
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
#ifndef SRC_I2C_H_
#define SRC_I2C_H_


#include "em_common.h"


/*******************************************************************************
 * Initializes the I2C0 with proper PORT and PIN values.
 ******************************************************************************/
void I2C0_init();


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
int I2C0_write(uint16_t dev_addr, uint8_t *data, uint8_t data_len);


/*******************************************************************************
 * First writes the reg _addr and the reads the data over I2C of given length.
 * On I2C read complete, the interrupt is generated and is handled in irq.c
 *
 * @param     dev_addr  I2C peripheral device address
 * @param     reg_addr  Register address from which data to read
 * @param     *data     Data to be written
 * @param     data_len  Length of *data array
 *
 * @return    Returns non-zero value on fail and 0 on success.
 *
 ******************************************************************************/
int I2C0_read(uint16_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t data_len);


#endif /* SRC_I2C_H_ */
