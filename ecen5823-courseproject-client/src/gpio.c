/*******************************************************************************
 * @file  gpio.c
 * @brief Function definitions for GPIO
 *******************************************************************************
 * Editor: Dec 08, 2022, Amey More
 ******************************************************************************/

#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>

#include "gpio.h"

#define INCLUDE_LOG_DEBUG (1)
#include "log.h"

// Student Edit: Define these, 0's are placeholder values.
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.

// Set GPIO drive strengths and modes of operation
void gpioInit(){

  // Student Edit:

	//GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

	GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInputPullFilter, true);
	GPIO_ExtIntConfig (PB0_port, PB0_pin, PB0_pin, true, true, true);

	GPIO_PinModeSet(PB1_port, PB1_pin, gpioModeInputPullFilter, true);
	GPIO_ExtIntConfig (PB1_port, PB1_pin, PB1_pin, true, true, true);

	GPIO_DriveStrengthSet(RELAY_PORT, gpioDriveStrengthStrongAlternateStrong);
	GPIO_PinModeSet(RELAY_PORT, RELAY_PIN, gpioModePushPull, false);

} // gpioInit()


void gpioRelayOn()  {
  LOG_INFO("Relay ON");
  GPIO_PinOutSet(RELAY_PORT,RELAY_PIN);
}

void gpioRelayOff() {
  LOG_INFO("Relay OFF");
  GPIO_PinOutClear(RELAY_PORT,RELAY_PIN);
}

void gpioLed0SetOn(){
	GPIO_PinOutSet(LED0_port,LED0_pin);
}


void gpioLed0SetOff(){
	GPIO_PinOutClear(LED0_port,LED0_pin);
}


void gpioLed1SetOn(){
	GPIO_PinOutSet(LED1_port,LED1_pin);
}


void gpioLed1SetOff(){
	GPIO_PinOutClear(LED1_port,LED1_pin);
}

void gpioSensorEnSetOn() {
  GPIO_PinOutSet(SENSOR_ENABLE_port,SENSOR_ENABLE_pin);
}

void gpioSetDisplayExtcomin(bool value) {
  if(value == true) {
      GPIO_PinOutSet(EXTCOMIN_port,EXTCOMIN_pin);
  }
  else {
      GPIO_PinOutClear(EXTCOMIN_port,EXTCOMIN_pin);
  }
}
