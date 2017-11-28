#include <hal_gpio.h>  // for gpio_set_pin_level
#include <FreeRTOS.h>  // Required for task.h
#include <task.h>      // for TaskHandle_t
#include "mcu.h"       // for breakpoint, reset
#include "pins.h"      // for LED_OUT

extern "C"
void HardFault_Handler()
{
    mcu::breakpoint();
    mcu::reset();
}

extern "C"
void vApplicationStackOverflowHook(
    TaskHandle_t task,
    const char *name)
{
    mcu::breakpoint();
    mcu::reset();
}

void vApplicationMallocFailedHook()
{
    mcu::breakpoint();
    mcu::reset();
}
