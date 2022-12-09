/*******************************************************************************
 * @file  gpio.h
 * @brief Header file
 *******************************************************************************
 * Editor: Dec 08, 2022, Amey More
 ******************************************************************************/

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_

#define LED0_port  gpioPortF // change to correct ports and pins
#define LED0_pin   4
#define LED1_port  gpioPortF
#define LED1_pin   5

#define SENSOR_ENABLE_port  gpioPortD
#define SENSOR_ENABLE_pin   15

#define EXTCOMIN_port  gpioPortD
#define EXTCOMIN_pin   13

#define PB0_port gpioPortF
#define PB0_pin 6

#define PB1_port gpioPortF
#define PB1_pin 7

#define RELAY_PORT gpioPortA
#define RELAY_PIN 3

// Function prototypes
void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();

void gpioRelayOn();
void gpioRelayOff();

void gpioSensorEnSetOn();
void gpioSetDisplayExtcomin(bool value);


#endif /* SRC_GPIO_H_ */
