/*******************************************************************************
 * @file        scheduler.h
 * @brief       Provides interfaces to set the events and execute the LM75
 *              state machine.
 * @author      Amey More, Amey.More@colorado.edu
 *
 * @due         Nov 24, 2022
 * @project     ecen5823-courseproject-server
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Fall 2022)
 * @instructor  David Sluiter
 *
 * @editor      Nov 28, 2022, Ajay Kandagal, ajka9053@colorado.edu
 * @change      Added interfaces for setting I2C and LETIMER0 events and an
 *              interface to execute LM75 temperature sensor state machine.
 *
 ******************************************************************************/
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#include "sl_bt_api.h"

// Function prototypes
void schedulerSetEventPB0Pressed(void);
void schedulerSetEventPB1Pressed(void);
void schedulerSetEventB1Pressed(void);
void schedulerSetEventB2Pressed(void);
void schedulerSetEventB3Pressed(void);
void schedulerSetEventB4Pressed(void);
void schedulerSetI2CEventComplete(void);
void schedulerSetI2CEventFail(void);
void schedulerSetTimerComp0Event(void);
void schedulerSetTimerComp1Event(void);

void temperatureStateMachine(sl_bt_msg_t *evt);


#endif  /* SCHEDULER_H */
