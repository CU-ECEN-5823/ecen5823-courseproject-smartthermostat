/*******************************************************************************
 * @file        irq.h
 * @brief       Interrupt header file
 * @author      Amey More, Amey.More@colorado.edu
 *
 * @due         Nov 24, 2022
 * @project     ecen5823-courseproject-server
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Fall 2022)
 * @instructor  David Sluiter
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
