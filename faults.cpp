#include <hal_gpio.h>  // for gpio_set_pin_level
#include <FreeRTOS.h>  // Required for task.h
#include <task.h>      // for TaskHandle_t
#include "mcu.h"       // for breakpoint, reset
#include "pins.h"      // for LED_OUT

static inline void errorBlink()
{
    bool v = true;

    while (1) {
        gpio_set_pin_level(LED_OUT, v = !v);
        int c = 0x80000;
        while (--c > 0) {
            asm("");
        }
    }
}

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
