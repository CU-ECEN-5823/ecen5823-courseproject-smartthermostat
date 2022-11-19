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
   evtLETIMER0_UF        = 1,
   evtLETIMER0_COMP1     = 2,
   evtI2C_Done           = 3,
   evtPB0_Pressed        = 4,
   evtPB1_Pressed        = 5,
} event_type_t;

typedef enum {
  IdleState,
  PowerOn,
  SiWrite,
  SiRead,
  PowerOff
//  n_states
} states_t;

typedef enum {
  Connected,
  DiscoverTemperatureServices,
  DiscoverButtonServices,
  DiscoverTemperatureCharacteristics,
  DiscoverButtonCharacteristics,
  SetTemperatureNotification,
  SetButtonNotification
  //  n_states
} discover_states_t;


// Function prototypes
void schedulerSetEventUF(void);
void schedulerSetEventCOMP1(void);
void schedulerSetEventI2CDone(void);
void schedulerSetEventPB0Pressed(void);
void schedulerSetEventPB1Pressed(void);
//uint32_t getNextEvent(void);
void temperature_state_machine(sl_bt_msg_t *event);
void discover_state_machine(sl_bt_msg_t *event);

#endif  /* SCHEDULER_H */
