/*******************************************************************************
 * @file        oscillators.h
 * @brief       Has interfaces to initialize LXFO and ULFRCO.
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
#ifndef SRC_OSCILLATORS_H_
#define SRC_OSCILLATORS_H_


/*******************************************************************************
 * This function selects ULFRCO oscillator with 1 KHz for LETIMER0 clock tree.
 ******************************************************************************/
void init_ULFRCO();


/*******************************************************************************
 * This function selects LFXO oscillator with 32.768 KHz for LETIMER0.
 ******************************************************************************/
void init_LFXO();


#endif /* SRC_OSCILLATORS_H_ */
