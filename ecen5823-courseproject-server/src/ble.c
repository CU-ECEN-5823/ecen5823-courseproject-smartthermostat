/*******************************************************************************
 * @file    ble.c
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
#include "ble.h"
#include "lcd.h"
#include "scheduler.h"
#include "../autogen/gatt_db.h"

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


client_data_t g_client_data[] = {
    {
        {.addr = {0x49, 0x4a, 0xa6, 0x14, 0x2e, 0x84}},
        .client_type = CLIENT_TYPE_AC,
        .conn_state = CONN_STATE_UNKNOWN,
        .onoff_state = CLIENT_STATE_OFF,
        .conn_handle = 0,
        .bond_handle = 0,
        .indications_enabled = 0,
        .gatt_ack_pending = 0
    },
    {
        {.addr = {0x3f, 0x4a, 0xa6, 0x14, 0x2e, 0x84}},
        .client_type = CLIENT_TYPE_HEATER,
        .conn_state = CONN_STATE_UNKNOWN,
        .onoff_state = CLIENT_STATE_OFF,
        .conn_handle = 0,
        .bond_handle = 0,
        .indications_enabled = 0,
        .gatt_ack_pending = 0
    }
};

server_data_t g_server_data = {
    .current_temp = 0,
    .target_temp = 0,
    .offset_temp = 0,
    .session_scans_count = 0,
    .automatic_temp_control = 1,
    .clients_data = g_client_data,
    .clients_count = sizeof(g_client_data) / sizeof(client_data_t)
};


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Initializes the LCD display and sets the BT soft timer.
 ******************************************************************************/
void ble_init()
{
  displayInit();
}


/******************************************************************************
 * @brief   Searches for client with matching address value.
 *
 * @param
 *  addr    Client address to be searched.
 *
 * @return
 *  Returns NULL if the client with matching address is not found else returns
 *  the pointer to the client.
 *
 ******************************************************************************/
client_data_t* get_client_by_addr(bd_addr addr)
{
  for (int i = 0; i < g_server_data.clients_count; i++) {
      if (!memcmp(addr.addr, g_server_data.clients_data[i].addr.addr, 6)) {
          return &g_server_data.clients_data[i];
      }
  }
  return NULL;
}


/******************************************************************************
 * @brief   Searches for client with matching connection handle value.
 *
 * @param
 *  conn_handle    Connection handle value to be searched.
 *
 * @return
 *  Returns NULL if the client with matching connection handle is not found else
 *  returns the pointer to the client.
 *
 ******************************************************************************/
client_data_t* get_client_by_conn_handle(uint8_t conn_handle)
{
  for (int i = 0; i < g_server_data.clients_count; i++) {
      if (conn_handle == g_server_data.clients_data[i].conn_handle) {
          return &g_server_data.clients_data[i];
      }
  }
  return NULL;
}


/******************************************************************************
 * @brief   Searches for client with matching connection state.
 *
 * @param
 *  conn_handle    Client with given connection state to be searched.
 *
 * @return
 *  Returns NULL if the client with matching connection state is not found else
 *  returns the pointer to the client.
 *
 ******************************************************************************/
client_data_t* get_client_by_conn_state(uint32_t conn_state)
{
  for (int i = 0; i < g_server_data.clients_count; i++) {
      if (conn_state & (uint32_t)g_server_data.clients_data[i].conn_state) {
          return &g_server_data.clients_data[i];
      }
  }
  return NULL;
}


/******************************************************************************
 * @brief   Searches for client with matching client type it can be wither
 * AC/Heater.
 *
 * @param
 *  client_type    Client with given type to be searched.
 *
 * @return
 *  Returns NULL if the client with matching type is not found else returns the
 *  pointer to the client.
 *
 ******************************************************************************/
client_data_t* get_client_by_type(client_type_t client_type)
{
  for (int i = 0; i < g_server_data.clients_count; i++) {
      if (client_type == g_server_data.clients_data[i].client_type) {
          return &g_server_data.clients_data[i];
      }
  }
  return NULL;
}

/******************************************************************************
 * @brief   Updates the server and client info on the LCD.
 ******************************************************************************/
void update_lcd(void)
{
  for (uint8_t i = 0; i < g_server_data.clients_count; i++) {
      char display_str[20];
      uint32_t display_row;

      if (g_server_data.clients_data[i].client_type == CLIENT_TYPE_AC) {
          strcpy (display_str, "AC: ");
          display_row = DISPLAY_ROW_BTADDR2;
      }
      else if (g_server_data.clients_data[i].client_type == CLIENT_TYPE_HEATER) {
          strcpy (display_str, "Heater: ");
          display_row = DISPLAY_ROW_CLIENTADDR;
      }

      switch (g_server_data.clients_data[i].conn_state)
      {
        case CONN_STATE_UNKNOWN:
          strcat(display_str, "UNKNOWN");
          break;
        case CONN_STATE_SCANNING:
          strcat(display_str, "SCANNING");
          break;
        case CONN_STATE_CONNECTING:
          strcat(display_str, "CONNECTING");
          break;
        case CONN_STATE_CONNECTED:
          strcat(display_str, "CONNECTED");
          break;
        case CONN_STATE_BONDING:
        case CONN_STATE_PASSKEY:
          strcat(display_str, "BONDING");
          break;
        case CONN_STATE_BONDED:
          if (g_server_data.clients_data[i].onoff_state == CLIENT_STATE_ON)
            strcat(display_str, "ON");
          else
            strcat(display_str, "OFF");
          break;
        case CONN_STATE_NOT_BONDED:
          strcat(display_str, "NOT BOND");
          break;
        case CONN_STATE_DISCONNECTED:
          strcat(display_str, "DISCONNECTED");
          break;
        case CONN_STATE_NOT_FOUND:
          strcat(display_str, "NOT FOUND");
          break;
      }

      displayPrintf( display_row, display_str);
  }

  if (get_client_by_conn_state(CONN_STATE_PASSKEY) == NULL) {
      displayPrintf(DISPLAY_ROW_PASSKEY, "");
      displayPrintf(DISPLAY_ROW_ACTION, "");
  }

  displayPrintf(DISPLAY_ROW_NAME, "Smart Thermostat");
  displayPrintf(DISPLAY_ROW_8, "Curr Temp : %dF", g_server_data.current_temp);
  displayPrintf(DISPLAY_ROW_ASSIGNMENT, "Course Project");

  if (g_server_data.automatic_temp_control) {
      displayPrintf(DISPLAY_ROW_11, "Auto On");
      displayPrintf(DISPLAY_ROW_9, "Target Temp : %dF", g_server_data.target_temp);
  }
  else {
      displayPrintf(DISPLAY_ROW_11, "Auto Off");
      displayPrintf(DISPLAY_ROW_9, "");
  }
}


/******************************************************************************
 * @brief   Turns On/Off a client of type AC/Heater, sends respective
 * indication to respective client and finally updates the same data on the
 * LCD.
 *
 * @param
 *  client_type   The client device type to be controlled AC/Heater.
 *  onoff_state   State to which client device to be set On/Off.
 *
 ******************************************************************************/
void set_client_state(client_type_t client_type, client_state_t onoff_state)
{
  client_data_t *client = get_client_by_type(client_type);

  if (client == NULL)
    return;

  if (client->conn_state == CONN_STATE_BONDED && client->onoff_state != onoff_state) {
      LOG_INFO("TURNED ON/OFF THE CLIENT\n");
      client->onoff_state = onoff_state;
      // send indication
      sl_status_t status = sl_bt_gatt_server_send_indication(
          client->conn_handle,
          gattdb_ac_state,
          1,
          &client->onoff_state
      );

      if (status != SL_STATUS_OK)
        LOG_ERROR("Failed to send indication %u\n", status);
      else
        LOG_INFO("Successfully sent indication\n");

      update_lcd();
  }
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Toggles the state of the connected client On/Off state.
 ******************************************************************************/
void toggle_client_state(client_type_t client_type)
{
  g_server_data.automatic_temp_control = 0;

  client_data_t *client = get_client_by_type(client_type);

  if (client == NULL)
    return;

  if (client->onoff_state == CLIENT_STATE_ON)
    set_client_state(client_type, CLIENT_STATE_OFF);
  else
    set_client_state(client_type, CLIENT_STATE_ON);
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Toggles the auto On/Off feature.
 ******************************************************************************/
void toggle_auto_feature()
{
  g_server_data.automatic_temp_control = !g_server_data.automatic_temp_control;

  if (g_server_data.automatic_temp_control)
    update_current_temperature(g_server_data.current_temp);

  update_lcd();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Updates the current temperature and displays the same on the LCD.
 ******************************************************************************/
void update_current_temperature(int16_t temp)
{
  if (temp > 0 && temp <= 125) {
      LOG_INFO("Current temperature = %d", temp);

      g_server_data.current_temp = temp;

      if (g_server_data.target_temp == 0)
        g_server_data.target_temp = temp;

      if (g_server_data.automatic_temp_control) {
          if (g_server_data.current_temp > (g_server_data.target_temp + g_server_data.offset_temp)) {
              set_client_state(CLIENT_TYPE_AC, CLIENT_STATE_ON);
              set_client_state(CLIENT_TYPE_HEATER, CLIENT_STATE_OFF);
          }
          else if (g_server_data.current_temp < (g_server_data.target_temp - g_server_data.offset_temp)) {
              set_client_state(CLIENT_TYPE_AC, CLIENT_STATE_OFF);
              set_client_state(CLIENT_TYPE_HEATER, CLIENT_STATE_ON);
          }
          else {
              set_client_state(CLIENT_TYPE_AC, CLIENT_STATE_OFF);
              set_client_state(CLIENT_TYPE_HEATER, CLIENT_STATE_OFF);
          }
      }

      update_lcd();
  }
  else {
      LOG_ERROR("Invalid temperature range!");
  }
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Increases the value of the target temperature value.
 ******************************************************************************/
void increase_taget_temperature()
{
  if (!g_server_data.automatic_temp_control)
    return;

  if (g_server_data.target_temp < 125)
    g_server_data.target_temp++;
  else
    g_server_data.target_temp = 125;

  update_current_temperature(g_server_data.current_temp);

  update_lcd();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Decreases the value of the target temperature value.
 ******************************************************************************/
void decrease_taget_temperature()
{
  if (!g_server_data.automatic_temp_control)
    return;

  if (g_server_data.target_temp > 1)
    g_server_data.target_temp--;
  else
    g_server_data.target_temp = 1;

  update_current_temperature(g_server_data.current_temp);

  update_lcd();
}


/******************************************************************************
 * @brief   Initiates the BT scanning. Does number of checks to verify if the
 * scan can be initiated (when one of the client is bonding) or if it is
 * necessary to initiate the scanning( when all the clients are connected).
 *
 ******************************************************************************/
void start_bt_scan(void)
{
  sl_status_t status;
  client_data_t *client;

  /* If one of the clients is in the process of connection/bonding then don't
   * scan until bonding is complete */
  client = get_client_by_conn_state(CONN_STATE_CONNECTING | CONN_STATE_CONNECTED
                                    | CONN_STATE_BONDING);

  if (client != NULL)
    return;

  /* If the clients are not found with the number of scans limit then set the
   * client status as NOT FOUND and block any further scanning unless scanning
   * is manually triggered by the user */
  if (g_server_data.session_scans_count == MAX_SESSION_SCANS) {
      g_server_data.session_scans_count = MAX_SESSION_SCANS;
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
      g_server_data.session_scans_count++;
  }
  else {
      g_server_data.session_scans_count = MAX_SESSION_SCANS;
  }
}


/******************************************************************************
 * @brief   Initiates the force scanning for clients even when the number of
 * scans has reached the limit.
 *
 ******************************************************************************/
void start_manual_scan(void)
{
  while (get_client_by_conn_state(CONN_STATE_NOT_FOUND) != NULL) {
      get_client_by_conn_state(CONN_STATE_NOT_FOUND)->conn_state = CONN_STATE_SCANNING;
  }

  g_server_data.session_scans_count = 0;

  start_bt_scan();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Handles PB0 event based on context.
 ******************************************************************************/
void pb0_event_handle(void)
{
  sl_status_t status;
  client_data_t *client = get_client_by_conn_state(CONN_STATE_PASSKEY);

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
 * @brief   Handles client boot event
 ******************************************************************************/
void handle_bt_boot(void)
{
  sl_status_t status;

  status = sl_bt_system_get_identity_address(&g_server_data.addr, &g_server_data.addr_type);
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to get the BT address");

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

  for (int i = 0; i < g_server_data.clients_count; i++)
    g_server_data.clients_data[i].conn_state = CONN_STATE_SCANNING;

  //  status = sl_bt_advertiser_create_set(&g_server_data.adv_handle);
  //  status = sl_bt_advertiser_set_timing(g_server_data.adv_handle, 400, 400, 0, 0);
  //  status = sl_bt_advertiser_start(g_server_data.adv_handle,
  //                                  sl_bt_advertiser_general_discoverable,
  //                                  sl_bt_advertiser_connectable_scannable);

  status = sl_bt_scanner_set_mode(sl_bt_gap_1m_phy, PASSIVE_SCANNING);
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to set scanner mode");

  status = sl_bt_scanner_set_timing(sl_bt_gap_1m_phy, SCAN_INTERVAL, SCAN_WINDOW);
  if (status != SL_STATUS_OK)
    LOG_ERROR("Failed to set scanner timing");
  start_bt_scan();

  update_lcd();
}


/******************************************************************************
 * @brief   Handles client scanned event
 *
 * @param
 *  *evt    Data structure of BT API message
 *
 ******************************************************************************/
void handle_bt_scanned(sl_bt_msg_t *evt)
{
  sl_status_t scan_status;

  scan_status = sl_bt_scanner_stop();

  client_data_t *client = get_client_by_addr(evt->data.evt_scanner_scan_report.address);

  if (client != NULL && client->conn_state != CONN_STATE_DISCONNECTED) {
      sl_status_t status;

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
      if (scan_status != SL_STATUS_OK) {
          LOG_ERROR("Failed to stop scanning :: %u\n", scan_status);
          return;
      }
      else {
          start_bt_scan();
      }
  }

  update_lcd();
}


/******************************************************************************
 * @brief   Handles client connection opened event
 *
 * @param
 *  *evt    Data structure of BT API message
 *
 ******************************************************************************/
void handle_bt_opened(sl_bt_msg_t *evt)
{
  sl_status_t status;

  LOG_INFO("handle_bt_opened\n");

  client_data_t *client = get_client_by_addr(evt->data.evt_connection_opened.address);

  //  client_data_t *client = get_client_by_type(CLIENT_TYPE_AC);

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
 * @brief   Handles client connection opened event
 *
 * @param
 *  *evt    Data structure of BT API message
 *
 ******************************************************************************/
void handle_bt_confirm_bonding(sl_bt_msg_t *evt)
{
  sl_status_t status;

  client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

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
 * @brief   Handles client connection opened event
 *
 * @param
 *  *evt    Data structure of BT API message
 *
 ******************************************************************************/
void handle_bt_confirm_passkey(sl_bt_msg_t *evt)
{
  client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      client->conn_state = CONN_STATE_PASSKEY;
      displayPrintf(DISPLAY_ROW_PASSKEY, "AC Passkey %u", evt->data.evt_sm_confirm_passkey.passkey);
      displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");

      update_lcd();
  }
}


/******************************************************************************
 * @brief   Handles client connection opened event
 *
 * @param
 *  *evt    Data structure of BT API message
 *
 ******************************************************************************/
void handle_bt_bonded(sl_bt_msg_t *evt)
{
  client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      client->conn_state = CONN_STATE_BONDED;
      client->bond_handle = evt->data.evt_sm_confirm_bonding.bonding_handle;
  }

  //  start_bt_scan();
  update_lcd();
}


/******************************************************************************
 * @brief   Handles client connection opened event
 *
 * @param
 *  *evt    Data structure of BT API message
 *
 ******************************************************************************/
void handle_bt_bonding_failed(sl_bt_msg_t *evt)
{
  client_data_t *client = get_client_by_conn_handle(evt->data.evt_sm_confirm_bonding.connection);

  if (client != NULL) {
      client->conn_state = CONN_STATE_NOT_BONDED;
      LOG_INFO("Bonding Failed:: reason :: %u", evt->data.evt_sm_bonding_failed.reason);
  }

  start_bt_scan();
  update_lcd();
}

/******************************************************************************
 * @brief Handles GATT characteristic status event
 *
 * @param *evt BT on event evt value
 ******************************************************************************/
void handle_gatt_server_characteristic_status(sl_bt_msg_t *evt)
{
  LOG_INFO("handle_gatt_server_characteristic_status\n");
  client_data_t *client = get_client_by_conn_handle(evt->data.evt_gatt_server_characteristic_status.connection);

  if (client == NULL)
    return;

  client = get_client_by_type(CLIENT_TYPE_HEATER);

  LOG_INFO("handle_gatt_server_characteristic_status\n");

  if (evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0x02) {
      if (client->client_type == CLIENT_TYPE_AC)
        LOG_INFO("AC State Indications Enabled\n");
      else if (client->client_type == CLIENT_TYPE_AC)
        LOG_INFO("AC State Indications Enabled\n");

      client->indications_enabled = 1;
  }
  else if (evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0x00) {
      if (client->client_type == CLIENT_TYPE_AC)
        LOG_INFO("AC State Indications Disabled\n");
      else if (client->client_type == CLIENT_TYPE_AC)
        LOG_INFO("AC State Indications Disabled\n");

      client->indications_enabled = 0;
  }
}


/******************************************************************************
 * @brief   Handles client connection closed event
 *
 * @param
 *  *evt    Data structure of BT API message
 *
 ******************************************************************************/
void handle_bt_closed(sl_bt_msg_t *evt)
{
  client_data_t *client = get_client_by_conn_handle(evt->data.evt_connection_closed.connection);

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
 * SEE HEADER FILE FOR FULL DETAILS
 * Common handler for handling all the BLE events.
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
    case sl_bt_evt_gatt_server_characteristic_status_id:
      handle_gatt_server_characteristic_status(evt);
      break;
    case sl_bt_evt_gatt_server_indication_timeout_id:
      // handle when gatt indication received ack by gatt pending flag
      break;
    case sl_bt_evt_system_soft_timer_id:
      displayUpdate(evt);
      break;
  } // end - switch
} // handle_ble_event()
