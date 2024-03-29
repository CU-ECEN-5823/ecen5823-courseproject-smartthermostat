/*******************************************************************************
 * @file  irq.c
 * @brief Interrupt functions
 *******************************************************************************
 * Editor: Dec 08, 2022, Amey More
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

// This function performs the following
// 1. Clear pending interrupts
// 2. Enable required interrupt
void IRQ_Init(){

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);

  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

} // IRQ_Init()

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
