#include "mtb.h"
#include "atmel_start_pins.h"
#include <hal_gpio.h>

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
    mtb::stop_trace();
    __asm("BKPT #0");
    errorBlink();
}

void (*vApplicationMallocFailedHook)() = HardFault_Handler;
void (*vApplicationStackOverflowHook)() = HardFault_Handler;

