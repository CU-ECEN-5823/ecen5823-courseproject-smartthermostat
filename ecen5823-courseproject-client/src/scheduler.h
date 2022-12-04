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
   evtPB0_Pressed        = 4,
   evtPB1_Pressed        = 5,
} event_type_t;

typedef enum {
  Advertising,
  Connected,
  ConfirmBonding,
  ConfirmPasskey,
  Bonded,
  DiscoverServices,
  DiscoverCharacteristics,
  SetNotification
//  n_states
} connection_states_t;

// Function prototypes
void schedulerSetEventPB0Pressed(void);
void schedulerSetEventPB1Pressed(void);

void connection_state_machine(sl_bt_msg_t *event);

#endif  /* SCHEDULER_H */
