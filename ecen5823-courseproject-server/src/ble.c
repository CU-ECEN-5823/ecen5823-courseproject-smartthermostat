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


ble_server_data_t ble_server_data = {
    .current_temp = 0,
    .target_temp = 0,
    .offset_temp = 0,
    .session_scans_count = 0,
    .automatic_temp_control = 1
};


const uint8_t clients_addr[CLIENTS_NUM][6] = {
    {0x3f, 0x4a, 0xa6, 0x14, 0x2e, 0x84},   // AC
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

ble_client_data_t* get_client_by_conn_state(uint32_t conn_state)
{
  for (int i = 0; i < CLIENTS_NUM; i++) {
      if (conn_state & (uint32_t)ble_server_data.ble_clients[i].conn_state) {
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
  char dis_string[20];

  for (uint8_t i = 0; i < CLIENTS_NUM; i++) {
      if (i == 0)
        strcpy(dis_string, "AC:");
      else
        strcpy(dis_string, "Heater:");

      switch (ble_server_data.ble_clients[i].conn_state)
      {
        case CONN_STATE_UNKNOWN:
          strcat(dis_string, "UNKNOWN");
          break;
        case CONN_STATE_SCANNING:
          strcat(dis_string, "SCANNING");
          break;
        case CONN_STATE_CONNECTING:
          strcat(dis_string, "CONNECTING");
          break;
        case CONN_STATE_CONNECTED:
          strcat(dis_string, "CONNECTED");
          break;
        case CONN_STATE_BONDING:
        case CONN_STATE_PASSKEY:
          strcat(dis_string, "BONDING");
          break;
        case CONN_STATE_BONDED:
          if (ble_server_data.ble_clients[i].onoff_state)
            strcat(dis_string, "ON");
          else
            strcat(dis_string, "OFF");
          break;
        case CONN_STATE_NOT_BONDED:
          strcat(dis_string, "NOT BOND");
          break;
        case CONN_STATE_DISCONNECTED:
          strcat(dis_string, "DISCONNECTED");
          break;
        case CONN_STATE_NOT_FOUND:
          strcat(dis_string, "NOT FOUND");
          break;
      }

      if (i == 0)
        displayPrintf( DISPLAY_ROW_BTADDR2, dis_string);
      else
        displayPrintf(DISPLAY_ROW_CLIENTADDR, dis_string);
  }

  if (get_client_by_conn_state(CONN_STATE_PASSKEY) == NULL) {
      displayPrintf(DISPLAY_ROW_PASSKEY, "");
      displayPrintf(DISPLAY_ROW_ACTION, "");
  }

  displayPrintf(DISPLAY_ROW_NAME, "Smart Thermostat");
  displayPrintf(DISPLAY_ROW_8, "Curr Temp : %dF", ble_server_data.current_temp);
  displayPrintf(DISPLAY_ROW_9, "Target Temp : %dF", ble_server_data.target_temp);
  displayPrintf(DISPLAY_ROW_ASSIGNMENT, "Course Project");

  if (ble_server_data.automatic_temp_control)
    displayPrintf(DISPLAY_ROW_11, "Auto On");
  else
    displayPrintf(DISPLAY_ROW_11, "Auto Off");
}

void set_ac_on()
{
  if ((ble_server_data.ble_clients[0].conn_state == CONN_STATE_BONDED) &&
      (ble_server_data.ble_clients[0].onoff_state == 0)) {
      ble_server_data.ble_clients[0].onoff_state = 1;
      LOG_INFO("TURNING ON AC\n");
      // send indication
  }
  update_lcd();
}

void set_ac_off()
{
  if ((ble_server_data.ble_clients[0].conn_state == CONN_STATE_BONDED) &&
      (ble_server_data.ble_clients[0].onoff_state == 1)) {
      ble_server_data.ble_clients[0].onoff_state = 0;
      LOG_INFO("TURNING OFF AC\n");
      // send indication
  }
  update_lcd();
}

void set_heater_on()
{
  if ((ble_server_data.ble_clients[1].conn_state == CONN_STATE_BONDED) &&
      (ble_server_data.ble_clients[1].onoff_state == 0)) {
      ble_server_data.ble_clients[1].onoff_state = 1;
      LOG_INFO("TURNING ON HEATER\n");
      // send indication
  }
  update_lcd();
}

void set_heater_off()
{
  if ((ble_server_data.ble_clients[1].conn_state == CONN_STATE_BONDED) &&
      (ble_server_data.ble_clients[1].onoff_state == 1)) {
      ble_server_data.ble_clients[1].onoff_state = 0;
      LOG_INFO("TURNING OFF HEATER\n");
      // send indication
  }
  update_lcd();
}

void toggle_ac()
{
  ble_server_data.automatic_temp_control = 0;

  if (ble_server_data.ble_clients[0].onoff_state)
    set_ac_off();
  else
    set_ac_on();
}

void toggle_heater()
{
  ble_server_data.automatic_temp_control = 0;

  if (ble_server_data.ble_clients[1].onoff_state)
    set_heater_off();
  else
    set_heater_on();
}

void toggle_auto_feature()
{
  ble_server_data.automatic_temp_control = !ble_server_data.automatic_temp_control;

  if (ble_server_data.automatic_temp_control)
    update_current_temperature(ble_server_data.current_temp);

  update_lcd();
}

void update_current_temperature(int16_t temp)
{
  if (temp > 0 && temp <= 125) {
      LOG_INFO("Current temperature = %d", temp);

      ble_server_data.current_temp = temp;

      if (ble_server_data.target_temp == 0)
        ble_server_data.target_temp = temp;

      if (ble_server_data.automatic_temp_control) {
          if (ble_server_data.current_temp > (ble_server_data.target_temp + ble_server_data.offset_temp)) {
              set_ac_on();
              set_heater_off();
          }
          else if (ble_server_data.current_temp < (ble_server_data.target_temp - ble_server_data.offset_temp)) {
              set_ac_off();
              set_heater_on();
          }
          else {
              set_ac_off();
              set_heater_off();
          }
      }

      update_lcd();
  }
  else {
      LOG_ERROR("Invalid temperature range!");
  }
}

void increase_taget_temperature()
{
  if (ble_server_data.target_temp < 125)
    ble_server_data.target_temp++;
  else
    ble_server_data.target_temp = 125;

  ble_server_data.automatic_temp_control = 1;
  update_current_temperature(ble_server_data.current_temp);

  update_lcd();
}

void decrease_taget_temperature()
{
  if (ble_server_data.target_temp > 1)
    ble_server_data.target_temp--;
  else
    ble_server_data.target_temp = 1;

  ble_server_data.automatic_temp_control = 1;
  update_current_temperature(ble_server_data.current_temp);

  update_lcd();
}

void start_bt_scan()
{
  sl_status_t status;
  ble_client_data_t *client;

  // If one of the clients is found and in process of connection/bonding then don't scan until process is complete
  client = get_client_by_conn_state(CONN_STATE_CONNECTING | CONN_STATE_CONNECTED | CONN_STATE_BONDING);

  if (client != NULL)
    return;

  if (ble_server_data.session_scans_count == MAX_SESSION_SCANS) {
      // Making sure this is triggered only once
      ble_server_data.session_scans_count = MAX_SESSION_SCANS;
      while(get_client_by_conn_state(CONN_STATE_SCANNING) != NULL){
          get_client_by_conn_state(CONN_STATE_SCANNING)->conn_state = CONN_STATE_NOT_FOUND;
      }
      update_lcd();
      return;
  }

  client = get_client_by_conn_state(CONN_STATE_SCANNING | CONN_STATE_NOT_FOUND);

  if (client != NULL) {
      client->conn_state = CONN_STATE_SCANNING;
      status = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to start scanning\n");

      update_lcd();
  }

  ble_server_data.session_scans_count++;
}

void start_manual_scan()
{
  while (get_client_by_conn_state(CONN_STATE_NOT_FOUND) != NULL) {
      get_client_by_conn_state(CONN_STATE_NOT_FOUND)->conn_state = CONN_STATE_SCANNING;
  }

  ble_server_data.session_scans_count = 0;

  start_bt_scan();
}

void pb0_event_handle()
{
  sl_status_t status;
  ble_client_data_t *client = get_client_by_conn_state(CONN_STATE_PASSKEY);

  if (client != NULL) {
      status = sl_bt_sm_passkey_confirm(client->conn_handle, 1);
      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to confirm passkey\n");
      else
        LOG_INFO("Succeeded to confirm passkey\n");
  }
  else {
      start_manual_scan();
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

  status = sl_bt_sm_delete_bondings();
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to delete bonding");
  else
    LOG_INFO("Succeeded to delete bonding");

  for (int i = 0; i < CLIENTS_NUM; i++)
    ble_server_data.ble_clients[i].conn_state = CONN_STATE_SCANNING;

  start_bt_scan();

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
    LOG_ERROR("Failed to stop scanning :: %u\n", status);

  ble_client_data_t *client = get_client_by_addr(evt->data.evt_scanner_scan_report.address);

  if (client != NULL && client->conn_state != CONN_STATE_DISCONNECTED) {
      client->conn_state = CONN_STATE_CONNECTING;

      status = sl_bt_connection_open(client->addr, \
                                     sl_bt_gap_public_address, \
                                     sl_bt_gap_1m_phy, \
                                     &client->conn_handle);

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to start connection\n");
      else
        LOG_INFO("Succeeded to start connection\n");
  }
  else {
      start_bt_scan();
  }

  update_lcd();
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

  LOG_INFO("handle_bt_opened\n");

  ble_client_data_t *client = get_client_by_addr(evt->data.evt_connection_opened.address);

  if (client != NULL) {
      LOG_INFO("Connected\n");

      client->conn_handle = evt->data.evt_connection_opened.connection;
      client->bond_handle = evt->data.evt_connection_opened.bonding;
      client->conn_state = CONN_STATE_CONNECTED;

      if (client->bond_handle == SL_BT_INVALID_BONDING_HANDLE || client->bond_handle == 0x00)
        status = sl_bt_sm_increase_security(client->conn_handle);
      else
        LOG_INFO("Already Bonded :: %x", client->bond_handle); // NEED TO HANDLE WHEN ALREADY BONDED,
      //if handled then remove delete_bondings when disconnected

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
  ble_client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      client->conn_state = CONN_STATE_PASSKEY;
      displayPrintf(DISPLAY_ROW_PASSKEY, "AC Passkey %u", evt->data.evt_sm_confirm_passkey.passkey);
      displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");

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
      client->bond_handle = evt->data.evt_sm_confirm_bonding.bonding_handle;
  }

  start_bt_scan();
  update_lcd();
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
  ble_client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      client->conn_state = CONN_STATE_NOT_BONDED;
      LOG_INFO("Bonding Failed:: reason :: %u", evt->data.evt_sm_bonding_failed.reason);
  }

  start_bt_scan();
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
  ble_client_data_t *client = get_client_by_conn_handle(evt->data.evt_connection_closed.connection);

  if (client != NULL) {
      LOG_INFO("Disconnected\n");

      sl_status_t status;

      status = sl_bt_sm_delete_bonding(client->bond_handle);
      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to delete bonding");
      else
        LOG_INFO("Succeeded to delete bonding");



      client->conn_handle = 0x00;
      client->bond_handle = 0x00;

      // If connection closed by client but not explicitly by server
      if (client->conn_state != CONN_STATE_DISCONNECTED) {
          client->conn_state = CONN_STATE_SCANNING;
          start_manual_scan();
      }
  }

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
    case sl_bt_evt_scanner_scan_report_id:
      handle_bt_scanned(evt);
      break;
    case sl_bt_evt_connection_opened_id:
      handle_bt_opened(evt);
      break;
    case sl_bt_evt_connection_closed_id:
      handle_bt_closed(evt);
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
    case sl_bt_evt_system_soft_timer_id:
      displayUpdate(evt);
      break;
  } // end - switch
} // handle_ble_event()

ble_server_data_t* get_ble_server_data()
{
  return &ble_server_data;
}
