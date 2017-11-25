#include "mcu.h"

#include "samd21.h"

void mcu::reset()
{
    NVIC_SystemReset();
    while (1) {}
}
