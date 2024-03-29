/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Date:        02-25-2022
 * Author:      Dave Sluiter
 * Description: This code was created by the Silicon Labs application wizard
 *              and started as "Bluetooth - SoC Empty".
 *              It is to be used only for ECEN 5823 "IoT Embedded Firmware".
 *              The MSLA referenced above is in effect.
 *
 *
 * Student Name: Amey More, amey.more@colorado.edu
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"
#include <em_cmu.h>

#include "sl_status.h"             // for sl_status_print()
#include "src/ble_device_type.h"
#include "src/gpio.h"
#include "src/lcd.h"
#include "src/ble.h"
#include "src/irq.h"
#include "src/oscillators.h"
#include "src/timers.h"
#include "src/scheduler.h"
#include "src/common.h"


// *************************************************
// Power Manager
// *************************************************
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)   // See: https://docs.silabs.com/gecko-platform/latest/service/power_manager/overview

// -----------------------------------------------------------------------------
// defines for power manager callbacks
// -----------------------------------------------------------------------------
// Return values for app_is_ok_to_sleep():
//   Return false to keep sl_power_manager_sleep() from sleeping the MCU.
//   Return true to allow system to sleep when you expect/want an IRQ to wake
//   up the MCU from the call to sl_power_manager_sleep() in the main while (1)
//   loop.
//
// Students: We'll need to modify this for A2 onward so that compile time we
//           control what the lowest EM (energy mode) the MCU sleeps to. So
//           think "#if (expression)".
#if LOWEST_ENERGY_MODE
#define APP_IS_OK_TO_SLEEP      (true)
#else
#define APP_IS_OK_TO_SLEEP      (false)
#endif

//#if (LOWEST_ENERGY_MODE>0 && LOWEST_ENERGY_MODE<4)
//#define APP_IS_OK_TO_SLEEP      (true)
//#endif


// Return values for app_sleep_on_isr_exit():
//   SL_POWER_MANAGER_IGNORE; // The module did not trigger an ISR and it doesn't want to contribute to the decision
//   SL_POWER_MANAGER_SLEEP;  // The module was the one that caused the system wakeup and the system SHOULD go back to sleep
//   SL_POWER_MANAGER_WAKEUP; // The module was the one that caused the system wakeup and the system MUST NOT go back to sleep
//
// Notes:
//       SL_POWER_MANAGER_IGNORE, we see calls to app_process_action() on each IRQ. This is the
//       expected "normal" behavior.
//
//       SL_POWER_MANAGER_SLEEP, the function app_process_action()
//       in the main while(1) loop will not be called! It would seem that sl_power_manager_sleep()
//       does not return in this case.
//
//       SL_POWER_MANAGER_WAKEUP, doesn't seem to allow ISRs to run. Main while loop is
//       running continuously, flooding the VCOM port with printf text with LETIMER0 IRQs
//       disabled somehow, LED0 is not flashing.

//#define APP_SLEEP_ON_ISR_EXIT   (SL_POWER_MANAGER_IGNORE)
//#define APP_SLEEP_ON_ISR_EXIT   (SL_POWER_MANAGER_SLEEP)
#define APP_SLEEP_ON_ISR_EXIT   (SL_POWER_MANAGER_WAKEUP)

#endif // defined(SL_CATALOG_POWER_MANAGER_PRESENT)


#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)

bool app_is_ok_to_sleep(void)
{
  return APP_IS_OK_TO_SLEEP;
} // app_is_ok_to_sleep()

sl_power_manager_on_isr_exit_t app_sleep_on_isr_exit(void)
{
  return APP_SLEEP_ON_ISR_EXIT;
} // app_sleep_on_isr_exit()

#endif // defined(SL_CATALOG_POWER_MANAGER_PRESENT)


/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
#if LOWEST_ENERGY_MODE == EM0
  init_LFXO();
#elif LOWEST_ENERGY_MODE == EM1
  init_LFXO();
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#elif LOWEST_ENERGY_MODE == EM2
  init_LFXO();
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM2);
#elif LOWEST_ENERGY_MODE == EM3
  init_ULFRCO();
#endif
  gpioInit();
  ble_init();
  IRQ_Init();

  init_LFXO();

  init_LETIMER0(0, LETIMER_PERIOD_MS);
} // app_init()


/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{

} // app_process_action()


/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *
 * The code here will process events from the Bluetooth stack. This is the only
 * opportunity we will get to act on an event.
 *
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  handle_ble_event(evt);

  handle_button_events(evt);

  temperatureStateMachine(evt);
} // sl_bt_on_event()
