/*******************************************************************************
 * @file    irq.h
 * @brief   Interrupt Service Routines and Interrupt Enable.
 *
 * @author  Amey More, Amey.More@colorado.edu
 * @date    Nov 24, 2022
 *
 * @editor  Nov 28, 2022, Ajay Kandagal, ajka9053@colorado.edu
 * @change  Added IRQ Handlers for LETIMER0 and I2C0
 *
 * @editor  Dec 2, 2022, Ajay Kandagal
 * @change  Added code to handle PB0 and PB1 events.
 *
 ******************************************************************************/
#ifndef IRQ_H
#define IRQ_H


/*******************************************************************************
 * Clears the pending interrupts and enables then enables NVIC flag for
 * GPIO_EVEN_IRQn, GPIO_ODD_IRQn and LETIMER0_IRQn.
 ******************************************************************************/
void IRQ_Init();


#endif  /* IRQ_H_ */
