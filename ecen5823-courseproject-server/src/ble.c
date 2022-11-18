/***************************************************************************//**
 * @file  ble.c
 * @brief Function definitions for handling BT events
 *******************************************************************************
 *
 * Editor: Oct 17, 2022, Ajay Kandagal
 * Change: Added code to initiate bonding from client to read PB0 characteristic
 ******************************************************************************/
#include "ble.h"
#include "lcd.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define PASSIVE_SCANNING 0
#define SCAN_INTERVAL 80          // => 50ms / 0.625ms = 80
#define SCAN_WINDOW 40            // => 25ms / 0.625ms = 40

#define CONNECTION_INTERVAL 75    // => 75ms / 1.25ms = 60
#define CONNECTION_LATENCY 4
#define CONNECTION_TIMEOUT 750    // => (1 + @p latency) * @p max_interval * 2 = (1 + 4) * 75 * 2 = 750ms
#define CONNECTION_MIN_CE 0
#define CONNECTION_MAX_CE 4

#define DEBUG

ble_server_data_t ble_data;

/******************************************************************************
 *
 * @brief Initializes all the fields of ble_data
 *
 ******************************************************************************/
void init_ble()
{
  displayInit();

  ble_data.current_temp = 0;
  ble_data.target_temp = 0;
  ble_data.offset_temp = 1;
  ble_data.waiting_for_user_input = false;
  ble_data.automatic_temp_control = true;
  ble_data.ble_client_devices_count = 0;
  ble_data.ble_client_ac.conn_state = UNKNOWN;
  ble_data.ble_client_heater.conn_state = UNKNOWN;
  uint8_t ac_addr[] = {0x3f, 0x4a, 0xa6, 0x14, 0x2e, 0x84};
  memcpy(ble_data.ble_client_ac.addr.addr, ac_addr, 6);
}


/******************************************************************************
 *
 * @brief Updates the Client info on the LCD
 *
 ******************************************************************************/
void update_lcd()
{
  char ac_string[20] = "AC: ";
  char heater_string[20] = "Heater: ";

  switch (ble_data.ble_client_ac.conn_state)
  {
    case UNKNOWN:
      strcat(ac_string, "UNKNOWN / ");
      break;
    case SCANNING:
      strcat(ac_string, "SCANNING / ");
      break;
    case CONNECTED:
      strcat(ac_string, "CONNECTED / ");
      break;
    case BONDING:
      strcat(ac_string, "BONDING / ");
      break;
    case BONDED:
      strcat(ac_string, "BONDED / ");
      break;
    case DISCONNECTED:
      strcat(ac_string, "DISCONNECTED / ");
      break;
  }

  switch (ble_data.ble_client_heater.conn_state)
  {
    case UNKNOWN:
      strcat(heater_string, "UNKNOWN / ");
      break;
    case SCANNING:
      strcat(heater_string, "SCANNING / ");
      break;
    case CONNECTED:
      strcat(heater_string, "CONNECTED / ");
      break;
    case BONDING:
      strcat(heater_string, "BONDING / ");
      break;
    case BONDED:
      strcat(heater_string, "BONDED / ");
      break;
    case DISCONNECTED:
      strcat(heater_string, "DISCONNECTED / ");
      break;
  }

  if (ble_data.ble_client_ac.onoff_state)
    strcat(ac_string, "ON");
  else
    strcat(ac_string, "OFF");

  if (ble_data.ble_client_heater.onoff_state)
    strcat(heater_string, "ON");
  else
    strcat(heater_string, "OFF");

  displayPrintf(DISPLAY_ROW_NAME, "Smart Thermostat");
  displayPrintf(DISPLAY_ROW_BTADDR2, heater_string);
  displayPrintf(DISPLAY_ROW_CLIENTADDR, ac_string);
  displayPrintf(DISPLAY_ROW_TEMPVALUE, "Curr Temp : %dC", ble_data.current_temp);
  displayPrintf(DISPLAY_ROW_8, "Target Temp : %dC", ble_data.target_temp);
  displayPrintf(DISPLAY_ROW_ASSIGNMENT, "Course Project");
}

/******************************************************************************
 *
 * @brief Handles client boot event
 *
 ******************************************************************************/
void handle_bt_boot()
{
  sl_status_t status;

  status = sl_bt_system_get_identity_address(&ble_data.addr, &ble_data.addr_type);
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to get the BT address");

  status = sl_bt_scanner_set_mode(sl_bt_gap_1m_phy, PASSIVE_SCANNING);
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to set scanner mode");

  status = sl_bt_scanner_set_timing(sl_bt_gap_1m_phy, SCAN_INTERVAL, SCAN_WINDOW);
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to set scanner timing");

  status = sl_bt_connection_set_default_parameters(CONNECTION_INTERVAL, \
                                                   CONNECTION_INTERVAL, \
                                                   CONNECTION_LATENCY, \
                                                   CONNECTION_TIMEOUT, \
                                                   CONNECTION_MIN_CE, \
                                                   CONNECTION_MAX_CE);
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to set connection parameters");

  // Sets the bonding requirements and IO capability
  status = sl_bt_sm_configure(0x0F, sm_io_capability_displayyesno);
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to configure security");

  status = sl_bt_sm_set_bondable_mode(1);
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to set bonding mode");

  status = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
  ble_data.ble_client_ac.conn_state = SCANNING;
  ble_data.ble_client_heater.conn_state = SCANNING;

  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to start scanning");

  update_lcd();

#ifdef DEBUG
  LOG_INFO("BLE Client: Scanning started\n");
#endif
}

/******************************************************************************
 *
 * @brief Handles client scanned event
 *
 ******************************************************************************/
void handle_bt_scanned()
{
  sl_status_t status;

  status = sl_bt_scanner_stop();
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to stop scanning");

  if (ble_data.ble_client_ac.conn_state == UNKNOWN || ble_data.ble_client_ac.conn_state == SCANNING) {
      status = sl_bt_connection_open(ble_data.ble_client_ac.addr, \
                                     sl_bt_gap_public_address, \
                                     sl_bt_gap_1m_phy, \
                                     &ble_data.ble_client_ac.conn_handle);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to start connection with AC");
  }
  else if (ble_data.ble_client_heater.conn_state == UNKNOWN || ble_data.ble_client_heater.conn_state == SCANNING) {
      status = sl_bt_connection_open(ble_data.ble_client_heater.addr, \
                                     sl_bt_gap_public_address, \
                                     sl_bt_gap_1m_phy, \
                                     &ble_data.ble_client_heater.conn_handle);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to start connection with Heater");
  }

#ifdef DEBUG
  LOG_INFO("BLE Client: Trying to connect\n");
#endif
}

/******************************************************************************
 *
 * @brief Handles client connection opened event
 *
 * @param *evt BT on event value
 *
 ******************************************************************************/
void handle_bt_opened(sl_bt_msg_t *evt)
{
  sl_status_t status;

  if (!memcmp(evt->data.evt_connection_opened.address.addr, ble_data.ble_client_ac.addr.addr, 6)) {
      ble_data.ble_client_ac.conn_state = CONNECTED;
      ble_data.ble_client_ac.conn_handle = evt->data.evt_connection_opened.connection;
#ifdef DEBUG
      LOG_INFO("AC Connected\n");
#endif
  }
  else if (!memcmp(evt->data.evt_connection_opened.address.addr, ble_data.ble_client_ac.addr.addr, 6)) {
      ble_data.ble_client_heater.conn_state = CONNECTED;
      ble_data.ble_client_heater.conn_handle = evt->data.evt_connection_opened.connection;
#ifdef DEBUG
      LOG_INFO("Heater Connected\n");
#endif
  }

  status = sl_bt_sm_delete_bondings();

  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to delete bondings");

  update_lcd();
}


/******************************************************************************
 *
 * @brief Handles client connection closed event
 *
 * @param *evt BT on event value
 *
 ******************************************************************************/
void handle_bt_closed(sl_bt_msg_t *evt)
{
  if (evt->data.evt_connection_closed.connection == ble_data.ble_client_ac.conn_handle) {
      ble_data.ble_client_ac.conn_state = DISCONNECTED;
      ble_data.ble_client_ac.conn_handle = 0;

#ifdef DEBUG
      LOG_INFO("AC Disconnected\n");
#endif
  }
  else if (evt->data.evt_connection_closed.connection == ble_data.ble_client_ac.conn_handle) {
      ble_data.ble_client_heater.conn_state = DISCONNECTED;
      ble_data.ble_client_heater.conn_handle = 0;

#ifdef DEBUG
      LOG_INFO("Heater Disconnected\n");
#endif
  }

  sl_status_t status = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_limited);
  ble_data.ble_client_ac.conn_state = SCANNING;
  ble_data.ble_client_heater.conn_state = SCANNING;

  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to start BT scanning");

  update_lcd();
}

/******************************************************************************
 *
 * @brief Event handler for BLE events
 *
 * @param *evt BT on event value
 *
 ******************************************************************************/
void handle_ble_event(sl_bt_msg_t *evt)
{
  switch (SL_BT_MSG_ID(evt->header))
  {
    case sl_bt_evt_system_boot_id:
      handle_bt_boot();
      break;
    case sl_bt_evt_connection_opened_id:
      handle_bt_opened(evt);
      break;
    case sl_bt_evt_connection_closed_id:
      handle_bt_closed(evt);
      break;
    case sl_bt_evt_scanner_scan_report_id:
      handle_bt_scanned();
      break;
    case sl_bt_evt_system_soft_timer_id:
      displayUpdate();
      break;
  } // end - switch
} // handle_ble_event()

ble_server_data_t* get_ble_data()
{
  return &ble_data;
}
