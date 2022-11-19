/*******************************************************************************
 * @file        ble.c
 * @brief       Bluetooth Stack
 * @author      Amey More, Amey.More@colorado.edu
 * @date        Oct 24, 2022
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Fall 2022)
 * @instructor  David Sluiter
 *
 * @assignment  ecen5823-assignment9-ameyflash
 * @due         Oct 28, 2022
 *
 * @Resources
 * SOC Thermometer Client from Silicon Labs
 ******************************************************************************/

#include "ble.h"
#include "gatt_db.h"

#define INCLUDE_LOG_DEBUG (1)
#include "log.h"

#include "lcd.h"
#include "ble_device_type.h"
#include "sl_bt_api.h"
#include "em_gpio.h"
#include "gpio.h"

// BLE private data
ble_data_struct_t ble_data = {
    // For Server
    .Advertising_min            =   250*1.6,
    .Advertising_max            =   250*1.6,
    .Advertising_duration       =   0,
    .Advertising_maxevents      =   0,
    .min_interval               =   75/1.25,
    .max_interval               =   75/1.25,
    .latency                    =   300/75, // this is a count
    .timeout                    =   ((1 + (300/75)) * (75/1.25) * 2)+(75/1.25),
    .min_ce_length              =   0,
    .max_ce_length              =   4,

    .connectionFlag             =   false,
    .TemperatureIndicationFlag  =   false,
    .InFlightFlag               =   false,

    .BondingFlag                =   0,
    .ButtonIndicationFlag       =   false,


    // For Client
    .scanInterval               =   50/0.625,       // = 50 ms -> how often to scan
    .scanWindow                 =   25/0.625,       // = 25 ms -> Duration of 1 scan (should be <= Scan Interval)

    .stateConnected             =   false,
    .stateDisconnected          =   false,
    .stateCompleted             =   false,
    .stateCharacteristics       =   false

};

// BLE Structure pointer function
ble_data_struct_t *getbleData() {

  return &ble_data;

}   //    *getbleData

// This function handles all the events
// for Client
void handle_client_events(sl_bt_msg_t *evt) {
  sl_status_t sl_status;
  int i,count=0;

  switch (SL_BT_MSG_ID(evt->header)) {
    // ******************************************************
    // Events common to both Servers and Clients
    // ******************************************************
    // --------------------------------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack API commands before receiving this boot event!
    // Including starting BT stack soft timers!
    // --------------------------------------------------------

    case sl_bt_evt_system_boot_id:

      //LOG_INFO("ble -> Boot ID");
      displayInit();
      displayPrintf(DISPLAY_ROW_NAME, "%s",BLE_DEVICE_TYPE_STRING);
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A9");

      // Extract unique ID from BT Address.
      sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddress_type);

      displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x",ble_data.myAddress.addr[0],
                                                            ble_data.myAddress.addr[1],
                                                            ble_data.myAddress.addr[2],
                                                            ble_data.myAddress.addr[3],
                                                            ble_data.myAddress.addr[4],
                                                            ble_data.myAddress.addr[5]
                    );

      sl_bt_scanner_set_mode(sl_bt_gap_phy_1m , 0);   // 0 - Passive Mode
      sl_bt_scanner_set_timing(sl_bt_gap_phy_1m, ble_data.scanInterval, ble_data.scanWindow);
      sl_status = sl_bt_connection_set_default_parameters(ble_data.min_interval,
                                                          ble_data.max_interval,
                                                          ble_data.latency,
                                                          ble_data.timeout,
                                                          ble_data.min_ce_length,
                                                          ble_data.max_ce_length
                                                         );
      if(sl_status != SL_STATUS_OK)  {
          LOG_ERROR("Connection Set Default Parameters Error 0x%x",sl_status);
      }

      sl_status = sl_bt_scanner_start(sl_bt_gap_phy_1m, sl_bt_scanner_discover_generic);
      if(sl_status == SL_STATUS_OK) {
          displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
      }
      else  {
          LOG_ERROR("Scanner Start Error 0x%x",sl_status);
      }

      sl_status = sl_bt_sm_delete_bondings();
      if(sl_status != SL_STATUS_OK) {
          LOG_ERROR("Delete Bondings Error 0x%x",sl_status);
      }
      sl_status = sl_bt_sm_configure(0x0f, sm_io_capability_displayyesno);
      if(sl_status != SL_STATUS_OK) {
          LOG_ERROR("SM Configure Error 0x%x",sl_status);
      }

      break;

// This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:

      //LOG_INFO("ble -> Connected");
      ble_data.stateConnected = true;

      break;



    case sl_bt_evt_connection_closed_id:

      //LOG_INFO("ble -> Disconnected");
      ble_data.stateDisconnected = true;

      break;



    case sl_bt_evt_connection_parameters_id:

      //LOG_INFO("ble -> Parameters");

      break;

    case sl_bt_evt_system_soft_timer_id:

      displayUpdate();

      break;

    // ******************************************************
    // Events only for Clients
    // ******************************************************
    case sl_bt_evt_scanner_scan_report_id:

      //LOG_INFO("ble -> Scanner scan report id");

      if (evt->data.evt_scanner_scan_report.packet_type == 0) {
          for(i=0;i<6;i++) {
              if(evt->data.evt_scanner_scan_report.address.addr[i] == SERVER_BT_ADDRESS.addr[i]) {
                  count++;
              }
          }
          if(count==6) {
              //LOG_INFO("ble -> Matched");
              sl_bt_scanner_stop();
              sl_status = sl_bt_connection_open(evt->data.evt_scanner_scan_report.address,
                                                sl_bt_gap_public_address,
                                                sl_bt_gap_phy_1m,
                                                NULL
                                               );
              if(sl_status == SL_STATUS_OK) {
                  displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
                  displayPrintf(DISPLAY_ROW_BTADDR2, "%x:%x:%x:%x:%x:%x",SERVER_BT_ADDRESS.addr[0],
                                                                         SERVER_BT_ADDRESS.addr[1],
                                                                         SERVER_BT_ADDRESS.addr[2],
                                                                         SERVER_BT_ADDRESS.addr[3],
                                                                         SERVER_BT_ADDRESS.addr[4],
                                                                         SERVER_BT_ADDRESS.addr[5]
                                );
              }
              else {
                  LOG_ERROR("Connection Open Error 0x%x",sl_status);
              }
          }
      }

      break;

    case sl_bt_evt_gatt_procedure_completed_id:

      //LOG_INFO("ble -> Procedure Completed");
      ble_data.stateCompleted = true;

      //LOG_INFO("Procedure Completed : 0x%04x",evt->data.evt_gatt_procedure_completed.result);
      if(evt->data.evt_gatt_procedure_completed.result == 0x110F) {
          sl_bt_sm_increase_security(ble_data.connectionHandle);
      }

      break;

    case sl_bt_evt_gatt_service_id:

      //LOG_INFO("ble -> Service ID");
      //LOG_INFO("UUID : %x",evt->data.evt_gatt_service.uuid.data[0]);
      if( evt->data.evt_gatt_service.uuid.data[0] == thermo_service[0] && evt->data.evt_gatt_service.uuid.data[1] == thermo_service[1]) {
          ble_data.HTMServiceHandle = evt->data.evt_gatt_service.service;
      }

      if( evt->data.evt_gatt_service.uuid.data[0] == button_service[0] && evt->data.evt_gatt_service.uuid.data[1] == button_service[1] &&
          evt->data.evt_gatt_service.uuid.data[2] == button_service[2] && evt->data.evt_gatt_service.uuid.data[3] == button_service[3] &&
          evt->data.evt_gatt_service.uuid.data[4] == button_service[4] && evt->data.evt_gatt_service.uuid.data[5] == button_service[5] &&
          evt->data.evt_gatt_service.uuid.data[6] == button_service[6] && evt->data.evt_gatt_service.uuid.data[7] == button_service[7] &&
          evt->data.evt_gatt_service.uuid.data[8] == button_service[8] && evt->data.evt_gatt_service.uuid.data[9] == button_service[9] &&
          evt->data.evt_gatt_service.uuid.data[10] == button_service[10] && evt->data.evt_gatt_service.uuid.data[11] == button_service[11] &&
          evt->data.evt_gatt_service.uuid.data[12] == button_service[12] && evt->data.evt_gatt_service.uuid.data[13] == button_service[13] &&
          evt->data.evt_gatt_service.uuid.data[14] == button_service[14] && evt->data.evt_gatt_service.uuid.data[15] == button_service[15]
        ) {
          ble_data.ButtonServiceHandle = evt->data.evt_gatt_service.service;
      }

      //ble_data.HTMServiceHandle = evt->data.evt_gatt_service.service;

      break;

    case sl_bt_evt_gatt_characteristic_id:

      //LOG_INFO("ble -> Char ID");
      if( evt->data.evt_gatt_characteristic.uuid.data[0] == thermo_char[0] && evt->data.evt_gatt_characteristic.uuid.data[1] == thermo_char[1]) {
          ble_data.HTMCharacteristicsHandle = evt->data.evt_gatt_characteristic.characteristic;
      }

      if( evt->data.evt_gatt_characteristic.uuid.data[0] == button_char[0] && evt->data.evt_gatt_characteristic.uuid.data[1] == button_char[1] &&
          evt->data.evt_gatt_characteristic.uuid.data[2] == button_char[2] && evt->data.evt_gatt_characteristic.uuid.data[3] == button_char[3] &&
          evt->data.evt_gatt_characteristic.uuid.data[4] == button_char[4] && evt->data.evt_gatt_characteristic.uuid.data[5] == button_char[5] &&
          evt->data.evt_gatt_characteristic.uuid.data[6] == button_char[6] && evt->data.evt_gatt_characteristic.uuid.data[7] == button_char[7] &&
          evt->data.evt_gatt_characteristic.uuid.data[8] == button_char[8] && evt->data.evt_gatt_characteristic.uuid.data[9] == button_char[9] &&
          evt->data.evt_gatt_characteristic.uuid.data[10] == button_char[10] && evt->data.evt_gatt_characteristic.uuid.data[11] == button_char[11] &&
          evt->data.evt_gatt_characteristic.uuid.data[12] == button_char[12] && evt->data.evt_gatt_characteristic.uuid.data[13] == button_char[13] &&
          evt->data.evt_gatt_characteristic.uuid.data[14] == button_char[14] && evt->data.evt_gatt_characteristic.uuid.data[15] == button_char[15]
        ) {
          ble_data.ButtonCharacteristicsHandle = evt->data.evt_gatt_characteristic.characteristic;
      }

      //ble_data.HTMCharacteristicsHandle = evt->data.evt_gatt_characteristic.characteristic;

      break;

    case sl_bt_evt_gatt_characteristic_value_id:

      //LOG_INFO("ble -> Char Value ID");
      ble_data.stateCharacteristics = true;

      break;

    case sl_bt_evt_system_external_signal_id:
      //LOG_INFO("External Signal Called");
      //displayUpdate();

      if((ble_data.BondingFlag == 1) && (evt->data.evt_system_external_signal.extsignals == evtPB0_Pressed))  {
          //LOG_INFO("Pressed");
          uint8_t button_state = GPIO_PinInGet(PB0_port,PB0_pin);
          if(button_state == 0){
              sl_status = sl_bt_sm_passkey_confirm(ble_data.connectionHandle, 1);
              if(sl_status != SL_STATUS_OK) {
                  LOG_ERROR("SM Passkey Confirm Error 0x%x",sl_status);
              }
          }
      }

      if(evt->data.evt_system_external_signal.extsignals == evtPB1_Pressed) {
          //LOG_INFO("PB1 Pressed");
          uint8_t button_state = GPIO_PinInGet(PB1_port,PB1_pin);
          if(button_state == 0) {
              sl_bt_gatt_read_characteristic_value(ble_data.connectionHandle,ble_data.ButtonCharacteristicsHandle);
          }
      }

      if(evt->data.evt_system_external_signal.extsignals == evtPB0_Pressed) {

          while(GPIO_PinInGet(PB0_port,PB0_pin) == 0) {
              if(GPIO_PinInGet(PB1_port,PB1_pin) == 0)  {

                  if(ble_data.ButtonIndicationFlag == true) {
                      sl_status = sl_bt_gatt_set_characteristic_notification(ble_data.connectionHandle,
                                                                             ble_data.ButtonCharacteristicsHandle,
                                                                             sl_bt_gatt_disable
                                                                            );
                      if(sl_status == SL_STATUS_OK) {
                          ble_data.ButtonIndicationFlag = false;
                      }
                  }

                  if(ble_data.ButtonIndicationFlag == false) {
                      sl_status = sl_bt_gatt_set_characteristic_notification(ble_data.connectionHandle,
                                                                             ble_data.ButtonCharacteristicsHandle,
                                                                             sl_bt_gatt_indication
                                                                            );
                      if(sl_status == SL_STATUS_OK) {
                          ble_data.ButtonIndicationFlag = true;
                      }
                  }

              }
          }
      }

      break;

    case sl_bt_evt_sm_confirm_passkey_id:
      //LOG_INFO("Confirm Passkey");
      displayPrintf(DISPLAY_ROW_PASSKEY, "Passkey %d", evt->data.evt_sm_confirm_passkey.passkey);
      displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
      ble_data.BondingFlag = 1;
      break;

    case sl_bt_evt_sm_bonded_id:
      //LOG_INFO("Bonded");
      displayPrintf(DISPLAY_ROW_PASSKEY, " ");
      displayPrintf(DISPLAY_ROW_ACTION, " ");
      displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
      ble_data.BondingFlag = 2;
      break;

    case sl_bt_evt_sm_bonding_failed_id:
      LOG_INFO("Bonding Failed");
      break;
  }

}   //    handle_client_events



// To handle ble events depending on
// client or server implementation
void handle_ble_event(sl_bt_msg_t *evt) {
  //LOG_INFO("Handle BLE Event");
  sl_status_t sl_status;
  uint8_t gattCount=0;

  switch (SL_BT_MSG_ID(evt->header)) {

    case sl_bt_evt_system_boot_id:
      LOG_INFO("Boot");
      displayInit();
      displayPrintf(DISPLAY_ROW_NAME, "%s",BLE_DEVICE_TYPE_STRING);
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "Course Project");

      // Extract unique ID from BT Address.
      sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddress_type);

      displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x",ble_data.myAddress.addr[0],
                                                            ble_data.myAddress.addr[1],
                                                            ble_data.myAddress.addr[2],
                                                            ble_data.myAddress.addr[3],
                                                            ble_data.myAddress.addr[4],
                                                            ble_data.myAddress.addr[5]
                    );

      sl_status = sl_bt_advertiser_create_set(&ble_data.advertisingSetHandle);
      if(sl_status != SL_STATUS_OK) {
          LOG_ERROR("Advertiser Create Set Error 0x%x",sl_status);
      }

      sl_status = sl_bt_advertiser_set_timing(ble_data.advertisingSetHandle,
                                              ble_data.Advertising_min,
                                              ble_data.Advertising_max,
                                              ble_data.Advertising_duration,
                                              ble_data.Advertising_maxevents
                                             );
      if(sl_status != SL_STATUS_OK) {
          LOG_ERROR("Advertiser Set Timing Error 0x%x",sl_status);
      }

      sl_status = sl_bt_advertiser_start(ble_data.advertisingSetHandle,
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

      sl_status = sl_bt_sm_configure(0x0f, sm_io_capability_displayyesno);
      if(sl_status != SL_STATUS_OK) {
          LOG_ERROR("SM Configure Error 0x%x",sl_status);
      }
      break;

    case sl_bt_evt_connection_opened_id:
      LOG_INFO("Connected");
      ble_data.connectionHandle = evt->data.evt_connection_opened.connection;

      sl_bt_advertiser_stop(ble_data.advertisingSetHandle);
      sl_status =  sl_bt_connection_set_parameters ( ble_data.connectionHandle,
                                                     ble_data.min_interval,
                                                     ble_data.max_interval,
                                                     ble_data.latency,
                                                     ble_data.timeout,
                                                     ble_data.min_ce_length,
                                                     ble_data.max_ce_length
                                                    );

      if(sl_status == SL_STATUS_OK) {
          //LOG_INFO("Connection Opened");
          displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
          ble_data.connectionFlag = true;
      }
      else {
          LOG_ERROR("Error 0x%x",sl_status);
      }
      break;

    case sl_bt_evt_sm_confirm_bonding_id:
      LOG_INFO("Confirm Bonding");
      sl_status = sl_bt_sm_bonding_confirm(ble_data.connectionHandle, 1);
      if(sl_status != SL_STATUS_OK) {
          LOG_ERROR("SM Bonding Confirm Error 0x%x",sl_status);
      }
      break;

    case sl_bt_evt_sm_confirm_passkey_id:
      LOG_INFO("Confirm Passkey");
      displayPrintf(DISPLAY_ROW_PASSKEY, "Passkey %d", evt->data.evt_sm_confirm_passkey.passkey);
      displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
      ble_data.BondingFlag = 1;
      break;

    case sl_bt_evt_system_external_signal_id:
      if((evt->data.evt_system_external_signal.extsignals == evtPB0_Pressed))  {
          LOG_INFO("Pressed");
          sl_status = sl_bt_sm_passkey_confirm(ble_data.connectionHandle, 1);
          if(sl_status != SL_STATUS_OK) {
              LOG_ERROR("SM Passkey Confirm Error 0x%x",sl_status);
          }
      }
      break;

    case sl_bt_evt_sm_bonded_id:
      LOG_INFO("Bonded");
      displayPrintf(DISPLAY_ROW_PASSKEY, " ");
      displayPrintf(DISPLAY_ROW_ACTION, " ");
      displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");

      sl_status = sl_bt_gatt_discover_primary_services_by_uuid(ble_data.connectionHandle,
                                                               sizeof(thermo_service),
                                                               (const uint8_t*)thermo_service
                                                              );
      if(sl_status != SL_STATUS_OK) {
          LOG_ERROR("Primary Services Discovery Error 0x%04x",sl_status);
      }
      break;

    case sl_bt_evt_sm_bonding_failed_id:
      LOG_ERROR("Bonding Failed");
      break;

    case sl_bt_evt_gatt_procedure_completed_id:
      LOG_INFO("Gatt Complete");
      if(gattCount == 0)  {
          LOG_INFO("Count 0");
          sl_status = sl_bt_gatt_discover_characteristics_by_uuid(ble_data.connectionHandle,
                                                                  ble_data.ButtonServiceHandle,
                                                                  sizeof(button_char),
                                                                  (const uint8_t*)button_char
                                                                 );
          if(sl_status != SL_STATUS_OK) {
              LOG_ERROR("Button Discover Characteristics Error 0x%04x",sl_status);
          }
          else {
              gattCount = 1;
          }
      }

      if(gattCount == 1)  {
          LOG_INFO("Count 1");
          sl_status = sl_bt_gatt_set_characteristic_notification(ble_data.connectionHandle,
                                                                 ble_data.HTMCharacteristicsHandle,
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

      break;

    case sl_bt_evt_connection_closed_id:
      LOG_INFO("Closed");
      gattCount = 0;
      sl_status = sl_bt_advertiser_start(ble_data.advertisingSetHandle,
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
      break;
  }

}   //    handle_ble_event
