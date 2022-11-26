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
#include "scheduler.h"

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


ble_server_data_t ble_server_data;


const uint8_t clients_addr[CLIENTS_NUM][6] = {{0x3f, 0x4a, 0xa6, 0x14, 0x2e, 0x84},   // AC
    {0x49, 0x4a, 0xa6, 0x14, 0x2e, 0x84}    // Heater
};


/******************************************************************************
 *
 * @brief Initializes all the fields of ble_server_data
 *
 ******************************************************************************/
void ble_init()
{
  displayInit();

  ble_server_data.current_temp = 0;
  ble_server_data.target_temp = 0;
  ble_server_data.offset_temp = 1;
  ble_server_data.waiting_for_user_input = false;
  ble_server_data.automatic_temp_control = true;

  for (int i = 0; i < CLIENTS_NUM; i++) {
      ble_server_data.ble_clients[i].conn_state = CONN_STATE_UNKNOWN;
      memcpy(ble_server_data.ble_clients[i].addr.addr, clients_addr[i], 6);
  }
}


ble_client_data_t* get_client_by_addr(bd_addr addr)
{
  for (int i = 0; i < CLIENTS_NUM; i++) {
      if (!memcmp(addr.addr, ble_server_data.ble_clients[i].addr.addr, 6)) {
          return &ble_server_data.ble_clients[i];
      }
  }
  return NULL;
}

ble_client_data_t* get_client_by_conn_handle(uint8_t conn_handle)
{
  for (int i = 0; i < CLIENTS_NUM; i++) {
      if (conn_handle == ble_server_data.ble_clients[i].conn_handle) {
          return &ble_server_data.ble_clients[i];
      }
  }
  return NULL;
}


ble_client_data_t* get_client_by_conn_state(client_conn_state_t conn_state)
{
  for (int i = 0; i < CLIENTS_NUM; i++) {
      if (conn_state == ble_server_data.ble_clients[i].conn_state) {
          return &ble_server_data.ble_clients[i];
      }
  }
  return NULL;
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

  switch (ble_server_data.ble_clients[0].conn_state)
  {
    case CONN_STATE_UNKNOWN:
      strcat(ac_string, "UNKNOWN");
      break;
    case CONN_STATE_SCANNING:
      strcat(ac_string, "SCANNING");
      break;
    case CONN_STATE_CONNECTING:
      strcat(ac_string, "CONNECTING");
      break;
    case CONN_STATE_CONNECTED:
      strcat(ac_string, "CONNECTED");
      break;
    case CONN_STATE_BONDING:
      strcat(ac_string, "BONDING");
      break;
    case CONN_STATE_BONDED:
      if (ble_server_data.ble_clients[0].onoff_state)
        strcat(ac_string, "ON");
      else
        strcat(ac_string, "OFF");
      break;
    case CONN_STATE_DISCONNECTED:
      strcat(ac_string, "DISCONNECTED");
      break;
  }

  switch (ble_server_data.ble_clients[1].conn_state)
  {
    case CONN_STATE_UNKNOWN:
      strcat(heater_string, "UNKNOWN");
      break;
    case CONN_STATE_SCANNING:
      strcat(heater_string, "SCANNING");
      break;
    case CONN_STATE_CONNECTING:
      strcat(ac_string, "CONNECTING");
      break;
    case CONN_STATE_CONNECTED:
      strcat(heater_string, "CONNECTED");
      break;
    case CONN_STATE_BONDING:
      strcat(heater_string, "BONDING");
      break;
    case CONN_STATE_BONDED:
      if (ble_server_data.ble_clients[1].onoff_state)
        strcat(heater_string, "ON");
      else
        strcat(heater_string, "OFF");
      break;
    case CONN_STATE_DISCONNECTED:
      strcat(heater_string, "DISCONNECTED");
      break;
  }

  displayPrintf(DISPLAY_ROW_NAME, "Smart Thermostat");
  displayPrintf(DISPLAY_ROW_BTADDR2, heater_string);
  displayPrintf(DISPLAY_ROW_CLIENTADDR, ac_string);
  displayPrintf(DISPLAY_ROW_TEMPVALUE, "Curr Temp : %dC", ble_server_data.current_temp);
  displayPrintf(DISPLAY_ROW_8, "Target Temp : %dC", ble_server_data.target_temp);
  displayPrintf(DISPLAY_ROW_ASSIGNMENT, "Course Project");
}

void start_bt_scan()
{
  sl_status_t status;

  ble_client_data_t *client = get_client_by_conn_state(CONN_STATE_SCANNING);

  if (client == NULL)
    client = get_client_by_conn_state(CONN_STATE_DISCONNECTED);

  if (client != NULL) {
      client->conn_state = CONN_STATE_SCANNING;
      status = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to start scanning\n");
      else
        LOG_INFO("Succeeded to start scanning\n");

      update_lcd();
  }
}

/******************************************************************************
 *
 * @brief Handles client boot event
 *
 ******************************************************************************/
void handle_bt_boot()
{
  sl_status_t status;

  status = sl_bt_system_get_identity_address(&ble_server_data.addr, &ble_server_data.addr_type);
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

  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to start scanning");
  else
    LOG_INFO("Succeeded to start scanning");

  for (int i = 0; i < CLIENTS_NUM; i++)
    ble_server_data.ble_clients[i].conn_state = CONN_STATE_SCANNING;

  update_lcd();
}

/******************************************************************************
 *
 * @brief Handles client scanned event
 *
 ******************************************************************************/
void handle_bt_scanned(sl_bt_msg_t *evt)
{
  sl_status_t status;

  status = sl_bt_scanner_stop();

  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to stop scanning\n");

  ble_client_data_t *client = get_client_by_addr(evt->data.evt_scanner_scan_report.address);

  if (client != NULL) {
      client->conn_state = CONN_STATE_CONNECTING;

      status = sl_bt_connection_open(client->addr, \
                                     sl_bt_gap_public_address, \
                                     sl_bt_gap_1m_phy, \
                                     &client->conn_handle);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to start connection\n");
      else
        LOG_ERROR("Succeeded to start connection\n");

      update_lcd();
  }
  else {
      start_bt_scan();
  }
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

  ble_client_data_t *client = get_client_by_addr(evt->data.evt_scanner_scan_report.address);

  if (client != NULL) {
      client->conn_state = CONN_STATE_CONNECTED;

      client->conn_handle = evt->data.evt_connection_opened.connection;

      LOG_INFO("Connected\n");

      status = sl_bt_sm_increase_security(client->conn_handle);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to start bonding\n");
      else
        LOG_INFO("Succeeded to start bonding\n");

      update_lcd();
  }
}


/******************************************************************************
 *
 * @brief Handles client connection opened event
 *
 * @param *evt BT on event value
 *
 ******************************************************************************/
void handle_bt_confirm_bonding(sl_bt_msg_t *evt)
{
  sl_status_t status;

  ble_client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      client->conn_state = CONN_STATE_BONDING;

      status = sl_bt_sm_bonding_confirm(client->conn_handle, 1);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to confirm bonding\n");
      else
        LOG_INFO("Succeeded to confirm bonding\n");

      update_lcd();
  }
}


/******************************************************************************
 *
 * @brief Handles client connection opened event
 *
 * @param *evt BT on event value
 *
 ******************************************************************************/
void handle_bt_confirm_passkey(sl_bt_msg_t *evt)
{
  sl_status_t status;

  ble_client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      client->conn_state = CONN_STATE_BONDING;
      displayPrintf(DISPLAY_ROW_PASSKEY, "AC Passkey %u", evt->data.evt_sm_confirm_passkey.passkey);
      displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
      status = sl_bt_sm_passkey_confirm(client->conn_handle, 1);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to confirm passkey\n");
      else
        LOG_INFO("Succeeded to confirm passkey\n");

      update_lcd();
  }
}


/******************************************************************************
 *
 * @brief Handles client connection opened event
 *
 * @param *evt BT on event value
 *
 ******************************************************************************/
void handle_bt_bonded(sl_bt_msg_t *evt)
{
  ble_client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      client->conn_state = CONN_STATE_BONDED;
      displayPrintf(DISPLAY_ROW_PASSKEY, "");
      displayPrintf(DISPLAY_ROW_ACTION, "");
  }

  start_bt_scan();
}


/******************************************************************************
 *
 * @brief Handles client connection opened event
 *
 * @param *evt BT on event value
 *
 ******************************************************************************/
void handle_bt_bonding_failed(sl_bt_msg_t *evt)
{
  sl_status_t status;

  ble_client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      client->conn_state = CONN_STATE_BONDING;
      displayPrintf(DISPLAY_ROW_PASSKEY, "");
      displayPrintf(DISPLAY_ROW_ACTION, "");

      status = sl_bt_sm_increase_security(client->conn_handle);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to start bonding\n");
      else
        LOG_INFO("Succeeded to start bonding\n");

      update_lcd();
  }
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
  sl_status_t status;

  ble_client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      LOG_INFO("Disconnected\n");

      client->conn_state = CONN_STATE_SCANNING;
      client->conn_handle = 0;

      status = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to start scanning\n");
      else
        LOG_INFO("Succeeded to start scanning\n");

      update_lcd();
  }
}

void handle_bt_external_signals(sl_bt_msg_t *evt) {
  //sl_status_t sl_status;

  if((evt->data.evt_system_external_signal.extsignals == evtB1_Pressed))  {
      LOG_INFO("Pressed 1");
  }

  if((evt->data.evt_system_external_signal.extsignals == evtB2_Pressed))  {
      LOG_INFO("Pressed 2");
  }

  if((evt->data.evt_system_external_signal.extsignals == evtB3_Pressed))  {
      LOG_INFO("Pressed 3");
  }

  if((evt->data.evt_system_external_signal.extsignals == evtB4_Pressed))  {
      LOG_INFO("Pressed 4");
  }
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
    case sl_bt_evt_scanner_scan_report_id:
      handle_bt_scanned(evt);
      break;
    case sl_bt_evt_connection_opened_id:
      handle_bt_opened(evt);
      break;
    case sl_bt_evt_connection_closed_id:
      handle_bt_closed(evt);
      break;
    case sl_bt_evt_system_soft_timer_id:
      displayUpdate(evt);
      break;
    case sl_bt_evt_system_external_signal_id:
      handle_bt_external_signals(evt);
      break;
    case sl_bt_evt_sm_bonded_id:
      handle_bt_bonded(evt);
      break;
    case sl_bt_evt_sm_bonding_failed_id:
      handle_bt_bonding_failed(evt);
      break;
    case sl_bt_evt_sm_confirm_bonding_id:
      handle_bt_confirm_bonding(evt);
      break;
    case sl_bt_evt_sm_confirm_passkey_id:
      handle_bt_confirm_passkey(evt);
      break;
  } // end - switch
} // handle_ble_event()

ble_server_data_t* get_ble_server_data()
{
  return &ble_server_data;
}
