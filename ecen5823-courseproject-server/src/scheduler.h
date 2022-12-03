/*******************************************************************************
 * @file    scheduler.h
 * @brief   Process the events and handles the LM75 temperature sensor state
 *          machine.
 *
 * @author  Amey More, Amey.More@colorado.edu
 * @date    Nov 24, 2022
 *
 * @editor  Nov 28, 2022, Ajay Kandagal, ajka9053@colorado.edu
 * @change  Added I2C and LETIMER0 events and state machine for handling LM75
 *          temperature sensor.
 *
 * @editor  Dec 2, 2022, Ajay Kandagal
 * @change  Updated temperature state machine to shutdown and wakeup LM75
 *          sensor. Added logic to handle button events.
 *
 ******************************************************************************/
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#include "sl_bt_api.h"


/******************************************************************************
 * @brief Generates an BT external signal when PB0 is pressed.
 ******************************************************************************/
void schedulerSetEventPB0Pressed(void);


/******************************************************************************
 * @brief Generates an BT external signal when PB1 is pressed.
 ******************************************************************************/
void schedulerSetEventPB1Pressed(void);


/******************************************************************************
 * @brief Generates an BT external signal when Button 1 is pressed from 1x4
 * keypad.
 ******************************************************************************/
void schedulerSetEventB1Pressed(void);


/******************************************************************************
 * @brief Generates an BT external signal when Button 2 is pressed from 1x4
 * keypad.
 ******************************************************************************/
void schedulerSetEventB2Pressed(void);


/******************************************************************************
 * @brief Generates an BT external signal when Button 3 is pressed from 1x4
 * keypad.
 ******************************************************************************/
void schedulerSetEventB3Pressed(void);


/******************************************************************************
 * @brief Generates an BT external signal when Button 4 is pressed from 1x4
 * keypad.
 ******************************************************************************/
void schedulerSetEventB4Pressed(void);


/******************************************************************************
 * @brief Generates an BT external signal when I2C complete event occurs.
 ******************************************************************************/
void schedulerSetI2CEventComplete(void);


/******************************************************************************
 * @brief Generates an BT external signal when I2C transfer error event occurs.
 ******************************************************************************/
void schedulerSetI2CEventFail(void);


/******************************************************************************
 * @brief Generates an BT external signal when LETIMER0 COMP0 event occurs.
 ******************************************************************************/
void schedulerSetTimerComp0Event(void);


/******************************************************************************
 * @brief Generates an BT external signal when LETIMER0 COMP1 event occurs.
 ******************************************************************************/
void schedulerSetTimerComp1Event(void);


/******************************************************************************
 * @brief Handles the button events.
 *
 * @param
 *  evt   Contains the BT external signal BT API message.
 ******************************************************************************/
void handle_button_events(sl_bt_msg_t *evt);


/******************************************************************************
 * @brief Handles the state machine of LM75 temperature sensor.
 *
 * @param
 *  evt   Contains the BT external signal BT API message.
 ******************************************************************************/
void temperatureStateMachine(sl_bt_msg_t *evt);


#endif  /* SCHEDULER_H */
