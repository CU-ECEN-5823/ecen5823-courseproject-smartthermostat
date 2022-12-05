/*******************************************************************************
 * @file    ble.h
 * @brief   Function definitions for handling BT events
 *
 * @editor  Oct 17, 2022, Ajay Kandagal
 * @change  Added code to initiate bonding from client to read PB0
 *          characteristic.
 *
 * @editor  Dec 2, 2022, Ajay Kandagal
 * @change  Added current temperature update, setting target temperature, logic
 *          to turn on/off the clients manually (auto feature off) and
 *          automatically (auto feature on) and manual triggering of scanning
 *          using PB0 event.
 *
 ******************************************************************************/
#ifndef SRC_BLE_H_
#define SRC_BLE_H_


#include "em_common.h"
#include "sl_bluetooth.h"


#define MAX_SESSION_SCANS 20
#define LCD_TIMEOUT_PERIOD 10


typedef enum {
  CLIENT_TYPE_AC = 1,
  CLIENT_TYPE_HEATER
}client_type_t;

typedef enum {
  CLIENT_STATE_OFF = 0,
  CLIENT_STATE_ON
}client_state_t;

typedef enum {
  CONN_STATE_UNKNOWN =    (1 << 0),
  CONN_STATE_SCANNING =   (1 << 1),
  CONN_STATE_CONNECTING = (1 << 2),
  CONN_STATE_CONNECTED =  (1 << 3),
  CONN_STATE_BONDING =    (1 << 4),
  CONN_STATE_PASSKEY =    (1 << 5),
  CONN_STATE_BONDED =     (1 << 6),
  CONN_STATE_NOT_BONDED = (1 << 7),
  CONN_STATE_DISCONNECTED = (1 << 8),
  CONN_STATE_NOT_FOUND =  (1 << 9)
}client_conn_state_t;


typedef struct {
  const bd_addr addr;
  const client_type_t client_type;
  uint8_t conn_handle;
  uint8_t bond_handle;
  client_conn_state_t conn_state;
  client_state_t onoff_state;
  uint8_t gatt_ack_pending;
  uint8_t indications_enabled;
}client_data_t;

typedef struct {
  bd_addr addr;
  uint8_t addr_type;
  uint8_t adv_handle;
  int16_t current_temp;
  int16_t target_temp;
  uint8_t offset_temp;
  uint8_t session_scans_count;
  uint8_t automatic_temp_control;
  client_data_t *clients_data;
  uint8_t clients_count;
  uint8_t lcd_on;
  uint8_t lcd_on_timeout;
}server_data_t;


/******************************************************************************
 * @brief Initializes the LCD display and sets the BT soft timer
 ******************************************************************************/
void ble_init(void);


/******************************************************************************
 * @brief   Toggles the state of the connected client On/Off state
 *
 * @param
 *  client_type   The appliance type that the client controls. It can be either
 *                AC/Heater.
 *
 ******************************************************************************/
void toggle_client_state(client_type_t client_type);


/******************************************************************************
 * @brief   Toggles the auto On/Off feature. If this feature is On then the
 * server will automatically turn On/Off respective client based on current
 * temperature and target temperature.
 *
 ******************************************************************************/
void toggle_auto_feature(void);


/******************************************************************************
 * @brief   Updates the current temperature and displays the same on the LCD.
 * In Auto feature On mode, if the current temperature drops below the target
 * temperature then Heater will be turned On and when the current temperature
 * goes above the target temperature then AC will be turned On.
 *
 * @param
 *  temp    The current measured temperature from temperature sensor
 *
 ******************************************************************************/
void update_current_temperature(int16_t temp);


/******************************************************************************
 * @brief   Increases the value of the target temperature value if its value
 * is below the limit 125
 *
 ******************************************************************************/
void increase_taget_temperature(void);


/******************************************************************************
 * @brief   Decreases the value of the target temperature value if its value
 * is above the limit 0
 *
 ******************************************************************************/
void decrease_taget_temperature(void);


/******************************************************************************
 * @brief   The PB0 event is used in contexts. When the server is bonding with
 * one of the clients then PB0 is used as confirmation for pass-key. Otherwise,
 * if one of the clients is not connected the PB0 event is used to initiate
 * the scanning.
 *
 ******************************************************************************/
void pb0_event_handle(void);


/******************************************************************************
 * @brief   Common handler for handling all the BLE events such as scanning,
 * connecting and bonding.
 *
 * @param
 *  *evt    Data structure of BT API message
 *
 ******************************************************************************/
void handle_ble_event(sl_bt_msg_t *evt);

uint8_t lcd_on_status();
void set_lcd_off();
void set_lcd_on();


#endif /* SRC_BLE_H_ */
