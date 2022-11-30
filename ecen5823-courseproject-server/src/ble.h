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
#define MAX_SESSION_SCANS 50
#define MAX_FAILED_SCANS 5


typedef enum {
  CONN_STATE_UNKNOWN = 0,
  CONN_STATE_SCANNING = 1,
  CONN_STATE_CONNECTING = 2,
  CONN_STATE_CONNECTED = 4,
  CONN_STATE_BONDING = 8,
  CONN_STATE_BONDED = 16,
  CONN_STATE_DISCONNECTED = 32,
  CONN_STATE_NOT_FOUND = 64
}client_conn_state_t;


typedef struct {
  bd_addr addr;
  const uint8_t addr_type;

  uint8_t conn_handle;
  uint8_t bond_handle;
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
  uint8_t session_scan_count;
  uint8_t failed_scan_count;
}ble_server_data_t;


void ble_init();
void handle_ble_event(sl_bt_msg_t *evt);
void start_manual_scan();
void update_lcd();
ble_server_data_t* get_ble_server_data();


#endif /* SRC_BLE_H_ */
