/*******************************************************************************
 * @file        scheduler.c
 * @brief       Schedule and process events
 * @author      Amey More, Amey.More@colorado.edu
 * @date        Oct 24, 2022
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Fall 2022)
 * @instructor  David Sluiter
 *
 * @assignment  ecen5823-assignment9-ameyflash
 * @due         Oct 28, 2022
 *
 ******************************************************************************/

#include "scheduler.h"
#include "em_core.h"
#include "em_gpio.h"
#include <app_log.h>
#include "ble.h"
#include "lcd.h"
#include "math.h"

#define INCLUDE_LOG_DEBUG (1)
#include "log.h"


void schedulerSetEventPB0Pressed() {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

//  myEvents |= (1<<evtLETIMER0_COMP1);
  sl_bt_external_signal(evtPB0_Pressed);

  CORE_EXIT_CRITICAL();
}   //    schedulerSetEventPB0Pressed()

void schedulerSetEventPB1Pressed() {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

//  myEvents |= (1<<evtLETIMER0_COMP1);
  sl_bt_external_signal(evtPB1_Pressed);

  CORE_EXIT_CRITICAL();
}   //    schedulerSetEventPB1Pressed()

