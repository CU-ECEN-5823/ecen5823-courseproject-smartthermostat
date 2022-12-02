/*
  gpio.c

   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.

 */


// *****************************************************************************
// Students:
// We will be creating additional functions that configure and manipulate GPIOs.
// For any new GPIO function you create, place that function in this file.
// *****************************************************************************

#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>

#include "gpio.h"


// Student Edit: Define these, 0's are placeholder values.
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.

#define LED0_port  gpioPortF // change to correct ports and pins
#define LED0_pin   4
#define LED1_port  gpioPortF
#define LED1_pin   5
#define LCD_EN_PORT gpioPortD
#define LCD_EN_PIN 15
#define EXTCOMIN_PORT gpioPortD
#define EXTCOMIN_PIN 13

// Set GPIO drive strengths and modes of operation
void gpioInit()
{

  // Student Edit:

  //GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

  //GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

  GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInputPullFilter, true);
  GPIO_ExtIntConfig (PB0_port, PB0_pin, PB0_pin, true, false, true);

  GPIO_PinModeSet(PB1_port, PB1_pin, gpioModeInputPullFilter, true);
  GPIO_ExtIntConfig (PB1_port, PB1_pin, PB1_pin, true, false, true);

  GPIO_PinModeSet(BUTTON_1_PORT, BUTTON_1_PIN, gpioModeInputPullFilter, true);
  GPIO_PinModeSet(BUTTON_2_PORT, BUTTON_2_PIN, gpioModeInputPullFilter, true);
  GPIO_PinModeSet(BUTTON_3_PORT, BUTTON_3_PIN, gpioModeInputPullFilter, true);
  GPIO_PinModeSet(BUTTON_4_PORT, BUTTON_4_PIN, gpioModeInputPullFilter, true);

  GPIO_ExtIntConfig(BUTTON_1_PORT, BUTTON_1_PIN, BUTTON_1_PIN, true, false, true);
  GPIO_ExtIntConfig(BUTTON_2_PORT, BUTTON_2_PIN, BUTTON_2_PIN, true, false, true);
  GPIO_ExtIntConfig(BUTTON_3_PORT, BUTTON_3_PIN, BUTTON_3_PIN, true, false, true);
  GPIO_ExtIntConfig(BUTTON_4_PORT, BUTTON_4_PIN, BUTTON_4_PIN, true, false, true);

} // gpioInit()


void gpioLed0SetOn()
{
  GPIO_PinOutSet(LED0_port,LED0_pin);
}


void gpioLed0SetOff()
{
  GPIO_PinOutClear(LED0_port,LED0_pin);
}


void gpioLed1SetOn()
{
  GPIO_PinOutSet(LED1_port,LED1_pin);
}


void gpioLed1SetOff()
{
  GPIO_PinOutClear(LED1_port,LED1_pin);
}

void gpioSensorEnSetOn()
{
  GPIO_PinOutSet(LCD_EN_PORT,LCD_EN_PIN);
}

void gpioSetDisplayExtcomin(bool extcomin_state)
{
  if (extcomin_state)
    GPIO_PinOutSet(EXTCOMIN_PORT, EXTCOMIN_PIN);
  else
    GPIO_PinOutClear(EXTCOMIN_PORT, EXTCOMIN_PIN);
}






