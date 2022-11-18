/*******************************************************************************
 * @file        i2c.h
 * @brief       I2C header file
 * @author      Amey More, Amey.More@colorado.edu
 * @due         Oct 24, 2022
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Fall 2022)
 * @instructor  David Sluiter
 *
 * @assignment  ecen5823-assignment9-ameyflash
 * @due         Oct 28, 2022
 *
 ******************************************************************************/

#ifndef I2C_H
#define I2C_H

// Function prototypes
void i2c_init();
void read_temp();
void si7021_send_cmd();
void si7021_read_data();
uint32_t print_temp();

#endif  /* I2C_H */
