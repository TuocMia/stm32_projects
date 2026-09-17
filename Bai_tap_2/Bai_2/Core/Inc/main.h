#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f1xx_hal.h"


/*
 * ================= GPIO LED CONFIGURATION =================
 *
 * LED1 -> PA0 -> 0.1 Hz
 * LED2 -> PA1 -> 1 Hz
 * LED3 -> PA2 -> 10 Hz
 */

#define LED1_PORT GPIOA
#define LED1_PIN  GPIO_PIN_0

#define LED2_PORT GPIOA
#define LED2_PIN  GPIO_PIN_1

#define LED3_PORT GPIOA
#define LED3_PIN  GPIO_PIN_2


#endif