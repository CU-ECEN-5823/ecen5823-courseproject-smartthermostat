/*******************************************************************************
 * @file        ble.h
 * @brief       BLE Header File
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

#ifndef BLE_H
#define BLE_H

#include "sl_bluetooth.h"
#include "scheduler.h"

typedef struct {
  bd_addr   myAddress;
  uint8_t   myAddress_type;

  uint8_t   advertisingHandle;
  uint8_t   connectionHandle;

  uint32_t  HTMServiceHandle;
  uint16_t  HTMCharacteristicsHandle;

  bool      connectionFlag;

} ble_client_data_t;

static const uint8_t thermo_service[2] = { 0x09, 0x18 };
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };

// Function Prototypes
ble_client_data_t *getbleData();
void handle_ble_event(sl_bt_msg_t *evt);


#endif    //    BLE_H
