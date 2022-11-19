/*******************************************************************************
 * @file        timers.c
 * @brief       Timer enable and compare set
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
 * @resources
 ******************************************************************************/

#include <em_cmu.h>
#include <stdbool.h>
#include <app_log.h>
#include "em_letimer.h"
#include "app.h"
#include "timers.h"
#include "gpio.h"
#include "scheduler.h"

#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#define ACTUAL_CLK_FREQ ( CMU_ClockFreqGet(cmuClock_LETIMER0) )
#define TOP_VALUE ( (( LETIMER_PERIOD_MS * ACTUAL_CLK_FREQ)/1000) )
//#define ON_VALUE ( (( LETIMER_ON_TIME_MS * ACTUAL_CLK_FREQ)/1000) )

// This function performs the following
// 1. Initialize LETIMER
// 2. Loads COMP0 and COMP1 value
// 3. Enable LETIMER
// 4. Enable LETIMER Interrupt
void LETIMER0_init(){
  const LETIMER_Init_TypeDef letimerInitData = {
      false,                 // enable;   don't enable when init completes, we'll enable last
      true,                  // debugRun; useful to have the timer running when stopped in the debugger
      true,                  // comp0Top; load COMP0 into CNT on underflow
      false,                 // bufTop;   don't load COMP1 into COMP0 when REP0==0
      0,                     // out0Pol;  0 default output pin value
      0,                     // out1Pol;  0 default output pin value
      letimerUFOANone,       // ufoa0;    no underflow output action
      letimerUFOANone,       // ufoa1;    no underflow output action
      letimerRepeatFree,     // repMode;  free running mode i.e. load & go forever
      0                      // top Value, we calc this below
    };

  LETIMER_Init  ( LETIMER0, &letimerInitData);

  LETIMER_CompareSet ( LETIMER0, 0, TOP_VALUE );

  LETIMER_Enable(LETIMER0, true );
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP0);

}   //    LETIMER0_init()

// This function generates a delay
// of us_wait micro-seconds with polling
void timerWaitUs_polled(uint32_t us_wait) {
  uint32_t delay,ticks;

  if( (us_wait/1000) > LETIMER_PERIOD_MS) {
      LOG_ERROR("Wait requested is outside range");
      us_wait = (LETIMER_PERIOD_MS*1000);
  }

  delay = (( (us_wait/1000) * ACTUAL_CLK_FREQ)/1000);
  ticks = LETIMER_CounterGet(LETIMER0);

  while(delay) {
      if(ticks != LETIMER_CounterGet(LETIMER0)) {
          delay--;
          ticks = LETIMER_CounterGet(LETIMER0);
      }
  }
}   //    timerWaitUs_polled()

// This function generates a delay
// of us_wait micro-seconds with interrupt
void timerWaitUs_irq(uint32_t us_wait) {
  uint32_t  wait_value, COUNT_VALUE;
  uint32_t  ms_wait = us_wait/1000;

  if(ms_wait > LETIMER_PERIOD_MS) {
      LOG_ERROR("Wait exceeded limit");
      ms_wait = LETIMER_PERIOD_MS;
  }

  wait_value = (ms_wait * ACTUAL_CLK_FREQ)/1000;

  if(wait_value < LETIMER_CounterGet(LETIMER0)) {
      COUNT_VALUE = (LETIMER_CounterGet(LETIMER0) - wait_value);
  }
  else {
      COUNT_VALUE = (TOP_VALUE - (wait_value - LETIMER_CounterGet(LETIMER0)));
  }

  LETIMER_CompareSet ( LETIMER0, 1, COUNT_VALUE );

  LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);

  LETIMER0->IEN |= LETIMER_IEN_COMP1;

}   //    timerWaitUs_irq()
