/*******************************************************************************
 * @file  ble.c
 * @brief Function definitions for handling BT events
 *******************************************************************************
 * Editor: Oct 18, 2022, Amey More
 ******************************************************************************/

#include "ble.h"
#include "gatt_db.h"
#include "lcd.h"
#include "ble_device_type.h"
#include "sl_bt_api.h"
#include "em_gpio.h"
#include "gpio.h"

#define INCLUDE_LOG_DEBUG (1)
#include "log.h"

#define ADVERTISING_MIN         (250*1.6)
#define ADVERTISING_MAX         (250*1.6)
#define ADVERTISING_DURATION    (0)
#define ADVERTISING_MAXEVENTS   (0)

#define CONNECTION_MIN_INTERVAL   (75/1.25)
#define CONNECTION_MAX_INTERVAL   (75/1.25)
#define CONNECTION_LATENCY        (300/75)
#define CONNECTION_TIMEOUT        (((1 + (300/75)) * (75/1.25) * 2)+(75/1.25))
#define CONNECTION_MIN_CE         (0)
#define CONNECTION_MAX_CE         (4)

sl_status_t sl_status;
uint8_t gattCount=0;

ble_client_data_t ble_client_data;

// BLE Structure pointer function
ble_client_data_t *getbleData() {
  return &ble_client_data;
}   //    *getbleData

void handle_bt_boot() {
  displayInit();
  displayPrintf(DISPLAY_ROW_NAME, "%s",(DEVICE_IS_HEATER ? "HEATER" : "AC"));
  displayPrintf(DISPLAY_ROW_ASSIGNMENT, "Course Project");

  // Extract unique ID from BT Address.
  sl_bt_system_get_identity_address(&ble_client_data.myAddress, &ble_client_data.myAddress_type);

  displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x",ble_client_data.myAddress.addr[0],
                                                        ble_client_data.myAddress.addr[1],
                                                        ble_client_data.myAddress.addr[2],
                                                        ble_client_data.myAddress.addr[3],
                                                        ble_client_data.myAddress.addr[4],
                                                        ble_client_data.myAddress.addr[5]
                );

  sl_status = sl_bt_advertiser_create_set(&ble_client_data.advertisingHandle);
  if(sl_status != SL_STATUS_OK) {
      LOG_ERROR("Advertiser Create Set Error 0x%x",sl_status);
  }

  sl_status = sl_bt_advertiser_set_timing(ble_client_data.advertisingHandle,
                                          ADVERTISING_MIN,
                                          ADVERTISING_MAX,
                                          ADVERTISING_DURATION,
                                          ADVERTISING_MAXEVENTS
                                         );
  if(sl_status != SL_STATUS_OK) {
      LOG_ERROR("Advertiser Set Timing Error 0x%x",sl_status);
  }

  sl_status = sl_bt_sm_delete_bondings();
  if(sl_status != SL_STATUS_OK) {
      LOG_ERROR("Delete Bondings Error 0x%x",sl_status);
  }

  sl_status = sl_bt_sm_configure(0x0f, sm_io_capability_displayyesno);
  if(sl_status != SL_STATUS_OK) {
      LOG_ERROR("SM Configure Error 0x%x",sl_status);
  }

  sl_status = sl_bt_advertiser_start(ble_client_data.advertisingHandle,
                                       sl_bt_advertiser_general_discoverable,
                                       sl_bt_advertiser_connectable_scannable
                                      );
  if(sl_status == SL_STATUS_OK) {
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
  }
  else {
      LOG_ERROR("Advertiser Start Error 0x%x",sl_status);
  }
}

void handle_bt_open(sl_bt_msg_t *evt) {
  ble_client_data.connectionHandle = evt->data.evt_connection_opened.connection;

  sl_bt_advertiser_stop(ble_client_data.advertisingHandle);
  sl_status =  sl_bt_connection_set_parameters ( ble_client_data.connectionHandle,
                                                 CONNECTION_MIN_INTERVAL,
                                                 CONNECTION_MAX_INTERVAL,
                                                 CONNECTION_LATENCY,
                                                 CONNECTION_TIMEOUT,
                                                 CONNECTION_MIN_CE,
                                                 CONNECTION_MAX_CE
                                                );

  if(sl_status == SL_STATUS_OK) {
      displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
      ble_client_data.connectionFlag = true;
  }
  else {
      LOG_ERROR("Error 0x%x",sl_status);
  }
}

void handle_bt_confirm_bonding()  {
  sl_status = sl_bt_sm_bonding_confirm(ble_client_data.connectionHandle, 1);
  if(sl_status != SL_STATUS_OK) {
      LOG_ERROR("SM Bonding Confirm Error 0x%x",sl_status);
  }
}

void handle_bt_confirm_paskey(sl_bt_msg_t *evt) {
  displayPrintf(DISPLAY_ROW_PASSKEY, "Passkey %d", evt->data.evt_sm_confirm_passkey.passkey);
  displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
}

void handle_bt_external_signals(sl_bt_msg_t *evt) {
  if((evt->data.evt_system_external_signal.extsignals == evtPB0_Pressed))  {
    LOG_INFO("Pressed");
    sl_status = sl_bt_sm_passkey_confirm(ble_client_data.connectionHandle, 1);
    if(sl_status != SL_STATUS_OK) {
        LOG_ERROR("SM Passkey Confirm Error 0x%x",sl_status);
    }
  }
}

void handle_bt_bonded() {
  displayPrintf(DISPLAY_ROW_PASSKEY, " ");
  displayPrintf(DISPLAY_ROW_ACTION, " ");
  displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");

  sl_status = sl_bt_gatt_discover_primary_services_by_uuid(ble_client_data.connectionHandle,
                                                           sizeof(thermo_service),
                                                           (const uint8_t*)thermo_service
                                                          );
  if(sl_status != SL_STATUS_OK) {
      LOG_ERROR("Primary Services Discovery Error 0x%04x",sl_status);
  }
}

void handle_bt_gatt_complete()  {
  if(gattCount == 0)  {
      sl_status = sl_bt_gatt_discover_characteristics_by_uuid(ble_client_data.connectionHandle,
                                                              ble_client_data.HTMServiceHandle,
                                                              sizeof(thermo_char),
                                                              (const uint8_t*)thermo_char
                                                             );
      if(sl_status != SL_STATUS_OK) {
          LOG_ERROR("Button Discover Characteristics Error 0x%04x",sl_status);
      }
      else {
          gattCount = 1;
      }
  }

  if(gattCount == 1)  {
      sl_status = sl_bt_gatt_set_characteristic_notification(ble_client_data.connectionHandle,
                                                             ble_client_data.HTMCharacteristicsHandle,
                                                             sl_bt_gatt_indication
                                                            );
      if(sl_status == SL_STATUS_OK) {
          displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
      }
      else  {
          LOG_ERROR("Temperature Characteristics Notification Error 0x%04x",sl_status);
      }
      gattCount = 2;
  }
}

void handle_bt_close()  {
  gattCount = 0;
  sl_status = sl_bt_advertiser_start(ble_client_data.advertisingHandle,
                                           sl_bt_advertiser_general_discoverable,
                                           sl_bt_advertiser_connectable_scannable
                                          );

  if(sl_status == SL_STATUS_OK) {
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
  }
  else {
      LOG_ERROR("Advertiser Start Error 0x%x",sl_status);
  }

  sl_status = sl_bt_sm_delete_bondings();
  if(sl_status != SL_STATUS_OK) {
      LOG_ERROR("Delete Bondings Error 0x%x",sl_status);
  }
}

// To handle ble events
void handle_ble_event(sl_bt_msg_t *evt) {

  switch (SL_BT_MSG_ID(evt->header)) {

    case sl_bt_evt_system_boot_id:
      LOG_INFO("Boot");
      handle_bt_boot();
      break;

    case sl_bt_evt_connection_opened_id:
      LOG_INFO("Connected");
      handle_bt_open(evt);
      break;

    case sl_bt_evt_sm_confirm_bonding_id:
      LOG_INFO("Confirm Bonding");
      handle_bt_confirm_bonding();
      break;

    case sl_bt_evt_sm_confirm_passkey_id:
      LOG_INFO("Confirm Passkey");
      handle_bt_confirm_paskey(evt);
      break;

    case sl_bt_evt_system_external_signal_id:
      handle_bt_external_signals(evt);
      break;

    case sl_bt_evt_sm_bonded_id:
      LOG_INFO("Bonded");
      handle_bt_bonded();
      break;

    case sl_bt_evt_sm_bonding_failed_id:
      LOG_ERROR("Bonding Failed");
      break;

    case sl_bt_evt_gatt_procedure_completed_id:
      LOG_INFO("Gatt Complete");
      handle_bt_gatt_complete();
      break;

    case sl_bt_evt_connection_closed_id:
      LOG_INFO("Closed");
      handle_bt_close();
      break;
  }
}   //    handle_ble_event
