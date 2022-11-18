/*******************************************************************************
 * @file        irq.c
 * @brief       Interrupt Service Routines and Interrupt Enable
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

#include "em_letimer.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "gpio.h"
#include "irq.h"
#include "scheduler.h"
#include "app.h"
#include "sl_i2cspm.h"

#define INCLUDE_LOG_DEBUG (1)
#include "log.h"

uint32_t milliseconds=0;

// This function performs the following
// 1. Clear pending interrupts
// 2. Enable required interrupt
void IRQ_Init(){

  NVIC_ClearPendingIRQ(LETIMER0_IRQn);
  NVIC_ClearPendingIRQ(I2C0_IRQn);
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);

  NVIC_EnableIRQ(LETIMER0_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

} // IRQ_Init()

// This function is an ISR.
// When a LETIMER interrupt occurs,
// depending on the interrupt flag,
// it calls event scheduler.
void LETIMER0_IRQHandler (){

  uint32_t flags = LETIMER_IntGetEnabled(LETIMER0);

  LETIMER_IntClear(LETIMER0, flags);

  if(flags & LETIMER_IF_UF) {
      milliseconds += ((LETIMER_PERIOD_MS) + (LETIMER_CounterGet(LETIMER0)/CMU_ClockFreqGet(cmuClock_LETIMER0)));
      schedulerSetEventUF();
  }
  if(flags & LETIMER_IF_COMP1) {
      schedulerSetEventCOMP1();
  }

}   //    LETIMER0_IRQHandler()

// Return milliseconds passed
// since power on
uint32_t letimerMilliseconds() {
  return milliseconds;
}   //    letimerMilliseconds()

// ISR for I2C interrupt
void I2C0_IRQHandler(void) {
  I2C_TransferReturn_TypeDef  transferStatus;

  // This shepherds the IC2 transfer along,
  // itâ€™s a state machine! see em_i2c.c
  // It accesses global variables :
  //    transferSequence
  //    cmd_data
  //    read_data
  // that we put into the data structure passed
  // to I2C_TransferInit()
  transferStatus = I2C_Transfer(I2C0);

  if (transferStatus == i2cTransferDone) {
      schedulerSetEventI2CDone();
  }
//  if (transferStatus != i2cTransferDone) {
//      LOG_ERROR("%d", transferStatus);
//  }
}   //    I2C0_IRQHandler()

// ISR for Button 0
void GPIO_EVEN_IRQHandler()  {
  uint32_t flags = GPIO_IntGetEnabled();

  GPIO_IntClear(flags);

  schedulerSetEventPB0Pressed();

}   //    GPIO_EVEN_IRQHandler()

// ISR for Button 1
void GPIO_ODD_IRQHandler()  {
  uint32_t flags = GPIO_IntGetEnabled();

  GPIO_IntClear(flags);

  schedulerSetEventPB1Pressed();

}   //    GPIO_ODD_IRQHandler()
