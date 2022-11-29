/***************************************************************************//**
 * @file  ble.h
 * @brief Function definitions for handling BT events
 *******************************************************************************
 *
 * Editor: Oct 17, 2022, Ajay Kandagal
 * Change: Added code to initiate bonding from client to read PB0 characteristic
******************************************************************************/
#ifndef SRC_BLE_H_
#define SRC_BLE_H_


#include "em_common.h"
#include "sl_bluetooth.h"

#define CLIENTS_NUM 2


typedef enum {
  CONN_STATE_UNKNOWN,
  CONN_STATE_SCANNING,
  CONN_STATE_CONNECTING,
  CONN_STATE_CONNECTED,
  CONN_STATE_BONDING,
  CONN_STATE_BONDED,
  CONN_STATE_DISCONNECTED
}client_conn_state_t;


typedef struct {
  bd_addr addr;
  const uint8_t addr_type;

  uint8_t conn_handle;
  client_conn_state_t conn_state;

  bool onoff_state;
  bool gatt_ack_pending;
}ble_client_data_t;

typedef struct {
  bd_addr addr;
  uint8_t addr_type;
  int8_t current_temp;
  int8_t target_temp;
  uint8_t offset_temp;
  bool waiting_for_user_input;
  bool automatic_temp_control;
  ble_client_data_t ble_clients[CLIENTS_NUM]; // 0: AC, 1: Heater
}ble_server_data_t;


void ble_init();
void handle_ble_event(sl_bt_msg_t *evt);
ble_server_data_t* get_ble_server_data();
void update_lcd();


#endif /* SRC_BLE_H_ */
