/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef ATMEL_START_PINS_H_INCLUDED
#define ATMEL_START_PINS_H_INCLUDED

#include <hal_gpio.h>

// SAMD21 has 8 pin functions

#define GPIO_PIN_FUNCTION_A 0
#define GPIO_PIN_FUNCTION_B 1
#define GPIO_PIN_FUNCTION_C 2
#define GPIO_PIN_FUNCTION_D 3
#define GPIO_PIN_FUNCTION_E 4
#define GPIO_PIN_FUNCTION_F 5
#define GPIO_PIN_FUNCTION_G 6
#define GPIO_PIN_FUNCTION_H 7

#define SOL_TRIG GPIO(GPIO_PORTA, 3)
#define CPU_PWM GPIO(GPIO_PORTA, 4)
#define PWM_EN GPIO(GPIO_PORTA, 5)
#define LED_OUT GPIO(GPIO_PORTA, 10)
#define KEYPADO0 GPIO(GPIO_PORTA, 14)
#define KEYPADO1 GPIO(GPIO_PORTA, 15)
#define KEYPADO2 GPIO(GPIO_PORTA, 16)
#define KEYPADI0 GPIO(GPIO_PORTA, 17)
#define KEYPADI1 GPIO(GPIO_PORTA, 18)
#define KEYPADI2 GPIO(GPIO_PORTA, 19)
#define KEYPADI3 GPIO(GPIO_PORTA, 22)
#define PA24 GPIO(GPIO_PORTA, 24)
#define PA25 GPIO(GPIO_PORTA, 25)

#endif // ATMEL_START_PINS_H_INCLUDED
