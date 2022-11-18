/*******************************************************************************
 * @file        timer.h
 * @brief       Timer header file
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

#ifndef TIMER_H
#define TIMER_H

// Function prototypes
void LETIMER0_init();
void timerWaitUs_polled(uint32_t us_wait);
void timerWaitUs_irq(uint32_t us_wait);

#endif  /* TIMER_H_ */
