#include "mcu.h"

#include <hal_init.h>
#include <samd21.h>

void mcu::init()
{
    init_mcu();
}

void mcu::reset()
{
    NVIC_SystemReset();
    while (1) {}
}

mcu::reset_reason
mcu::getResetReason()
{
    auto pm_val = PM->RCAUSE.reg;
    return
        (pm_val & PM_RCAUSE_POR)   ? reset_reason::PowerOn :
        (pm_val & PM_RCAUSE_BOD12) ? reset_reason::BrownOut12 :
        (pm_val & PM_RCAUSE_BOD33) ? reset_reason::BrownOut33 :
        (pm_val & PM_RCAUSE_EXT)   ? reset_reason::External :
        (pm_val & PM_RCAUSE_WDT)   ? reset_reason::Watchdog :
        (pm_val & PM_RCAUSE_SYST)  ? reset_reason::System :
                                     reset_reason::Unknown;
}
