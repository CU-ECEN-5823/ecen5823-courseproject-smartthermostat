/*******************************************************************************
 * @file        oscillators.c
 * @brief       Has functions to initialize LXFO and ULFRCO.
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
#include "oscillators.h"
#include "em_cmu.h"


/*******************************************************************************
 * This function selects ULFRCO oscillator with 1 KHz for LETIMER0 clock tree.
 ******************************************************************************/
void init_ULFRCO()
{
  CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);

  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);

  CMU_ClockEnable(cmuClock_LFA, true);

  CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_1);

  CMU_ClockEnable(cmuClock_LETIMER0, true);
}


/*******************************************************************************
 * This function selects LFXO oscillator with 32.768 KHz for LETIMER0.
 ******************************************************************************/
void init_LFXO()
{
  CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

  CMU_ClockEnable(cmuClock_LFA, true);

  // Pre-scaling helps in maintaining comp0 and comp1 value under 16bit
  // range for 7 second time-period
  CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);

  CMU_ClockEnable(cmuClock_LETIMER0, true);
}

