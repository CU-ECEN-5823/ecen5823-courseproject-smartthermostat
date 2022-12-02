/*******************************************************************************
 * @file        irq.c
 * @brief       Interrupt Service Routines and Interrupt Enable
 * @author      Amey More, Amey.More@colorado.edu
 *
 * @due         Nov 24, 2022
 * @project     ecen5823-courseproject-server
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Fall 2022)
 * @instructor  David Sluiter
 *
 * @editor      Nov 28, 2022, Ajay Kandagal, ajka9053@colorado.edu
 * @change      Added IRQ Handlers for LETIMER0 and I2C0
 *
 ******************************************************************************/
#include "em_common.h"
#include "em_letimer.h"
#include "sl_i2cspm.h"

#include "irq.h"
#include "gpio.h"
#include "scheduler.h"

#define INCLUDE_LOG_DEBUG (0)
#include "log.h"


/*******************************************************************************
 * Clears the pending interrupts and enables then enables NVIC flag for
 * GPIO_EVEN_IRQn, GPIO_ODD_IRQn and LETIMER0_IRQn.
 ******************************************************************************/
void IRQ_Init(){

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_ClearPendingIRQ(LETIMER0_IRQn);

  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(LETIMER0_IRQn);

} // IRQ_Init()


/*******************************************************************************
 * Calls scheduler to set an event when LETIMER0's COMP1 or COMP1
 * overflows.
 ******************************************************************************/
void LETIMER0_IRQHandler(void)
{
  // Get value of pending interrupts in LETIMER0
  uint32_t reason = LETIMER_IntGetEnabled(LETIMER0);

  // Clear pending interrupts in LETIMER0
  LETIMER_IntClear(LETIMER0, reason);

  if (reason & LETIMER_IEN_UF) {
      // Enters critical section and calls scheduler to set temperature read
      CORE_DECLARE_IRQ_STATE;
      CORE_ENTER_CRITICAL();
      schedulerSetTimerComp0Event();
      CORE_EXIT_CRITICAL();
  }

  if (reason & LETIMER_IEN_COMP1) {
      // Enters critical section and calls scheduler to set wait period over
      CORE_DECLARE_IRQ_STATE;
      CORE_ENTER_CRITICAL();
      schedulerSetTimerComp1Event();
      CORE_EXIT_CRITICAL();
  }
}


/*******************************************************************************
 * Calls scheduler to set an event when I2C interrupts occur
 ******************************************************************************/
void I2C0_IRQHandler(void) {
  I2C_TransferReturn_TypeDef transferStatus;
  transferStatus = I2C_Transfer(I2C0);

  if (transferStatus == i2cTransferDone) {
      schedulerSetI2CEventComplete ();
  }
  else if (transferStatus < 0) {
      LOG_ERROR("%d", transferStatus);
      schedulerSetI2CEventFail ();
  }
} // I2C0_IRQHandler()


// ISR for Button 0
void GPIO_EVEN_IRQHandler()  {
  uint32_t flags = GPIO_IntGetEnabled();

  GPIO_IntClear(flags);

  LOG_INFO("Flags = %d",flags);

  if( flags == (1 << BUTTON_1_PIN)) {
      LOG_INFO("1 Pressed");
      schedulerSetEventB1Pressed();
  }

  if( flags == (1 << BUTTON_3_PIN)) {
      LOG_INFO("3 Pressed");
      schedulerSetEventB3Pressed();
  }

  if( flags == (1 << PB0_pin)) {
      LOG_INFO("PB0 Pressed");
      schedulerSetEventPB0Pressed();
  }
}   //    GPIO_EVEN_IRQHandler()


// ISR for Button 1
void GPIO_ODD_IRQHandler()  {
  uint32_t flags = GPIO_IntGetEnabled();

  GPIO_IntClear(flags);

  LOG_INFO("Flags = %d",flags);

  if( flags == (1 << BUTTON_2_PIN)) {
      LOG_INFO("2 Pressed");
      schedulerSetEventB2Pressed();
  }

  if( flags == (1 << BUTTON_4_PIN)) {
      LOG_INFO("4 Pressed");
      schedulerSetEventB4Pressed();
  }

  if( flags == (1 << PB1_pin)) {
      LOG_INFO("PB1 Pressed");
      schedulerSetEventPB1Pressed();
  }
}   //    GPIO_ODD_IRQHandler()
