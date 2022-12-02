/*******************************************************************************
 * @file        i2c.h
 * @brief       Has functions to send and receive data over I2C0 and interrupt
 *              based approach is used. See irq.c on how to handle when I2C0
 *              interrupt occurs.
 * @author      Ajay Kandagal, ajka9053@colorado.edu
 *
 * @due         Nov 27, 2022
 * @project     ecen5823-courseproject-server
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Fall 2022)
 * @instructor  David Sluiter
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
int I2C0_read(uint16_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t data_len);


#endif /* SRC_I2C_H_ */
