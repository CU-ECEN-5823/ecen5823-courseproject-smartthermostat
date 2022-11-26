/*******************************************************************************
 * @file        scheduler.h
 * @brief       Schedule header file
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
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "sl_bt_api.h"

typedef enum {
  evtPB0_Pressed,
  evtPB1_Pressed,
  evtB1_Pressed,
  evtB2_Pressed,
  evtB3_Pressed,
  evtB4_Pressed
} event_type_t;

// Function prototypes

void schedulerSetEventPB0Pressed(void);
void schedulerSetEventPB1Pressed(void);
void schedulerSetEventB1Pressed(void);
void schedulerSetEventB2Pressed(void);
void schedulerSetEventB3Pressed(void);
void schedulerSetEventB4Pressed(void);


#endif  /* SCHEDULER_H */
