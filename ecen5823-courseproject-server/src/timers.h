/*******************************************************************************
 * @file    timers.h
 * @brief   Has functions to set the LETIMER0 to generate interrupts with COMP0
 *          and COMP1 overflow. Also contains to generate required delay using
 *          COMP1 interrupts.
 *
 * @author  Ajay Kandagal, ajka9053@colorado.edu
 * @date    Nov 27, 2022
 *
 ******************************************************************************/
#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_


#include "em_common.h"


/*******************************************************************************
 * Sets COMP1 and COMP0 of LETIMER0 to generate two interrupts at comp1_ms and
 * comp0_ms intervals respectively.
 * NOTE: comp1_ms should always be less than comp0_ms value.
 *
 * @param     comp1_ms    Value in milliseconds
 * @param     comp0_ms    Value in milliseconds
 *
 ******************************************************************************/
void init_LETIMER0(uint16_t comp1_ms, uint16_t comp0_ms);


/*******************************************************************************
 * This function creates mili-second delay using LETIMER0 COMP1 interrupt.
 *
 * @param     us_wait    Value in microseconds, to be generated by COMP1 OV
 *
 ******************************************************************************/
void timerWaitUs_irq(uint32_t us_wait);


#endif /* SRC_TIMERS_H_ */
