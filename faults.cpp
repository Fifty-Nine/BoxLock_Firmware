#include "mtb.h"
#include "pins.h"
#include <hal_gpio.h>
#include <FreeRTOS.h>
#include <task.h>
#include "mcu.h"

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
