/*******************************************************************************
 * @file  scheduler.c
 * @brief Scheduler functions
 *******************************************************************************
 * Editor: Dec 08, 2022, Amey More
 ******************************************************************************/

#include "scheduler.h"
#include "em_core.h"
#include "em_gpio.h"
#include <app_log.h>
#include "ble.h"
#include "lcd.h"
#include "math.h"

#define INCLUDE_LOG_DEBUG (1)
#include "log.h"

connection_states_t nextState;

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

void connection_state_machine(sl_bt_msg_t *event) {

  ble_client_data_t *bleDataPtr = getbleData();

  switch(nextState) {

    case Advertising:
      //LOG_INFO("Advertising");
      if(bleDataPtr->stateTransition == Connected)  {
          handle_bt_open(event);
          nextState = Connected;
      }
      break;

    case Connected:
      //LOG_INFO("Connected");
      if(bleDataPtr->stateTransition == ConfirmBonding) {
          handle_bt_confirm_bonding();
          nextState = ConfirmBonding;
      }

      if(bleDataPtr->stateTransition == Bonded) {
          LOG_INFO("connected->bonded");
          handle_bt_bonded();
          nextState = Bonded;
      }

      if(bleDataPtr->stateTransition == Advertising)  {
          handle_bt_close();
          nextState = Advertising;
      }
      break;

    case ConfirmBonding:
      //LOG_INFO("ConfirmBonding");
      if(bleDataPtr->stateTransition == ConfirmPasskey) {
          handle_bt_confirm_paskey(event);
          nextState = ConfirmPasskey;
      }

      if(bleDataPtr->stateTransition == Advertising)  {
          handle_bt_close();
          nextState = Advertising;
      }
      break;

    case ConfirmPasskey:
      //LOG_INFO("ConfirmPasskey");
      if(bleDataPtr->stateTransition == Bonded) {
          handle_bt_bonded();
          nextState = Bonded;
      }

      if(bleDataPtr->stateTransition == Advertising)  {
          handle_bt_close();
          nextState = Advertising;
      }
      break;

    case Bonded:
      //LOG_INFO("Bonded");
      if(bleDataPtr->stateTransition == DiscoverServices) {
          handle_bt_gatt_complete();
          nextState = DiscoverServices;
      }

      if(bleDataPtr->stateTransition == Advertising)  {
          handle_bt_close();
          nextState = Advertising;
      }
      break;

    case DiscoverServices:
      //LOG_INFO("DiscoverServices");
      if(bleDataPtr->stateTransition == DiscoverCharacteristics) {
          handle_bt_gatt_complete();
          nextState = DiscoverCharacteristics;
      }

      if(bleDataPtr->stateTransition == Advertising)  {
          handle_bt_close();
          nextState = Advertising;
      }
      break;

    case DiscoverCharacteristics:
      //LOG_INFO("DiscoverCharacteristics");
      if(bleDataPtr->stateTransition == SetNotification) {
          //handle_bt_gatt_complete();
          nextState = SetNotification;
      }

      if(bleDataPtr->stateTransition == Advertising)  {
          handle_bt_close();
          nextState = Advertising;
      }
      break;

    case SetNotification:
      //LOG_INFO("SetNotification");

      if(bleDataPtr->stateTransition == Advertising)  {
          handle_bt_close();
          nextState = Advertising;
      }
      break;
  }
}

