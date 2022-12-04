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

  uint32_t  HeaterServiceHandle;
  uint16_t  HeaterCharacteristicsHandle;

  uint32_t  ACServiceHandle;
  uint16_t  ACCharacteristicsHandle;

  bool      connectionFlag;

  connection_states_t stateTransition;

} ble_client_data_t;

// 21685485-b057-4cc5-bed4-f18cdfd32de3
static const uint8_t heater_service[]   = { 0xe3, 0x2d, 0xd3, 0xdf, 0x8c, 0xf1, 0xd4, 0xbe, 0xc5, 0x4c, 0x57, 0xb0, 0x85, 0x54, 0x68, 0x21 };
// 4abf55f3-f376-44c8-8dce-d767f2818227
static const uint8_t heater_char[]      = { 0x27, 0x82, 0x81, 0xf2, 0x67, 0xd7, 0xce, 0x8d, 0xc8, 0x44, 0x76, 0xf3, 0xf3, 0x55, 0xbf, 0x4a };

// 1032814e-7df0-4c6f-8c9f-a6735f9baa00
static const uint8_t ac_service[]       = { 0x00, 0xaa, 0x9b, 0x5f, 0x73, 0xa6, 0x9f, 0x8c, 0x6f, 0x4c, 0xf0, 0x7d, 0x4e, 0x81, 0x32, 0x10 };
// cf527086-8ccd-4ca5-8b48-85d5a0516d6d
static const uint8_t ac_char[]          = { 0x6d, 0x6d, 0x51, 0xa0, 0xd5, 0x85, 0x48, 0x8b, 0xa5, 0x4c, 0xcd, 0x8c, 0x86, 0x70, 0x52, 0xcf };

// Function Prototypes
ble_client_data_t *getbleData();
void handle_ble_event(sl_bt_msg_t *evt);

void handle_bt_boot();
void handle_bt_open(sl_bt_msg_t *evt);
void handle_bt_confirm_bonding();
void handle_bt_confirm_paskey(sl_bt_msg_t *evt);
void handle_bt_external_signals(sl_bt_msg_t *evt);
void handle_bt_bonded();
void handle_bt_gatt_complete();
void handle_bt_close();

#endif    //    BLE_H
