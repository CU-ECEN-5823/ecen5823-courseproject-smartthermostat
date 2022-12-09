/*******************************************************************************
 * @file  scheduler.h
 * @brief Header file
 *******************************************************************************
 * Editor: Dec 08, 2022, Amey More
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
