/*******************************************************************************
 * @file        i2c.c
 * @brief       I2C Communication setup and enable
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
#include "em_i2c.h"
#include "i2c.h"
#include "sl_i2cspm.h"
#include "timers.h"
#include "scheduler.h"

#define INCLUDE_LOG_DEBUG (0)
#include "log.h"

#define SI7021_DEVICE_ADDR (0x40)
#define SENSOR_ENABLE_port  gpioPortD
#define SENSOR_ENABLE_pin   15

I2C_TransferReturn_TypeDef     transferStatus;    // make this global for IRQs in A4
I2C_TransferSeq_TypeDef        transferSequence;  // this one can be local
uint8_t                        cmd_data = 0xF3;   // make this global for IRQs in A4
uint16_t                       read_data;         // make this global for IRQs in A4
uint8_t                        data[2];

// This function initializes
// I2C functionality
void i2c_init() {
  I2CSPM_Init_TypeDef i2cInitData = {
      .port            = I2C0,
      .sclPort         = gpioPortC,
      .sclPin          = 10,
      .sdaPort         = gpioPortC,
      .sdaPin          = 11,
      .portLocationScl = 14,
      .portLocationSda = 16,
      .i2cRefFreq      = 0,
      .i2cMaxFreq      = I2C_FREQ_STANDARD_MAX,
      .i2cClhr         = i2cClockHLRStandard
  };

  I2CSPM_Init(&i2cInitData);
  //uint32_t i2c_bus_frequency = I2C_BusFreqGet (I2C0);
}   //  i2c_init()

// This function reads temperature
// from Si7021
//void read_temp() {
//  GPIO_PinOutSet(SENSOR_ENABLE_port,SENSOR_ENABLE_pin);
//  //delayApprox(250000);
//  timerWaitUs_polled(80000);
//  si7021_send_cmd();
//  si7021_read_data();
//  GPIO_PinOutClear(SENSOR_ENABLE_port,SENSOR_ENABLE_pin);
//}   //  read_temp

// This function sends write
// command on I2C bus
// to communicate with slave
void si7021_send_cmd() {

  transferSequence.addr        = SI7021_DEVICE_ADDR << 1; // shift device address left
  transferSequence.flags       = I2C_FLAG_WRITE;
  transferSequence.buf[0].data = &cmd_data;               // pointer to data to write
  transferSequence.buf[0].len  = sizeof(cmd_data);

  NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2C_TransferInit (I2C0, &transferSequence);
  if (transferStatus < 0) {
      LOG_ERROR("I2C_TransferInit() Write error = %d", transferStatus);
  }
}   //  si7021_send_cmd

// This function sends read
// command on I2C bus
// to get data from slave
void si7021_read_data() {
  transferSequence.addr        = SI7021_DEVICE_ADDR << 1; // shift device address left
  transferSequence.flags       = I2C_FLAG_READ;
  transferSequence.buf[0].data = data;               // pointer to data to write
  transferSequence.buf[0].len  = sizeof(data);
//  timerWaitUs_irq(10800);

  NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2C_TransferInit (I2C0, &transferSequence);
  if (transferStatus < 0) {
      LOG_ERROR("I2C_TransferInit() Read error = %d", transferStatus);
  }
}   //  si7021_read_data

// Prints temperature read from Si7021
uint32_t print_temp()
{
  uint32_t temperature;
  read_data = ((data[0] << 8) | (data[1]));
  temperature = (((read_data*175.72)/65536)- 46.85);
  LOG_INFO("Temperature = %d",temperature);

  return temperature;

}   //    print_temp()
