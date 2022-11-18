/*******************************************************************************
 * @file        oscillators.c
 * @brief       Selecting and Enabling Oscillators
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

#include <em_cmu.h>
#include <stdbool.h>
#include "oscillators.h"
#include "app.h"
#include <app_log.h>

#define PRESCALER_VALUE cmuClkDiv_4

// This function performs the following
// 1. Enable LFXO or ULFRCO
// 2. Select appropriate clock
// 3. Enable clock tree for LFA
// 4. Set Pre-scaler
// 5. Enable clock tree for LETIMER
void osc_init(){

  if(LOWEST_ENERGY_MODE == 3){
      CMU_OscillatorEnable ( cmuOsc_ULFRCO, true, true );
      CMU_ClockSelectSet ( cmuClock_LFA, cmuSelect_ULFRCO );
  }

  else if((LOWEST_ENERGY_MODE >= 0) && (LOWEST_ENERGY_MODE <= 2)){
      CMU_OscillatorEnable ( cmuOsc_LFXO, true, true );
      CMU_ClockSelectSet ( cmuClock_LFA, cmuSelect_LFXO );
  }

  CMU_ClockEnable ( cmuClock_LFA, true );

  CMU_ClockDivSet( cmuClock_LETIMER0, PRESCALER_VALUE );

  CMU_ClockEnable ( cmuClock_LETIMER0, true );

} //osc_init()
