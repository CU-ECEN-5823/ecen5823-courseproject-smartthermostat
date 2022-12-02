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


typedef enum {
  CONN_STATE_UNKNOWN = 0,
  CONN_STATE_SCANNING = 1,
  CONN_STATE_CONNECTING = 2,
  CONN_STATE_CONNECTED = 4,
  CONN_STATE_BONDING = 8,
  CONN_STATE_PASSKEY = 16,
  CONN_STATE_BONDED = 32,
  CONN_STATE_NOT_BONDED = 64,
  CONN_STATE_DISCONNECTED = 128,
  CONN_STATE_NOT_FOUND = 256
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

  int16_t current_temp;
  int16_t target_temp;
  uint8_t offset_temp;

  uint8_t session_scans_count;

  uint8_t automatic_temp_control;

  ble_client_data_t ble_clients[CLIENTS_NUM]; // 0: Heater, 1: AC
}ble_server_data_t;


void ble_init();
void toggle_heater();
void toggle_ac();
void toggle_auto_feature();
void update_current_temperature(int16_t temp);
void increase_taget_temperature();
void decrease_taget_temperature();
void pb0_event_handle();
void handle_ble_event(sl_bt_msg_t *evt);

#endif /* SRC_BLE_H_ */
