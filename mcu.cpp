#include "mcu.h"

#include <hal_init.h>

#include "samd21.h"

void mcu::init()
{
    init_mcu();
}

void mcu::reset()
{
    NVIC_SystemReset();
    while (1) {}
}

void mcu::breakpoint()
{
#ifndef NDEBUG
    asm("BKPT #0");
#endif
}

void mcu::assert(bool value)
{
    if (!value) {
        breakpoint();
    }
}
