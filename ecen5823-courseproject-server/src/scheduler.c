/*******************************************************************************
 * @file    scheduler.c
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
#include "em_core.h"
#include "em_gpio.h"

#include "scheduler.h"
#include "ble.h"
#include "i2c.h"
#include "common.h"

typedef enum {
  EVT_PB0_Pressed = 1,
  EVT_PB1_Pressed = 2,
  EVT_B1_Pressed = 4,
  EVT_B2_Pressed = 16,
  EVT_B3_Pressed = 32,
  EVT_B4_Pressed = 64,
  EVT_I2C_TR_SUCCESS = 128,
  EVT_I2C_TR_FAIL = 256,
  EVT_TIMER_COMP0_UF = 512,
  EVT_TIMER_COMP1_UF = 1024
} event_type_t;

typedef enum {
  STATE_LM75_BOOT = 0,
  STATE_LM75_WAKEUP,
  STATE_LM75_READ_TEMP,
  STATE_LM75_SHUTDOWN
}ftm_state_lm75_t;


#define LM75_DEV_ADDR         (0x48)
#define LM75_REG_CONG_ADDR    (0x01)
#define LM75_REG_TEMP_ADDR    (0x00)
#define LM75_REG_PID_ADDR     (0x07)
#define LM75_SHUTDOWN_MASK    (0x01)
#define LM75_INTERRUPT_MASK   (0x02)
#define MAX_I2C_FAIL_COUNT    (10)


ftm_state_lm75_t g_next_state_lm75 = STATE_LM75_BOOT;


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when PB0 is pressed.
 ******************************************************************************/
void schedulerSetEventPB0Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_PB0_Pressed);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when PB1 is pressed.
 ******************************************************************************/
void schedulerSetEventPB1Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_PB1_Pressed);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when Button 1 is pressed from 1x4 keypad.
 ******************************************************************************/
void schedulerSetEventB1Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_B1_Pressed);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when Button 2 is pressed from 1x4 keypad.
 ******************************************************************************/
void schedulerSetEventB2Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_B2_Pressed);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when Button 3 is pressed from 1x4 keypad.
 ******************************************************************************/
void schedulerSetEventB3Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_B3_Pressed);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when Button 4 is pressed from 1x4 keypad.
 ******************************************************************************/
void schedulerSetEventB4Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_B4_Pressed);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when I2C complete event occurs.
 ******************************************************************************/
void schedulerSetI2CEventComplete()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_I2C_TR_SUCCESS);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when I2C transfer error event occurs.
 ******************************************************************************/
void schedulerSetI2CEventFail()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_I2C_TR_FAIL);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when LETIMER0 COMP0 event occurs.
 ******************************************************************************/
void schedulerSetTimerComp0Event()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_TIMER_COMP0_UF);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Generates an BT external signal when LETIMER0 COMP1 event occurs.
 ******************************************************************************/
void schedulerSetTimerComp1Event()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_TIMER_COMP1_UF);

  CORE_EXIT_CRITICAL();
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Handles the button events.
 ******************************************************************************/
void handle_button_events(sl_bt_msg_t *evt)
{
  uint32_t event = evt->data.evt_system_external_signal.extsignals;

  if (event == 0)
    return;

  if (!(event & (EVT_B1_Pressed | EVT_B2_Pressed | EVT_B3_Pressed | EVT_B4_Pressed |
      EVT_PB0_Pressed | EVT_PB1_Pressed)))
    return;

  switch (event) {
    case EVT_B1_Pressed:
      LOG_INFO("Pressed B1");
      toggle_client_state(CLIENT_TYPE_HEATER);
      break;

    case EVT_B2_Pressed:
      LOG_INFO("Pressed B2");
      toggle_client_state(CLIENT_TYPE_AC);
      break;

    case EVT_B3_Pressed:
      LOG_INFO("Pressed B3");
      increase_taget_temperature();
      break;

    case EVT_B4_Pressed:
      LOG_INFO("Pressed B4");
      decrease_taget_temperature();
      break;

    case EVT_PB0_Pressed:
      LOG_INFO("Pressed PB0");
      pb0_event_handle();
      break;

    case EVT_PB1_Pressed:
      LOG_INFO("Pressed PB1");
      toggle_auto_feature();
      break;
  }
}


/******************************************************************************
 * @brief Handles the temperature state machine fail event.
 ******************************************************************************/
void handleI2CFailedEvent()
{
  g_next_state_lm75 = STATE_LM75_BOOT;
  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
  NVIC_DisableIRQ(I2C0_IRQn);
}


/******************************************************************************
 * SEE HEADER FILE FOR FULL DETAILS
 * Handles the state machine of LM75 temperature sensor.
 ******************************************************************************/
void temperatureStateMachine(sl_bt_msg_t *evt)
{
  uint32_t event = evt->data.evt_system_external_signal.extsignals;

  if (event == 0)
    return;

  if (!(event & (EVT_TIMER_COMP0_UF | EVT_I2C_TR_FAIL | EVT_I2C_TR_SUCCESS)))
    return;

  static uint8_t i2c_data[2];

  switch(g_next_state_lm75)
  {
    case STATE_LM75_BOOT:
      if (event & EVT_TIMER_COMP0_UF) {
          g_next_state_lm75 = STATE_LM75_WAKEUP;
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

          NVIC_ClearPendingIRQ(I2C0_IRQn);
          NVIC_EnableIRQ(I2C0_IRQn);

          i2c_data[0] = (uint8_t)LM75_REG_CONG_ADDR;
          i2c_data[1] = (uint8_t)LM75_INTERRUPT_MASK;
          I2C0_write(LM75_DEV_ADDR, i2c_data, 2);
      }
      break;

    case STATE_LM75_WAKEUP:
      if ((event & EVT_I2C_TR_FAIL) || (event & EVT_TIMER_COMP0_UF)) {
          handleI2CFailedEvent();
      }
      else if (event & EVT_I2C_TR_SUCCESS) {
          g_next_state_lm75 = STATE_LM75_READ_TEMP;
          I2C0_read(LM75_DEV_ADDR, LM75_REG_TEMP_ADDR, i2c_data, 2);
      }
      break;

    case STATE_LM75_READ_TEMP:
      NVIC_DisableIRQ(I2C0_IRQn);
      if ((event & EVT_I2C_TR_FAIL) || (event & EVT_TIMER_COMP0_UF)) {
          handleI2CFailedEvent();
      }
      else if (event & EVT_I2C_TR_SUCCESS) {
          g_next_state_lm75 = STATE_LM75_SHUTDOWN;
          uint16_t i2c_data16 = i2c_data[0] << 8 | i2c_data[1];
          int16_t temp_val = (int16_t)((i2c_data16 * 9) / (5 *256)) + 32;

          if (temp_val != 33)
            update_current_temperature(temp_val);

          i2c_data[0] = (uint8_t)LM75_REG_CONG_ADDR;
          i2c_data[1] = (uint8_t)LM75_INTERRUPT_MASK | LM75_SHUTDOWN_MASK;
          I2C0_write(LM75_DEV_ADDR, i2c_data, 2);

          LOG_INFO("Temperature: %u\n", temp_val);
      }
      break;

    case STATE_LM75_SHUTDOWN:
      if ((event & EVT_I2C_TR_FAIL) || (event & EVT_TIMER_COMP0_UF)) {
          handleI2CFailedEvent();
      }
      else if (event & EVT_I2C_TR_SUCCESS) {
          g_next_state_lm75 = STATE_LM75_BOOT;
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          NVIC_DisableIRQ(I2C0_IRQn);
      }
      break;
  }
}
