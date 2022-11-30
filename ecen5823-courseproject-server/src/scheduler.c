/*******************************************************************************
 * @file        scheduler.c
 * @brief       Process the events and handles the LM75 temperature sensor state
 *              machine.
 * @author      Amey More, Amey.More@colorado.edu
 *
 * @due         Nov 24, 2022
 * @project     ecen5823-courseproject-server
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Fall 2022)
 * @instructor  David Sluiter
 *
 * @editor      Nov 28, 2022, Ajay Kandagal, ajka9053@colorado.edu
 * @change      Added I2C and LETIMER0 events and state machine for handling
 *              LM75 temperature sensor.
 *
 ******************************************************************************/
#include "em_core.h"
#include "em_gpio.h"

#include "scheduler.h"
#include "ble.h"
#include "i2c.h"

#define INCLUDE_LOG_DEBUG (1)
#include "log.h"


typedef enum {
  STATE_LM75_IDLE = 0,
  STATE_LM75_CONFIG,
  STATE_LM75_SELECT_TEMP_REG,
  STATE_LM75_READ_TEMP,
}ftm_state_lm75_t;


#define LM75_DEV_ADDR         (0x48)
#define LM75_REG_CONG_ADDR    (0x01)
#define LM75_REG_TEMP_ADDR    (0x00)
#define LM75_REG_PID_ADDR     (0x07)
#define LM75_SHUTDOWN_MASK    (0x01)
#define LM75_INTERRUPT_MASK   (0x02)


ftm_state_lm75_t g_next_state_lm75 = STATE_LM75_IDLE;


void schedulerSetEventPB0Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evtPB0_Pressed);

  CORE_EXIT_CRITICAL();
}


void schedulerSetEventPB1Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evtPB1_Pressed);

  CORE_EXIT_CRITICAL();
}


void schedulerSetEventB1Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evtB1_Pressed);

  CORE_EXIT_CRITICAL();
}


void schedulerSetEventB2Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evtB2_Pressed);

  CORE_EXIT_CRITICAL();
}


void schedulerSetEventB3Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evtB3_Pressed);

  CORE_EXIT_CRITICAL();
}


void schedulerSetEventB4Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evtB4_Pressed);

  CORE_EXIT_CRITICAL();
}


// This function sets scheduler flag to indicate I2C transfer is complete
void schedulerSetI2CEventComplete()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_I2C_TR_SUCCESS);

  CORE_EXIT_CRITICAL();
}


void schedulerSetI2CEventFail()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_I2C_TR_FAIL);

  CORE_EXIT_CRITICAL();
}


void schedulerSetTimerComp0Event()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_TIMER_COMP0_UF);

  CORE_EXIT_CRITICAL();
}


void schedulerSetTimerComp1Event()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(EVT_TIMER_COMP1_UF);

  CORE_EXIT_CRITICAL();
}

void handleI2CFailedEvent()
{
  if (get_ble_server_data()->failed_i2c_count >= MAX_I2C_FAIL_COUNT) {
    get_ble_server_data()->lm75_sensor_found = false;
    update_lcd();
  }
  else {
    get_ble_server_data()->failed_i2c_count++;
  }

  g_next_state_lm75 = STATE_LM75_IDLE;
  NVIC_DisableIRQ(I2C0_IRQn);
}


void temperatureStateMachine(sl_bt_msg_t *evt)
{
  uint32_t event = evt->data.evt_system_external_signal.extsignals;

  if (event != EVT_I2C_TR_SUCCESS && event != EVT_I2C_TR_FAIL &&
      event != EVT_TIMER_COMP0_UF)
    return;

  static uint8_t i2c_data[2];

  switch(g_next_state_lm75)
  {
    case STATE_LM75_IDLE:
      LOG_INFO("STATE_LM75_IDLE\n");
      if (event & EVT_TIMER_COMP0_UF) {
          g_next_state_lm75 = STATE_LM75_CONFIG;
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

          NVIC_ClearPendingIRQ(I2C0_IRQn);
          NVIC_EnableIRQ(I2C0_IRQn);

          i2c_data[0] = (uint8_t)LM75_REG_CONG_ADDR;
          i2c_data[1] = (uint8_t)LM75_INTERRUPT_MASK;
          I2C0_write(LM75_DEV_ADDR, i2c_data, 2);
      }
      break;

    case STATE_LM75_CONFIG:
      LOG_INFO("STATE_LM75_CONFIG\n");
      if (event & EVT_I2C_TR_FAIL) {
          handleI2CFailedEvent();
      }
      else if (event & EVT_I2C_TR_SUCCESS) {
          g_next_state_lm75 = STATE_LM75_SELECT_TEMP_REG;

          i2c_data[0] = (uint8_t)LM75_REG_TEMP_ADDR;
          I2C0_write(LM75_DEV_ADDR, i2c_data, 1);
      }
      break;

    case STATE_LM75_SELECT_TEMP_REG:
      LOG_INFO("STATE_LM75_SELECT_TEMP_REG\n");
      if (event & EVT_I2C_TR_FAIL) {
          handleI2CFailedEvent();
      }
      else if (event & EVT_I2C_TR_SUCCESS) {
          g_next_state_lm75 = STATE_LM75_READ_TEMP;

          I2C0_read((uint8_t)LM75_DEV_ADDR, i2c_data, 2);
      }

      break;
    case STATE_LM75_READ_TEMP:
      LOG_INFO("STATE_LM75_READ_TEMP\n");
      g_next_state_lm75 = STATE_LM75_IDLE;
      NVIC_DisableIRQ(I2C0_IRQn);
      if (event & EVT_I2C_TR_FAIL) {
          handleI2CFailedEvent();
      }
      else if (event & EVT_I2C_TR_SUCCESS) {
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
          uint16_t temp_val = i2c_data[0] << 8 | i2c_data[1];
          temp_val = ((temp_val * 9) / (5 *256)) + 32;
          get_ble_server_data()->current_temp = temp_val;
          update_lcd();

          LOG_INFO("Temperature: %u\n", temp_val);
      }
      break;
  }
}
