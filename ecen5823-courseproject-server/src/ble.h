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


typedef enum {
  UNKNOWN,
  SCANNING,
  CONNECTED,
  BONDING,
  BONDED,
  DISCONNECTED
}client_conn_state_t;


typedef struct {
  bd_addr addr;
  const uint8_t addr_type;
  uint8_t conn_handle;
  client_conn_state_t conn_state;
  bool onoff_state;
  bool gatt_ack_pending;
}ble_client_data_t;

//const void *update_lcd();
//const void *set_onoff_state(bool state);
//const void *handle_rec_data();

typedef struct {
  bd_addr addr;
  uint8_t addr_type;
  int8_t current_temp;
  int8_t target_temp;
  uint8_t offset_temp;
  bool waiting_for_user_input;
  bool automatic_temp_control;
  ble_client_data_t ble_client_ac; // = {00, 0, 0, UNKNOWN, false, false};
  ble_client_data_t ble_client_heater; // = {00, 0, 0, UNKNOWN, false, false};
  uint8_t ble_client_devices_count;
}ble_server_data_t;


void ble_init();
void handle_ble_event(sl_bt_msg_t *evt);
ble_server_data_t* get_ble_data();


#endif /* SRC_BLE_H_ */
