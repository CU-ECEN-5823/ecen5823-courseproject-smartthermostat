/*******************************************************************************
 * @file        scheduler.c
 * @brief       Schedule and process events
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
 ******************************************************************************/

#include "scheduler.h"
#include "em_core.h"
#include "em_gpio.h"
#include <app_log.h>
#include "timers.h"
#include "i2c.h"
#include "ble.h"
#include "lcd.h"
#include "math.h"

#define INCLUDE_LOG_DEBUG (1)
#include "log.h"

#define SI7021_DEVICE_ADDR (0x40)

uint32_t myEvents = 0;

// -----------------------------------------------
// Private function, original from Dan Walkes. I fixed a sign extension bug.
// We'll need this for Client A7 assignment to convert health thermometer
// indications back to an integer. Convert IEEE-11073 32-bit float to signed integer.
// -----------------------------------------------
static int32_t FLOAT_TO_INT32(const uint8_t *value_start_little_endian)
{
  uint8_t signByte = 0;
  int32_t mantissa;
  // input data format is:
  // [0] = flags byte
  // [3][2][1] = mantissa (2's complement)
  // [4] = exponent (2's complement)
  // BT value_start_little_endian[0] has the flags byte
  int8_t exponent = (int8_t)value_start_little_endian[4];
  // sign extend the mantissa value if the mantissa is negative
  if (value_start_little_endian[3] & 0x80) { // msb of [3] is the sign of the mantissa
      signByte = 0xFF;
  }
  mantissa = (int32_t) (value_start_little_endian[1] << 0) |
             (value_start_little_endian[2] << 8) |
             (value_start_little_endian[3] << 16) |
             (signByte << 24) ;
  // value = 10^exponent * mantissa, pow() returns a double type
  return (int32_t) (pow(10, exponent) * mantissa);
} // FLOAT_TO_INT32

// This function sets the event
// triggered by LETIMER0 UF
void schedulerSetEventUF(void) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

//  myEvents |= (1<<evtLETIMER0_UF);
  sl_bt_external_signal(evtLETIMER0_UF);

  CORE_EXIT_CRITICAL();
}   //  schedulerSetEventUF

// This function sets the event
// triggered by LETIMER0 COMP1
void schedulerSetEventCOMP1(void) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

//  myEvents |= (1<<evtLETIMER0_COMP1);
  sl_bt_external_signal(evtLETIMER0_COMP1);

  CORE_EXIT_CRITICAL();
}   //  schedulerSetEventCOMP1

// This function sets the event
// triggered by I2C
void schedulerSetEventI2CDone(void) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

//  myEvents |= (1<<evtI2C_Done);
  sl_bt_external_signal(evtI2C_Done);

  CORE_EXIT_CRITICAL();
}   //  schedulerSetEventI2cDone

// This function returns one
// event to the app process
//uint32_t getNextEvent(void) {
//  uint32_t theEvent = 0;
//
//  if((myEvents & (1<<evtLETIMER0_UF)) == (1<<evtLETIMER0_UF)) {
////      LOG_INFO("U");
//      theEvent = evtLETIMER0_UF;
//      myEvents &= ~(1<<evtLETIMER0_UF);
//  }
//
//  if((myEvents & (1<<evtLETIMER0_COMP1)) == (1<<evtLETIMER0_COMP1)) {
////      LOG_INFO("C1");
//      theEvent = evtLETIMER0_COMP1;
//      myEvents &= ~(1<<evtLETIMER0_COMP1);
//  }
//
//  if((myEvents & (1<<evtI2C_Done)) == (1<<evtI2C_Done)) {
//            theEvent = evtI2C_Done;
//            myEvents &= ~(1<<evtI2C_Done);
//  }
//
//  return theEvent;
//}   //  getNextEvent


void schedulerSetEventPB0Pressed() {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

//  myEvents |= (1<<evtLETIMER0_COMP1);
  sl_bt_external_signal(evtPB0_Pressed);

  CORE_EXIT_CRITICAL();
}   //    schedulerSetEventPB0Pressed()

void schedulerSetEventPB1Pressed() {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

//  myEvents |= (1<<evtLETIMER0_COMP1);
  sl_bt_external_signal(evtPB1_Pressed);

  CORE_EXIT_CRITICAL();
}   //    schedulerSetEventPB1Pressed()


// Handles state machine for
// Temperature measurement
void temperature_state_machine(sl_bt_msg_t *event) {

  ble_data_struct_t *bleDataPtr = getbleData();
  uint32_t temperature_in_c;

  states_t currentState;
  static states_t nextState = IdleState;
  currentState = nextState;

  if((bleDataPtr->connectionFlag == true) && (bleDataPtr->TemperatureIndicationFlag == true) && (SL_BT_MSG_ID(event->header) == sl_bt_evt_system_external_signal_id)) {
      switch(currentState) {
          case (IdleState):
              nextState = IdleState;
//              if(event == evtLETIMER0_UF) {
              if(event->data.evt_system_external_signal.extsignals == evtLETIMER0_UF) {
                  //LOG_INFO("PowerOn");
                  //GPIO_PinOutSet(SENSOR_ENABLE_port,SENSOR_ENABLE_pin);
                  timerWaitUs_irq(80000);
                  nextState = PowerOn;
              }
              break;
          case (PowerOn):
              nextState = PowerOn;

//              if(event == evtLETIMER0_COMP1) {
              if(event->data.evt_system_external_signal.extsignals == evtLETIMER0_COMP1) {
                  //LOG_INFO("SiWrite");
                  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
                  si7021_send_cmd();
                  nextState = SiWrite;
              }
              break;
          case (SiWrite):
              nextState = SiWrite;

//              if(event == evtI2C_Done) {
                if(event->data.evt_system_external_signal.extsignals == evtI2C_Done) {
                  //LOG_INFO("SiRead");
                  NVIC_DisableIRQ(I2C0_IRQn);
                  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
                  timerWaitUs_irq(10800);
                  nextState = SiRead;
              }
              break;
          case (SiRead):
              nextState = SiRead;
      //        LOG_INFO("SiRead");
//              if(event == evtLETIMER0_COMP1) {
                if(event->data.evt_system_external_signal.extsignals == evtLETIMER0_COMP1) {
                  //LOG_INFO("PowerOff");
                  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
                  si7021_read_data();
                  nextState = PowerOff;
              }
              break;
          case (PowerOff):
              nextState = PowerOff;
//              if(event == evtI2C_Done) {
                if(event->data.evt_system_external_signal.extsignals == evtI2C_Done) {
                  //LOG_INFO("Idle");
                  temperature_in_c = print_temp();
                  print_temp_bt(temperature_in_c);
                  NVIC_DisableIRQ(I2C0_IRQn);
                  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
                  //GPIO_PinOutClear(SENSOR_ENABLE_port,SENSOR_ENABLE_pin);
                  nextState = IdleState;
              }
              break;
          }
  }
}   //    temperature_state_machine


// Function to check for Disconnection
static void checkDisconnection(discover_states_t *nextState)  {

  sl_status_t sl_status;
  ble_data_struct_t *bleDataPtr = getbleData();

  if(bleDataPtr->stateDisconnected == true){
    bleDataPtr->stateDisconnected = false;

    sl_status = sl_bt_sm_delete_bondings();
    if(sl_status != SL_STATUS_OK) {
        LOG_ERROR("Delete Bondings Error 0x%04x",sl_status);
    }

    sl_status = sl_bt_scanner_start(sl_bt_gap_phy_1m, sl_bt_scanner_discover_generic);
    if(sl_status == SL_STATUS_OK) {
        displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
        displayPrintf(DISPLAY_ROW_BTADDR2, " ");
        displayPrintf(DISPLAY_ROW_9, " ");
        displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
        *nextState = Connected;
    }
    else  {
        LOG_ERROR("Scanner Start Error 0x%04x",sl_status);
    }
  }
}   //    checkDisconnection()

// Handles state machine for
// Client's discovery of devices and
// Reading HTM and Button service and characteristics
void discover_state_machine(sl_bt_msg_t *event) {
  sl_status_t sl_status;
  ble_data_struct_t *bleDataPtr = getbleData();

  discover_states_t currentState;
  static discover_states_t nextState = Connected;

  currentState = nextState;

  switch(currentState)  {
    case (Connected):
        //LOG_INFO("Sch -> Connected");
        nextState =  Connected;

        if(bleDataPtr->stateConnected == true){

            bleDataPtr->stateConnected = false;
            bleDataPtr->stateCompleted = false;

            displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
            bleDataPtr->connectionHandle = event->data.evt_connection_opened.connection;

            sl_status = sl_bt_gatt_discover_primary_services_by_uuid(bleDataPtr->connectionHandle,
                                                                     sizeof(thermo_service),
                                                                     (const uint8_t*)thermo_service
                                                                    );
            if(sl_status != SL_STATUS_OK) {
                LOG_ERROR("Temperature Primary Services Discovery Error 0x%04x",sl_status);
            }

            nextState =  DiscoverTemperatureServices;
        }

        checkDisconnection(&nextState);

        break;

    case (DiscoverTemperatureServices):
        //LOG_INFO("Sch -> DiscoverTemperatureServices");
        nextState =  DiscoverTemperatureServices;

        if(bleDataPtr->stateCompleted == true)  {

            bleDataPtr->stateCompleted = false;

            sl_status = sl_bt_gatt_discover_primary_services_by_uuid(bleDataPtr->connectionHandle,
                                                                     sizeof(button_service),
                                                                     (const uint8_t*)button_service
                                                                    );
            if(sl_status != SL_STATUS_OK) {
                LOG_ERROR("Button Primary Services Discovery Error 0x%04x",sl_status);
            }

            nextState = DiscoverButtonServices;
        }

        checkDisconnection(&nextState);

        break;

    case (DiscoverButtonServices):
        //LOG_INFO("Sch -> DiscoverButtonServices");
        nextState =  DiscoverButtonServices;

        if(bleDataPtr->stateCompleted == true){
            bleDataPtr->stateCompleted = false;

            sl_status = sl_bt_gatt_discover_characteristics_by_uuid(bleDataPtr->connectionHandle,
                                                                    bleDataPtr->HTMServiceHandle,
                                                                    sizeof(thermo_char),
                                                                    (const uint8_t*)thermo_char
                                                                   );
            if(sl_status != SL_STATUS_OK) {
                LOG_ERROR("Temperature Discover Characteristics Error 0x%04x",sl_status);
            }

            nextState = DiscoverTemperatureCharacteristics;
        }

        checkDisconnection(&nextState);

        break;

    case (DiscoverTemperatureCharacteristics):
        //LOG_INFO("Sch -> DiscoverTemperatureCharacteristics");
        nextState =  DiscoverTemperatureCharacteristics;

        if(bleDataPtr->stateCompleted == true){
            bleDataPtr->stateCompleted = false;

            sl_status = sl_bt_gatt_discover_characteristics_by_uuid(bleDataPtr->connectionHandle,
                                                                    bleDataPtr->ButtonServiceHandle,
                                                                    sizeof(button_char),
                                                                    (const uint8_t*)button_char
                                                                   );
            if(sl_status != SL_STATUS_OK) {
                LOG_ERROR("Button Discover Characteristics Error 0x%04x",sl_status);
            }

            nextState = DiscoverButtonCharacteristics;
        }

        checkDisconnection(&nextState);

        break;

    case (DiscoverButtonCharacteristics):
        //LOG_INFO("Sch -> DiscoverButtonCharacteristics");
        nextState =  DiscoverButtonCharacteristics;

        if(bleDataPtr->stateCompleted == true){
            bleDataPtr->stateCompleted = false;

            sl_status = sl_bt_gatt_set_characteristic_notification(bleDataPtr->connectionHandle,
                                                                   bleDataPtr->HTMCharacteristicsHandle,
                                                                   sl_bt_gatt_indication
                                                                  );
            if(sl_status == SL_STATUS_OK) {
                displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
            }
            else  {
                LOG_ERROR("Temperature Characteristics Notification Error 0x%04x",sl_status);
            }

            nextState =  SetTemperatureNotification;
        }

        checkDisconnection(&nextState);

        break;

    case (SetTemperatureNotification):
        //LOG_INFO("Sch -> SetTemperatureNotification");
        nextState =  SetTemperatureNotification;

        if(bleDataPtr->stateCompleted == true){
            bleDataPtr->stateCompleted = false;

            sl_status = sl_bt_gatt_set_characteristic_notification(bleDataPtr->connectionHandle,
                                                                   bleDataPtr->ButtonCharacteristicsHandle,
                                                                   sl_bt_gatt_indication
                                                                  );
            if(sl_status == SL_STATUS_OK) {
                displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
                bleDataPtr->ButtonIndicationFlag = true;
            }
            else  {
                LOG_ERROR("Button Characteristics Notification Error 0x%04x",sl_status);
            }

            nextState =  SetButtonNotification;
        }

        checkDisconnection(&nextState);

        break;

    case (SetButtonNotification):
        //LOG_INFO("Sch -> SetButtonNotification");
        nextState =  SetButtonNotification;
        bleDataPtr->stateCompleted = false;

        if(bleDataPtr->stateCharacteristics == true){
            bleDataPtr->stateCharacteristics = false;

            if(event->data.evt_gatt_characteristic_value.att_opcode != sl_bt_gatt_read_response)  {
                sl_status = sl_bt_gatt_send_characteristic_confirmation(bleDataPtr->connectionHandle);
                if(sl_status != SL_STATUS_OK) {
                    LOG_ERROR("Characteristics Confirmation Error 0x%04x",sl_status);
                }
            }

            if(event->data.evt_gatt_characteristic_value.characteristic == bleDataPtr->HTMCharacteristicsHandle)  {
                displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp = %d C",FLOAT_TO_INT32(&(event->data.evt_gatt_characteristic_value.value.data[0])));
            }

            if(event->data.evt_gatt_characteristic_value.characteristic == bleDataPtr->ButtonCharacteristicsHandle)  {
                //LOG_INFO("%x %x",event->data.evt_gatt_characteristic_value.value.data[0], event->data.evt_gatt_characteristic_value.value.data[1]);
                //LOG_INFO("Button Read");
                if(event->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
                    displayPrintf(DISPLAY_ROW_9, "Button Pressed");
                }
                if(event->data.evt_gatt_characteristic_value.value.data[0] == 0x00) {
                    displayPrintf(DISPLAY_ROW_9, "Button Released");
                }
            }

            nextState = SetButtonNotification;
        }

        if(bleDataPtr->stateCompleted == true)  {
            bleDataPtr->stateCompleted = false;
        }

        checkDisconnection(&nextState);

        break;
  }
}   //    discover_state_machine()
