#include "sleep.h"

#include <FreeRTOS.h>
#include <timers.h>                    // for StaticTimer_t

#include "mcu.h"                       // for mcu::getResetReason
#include "pins.h"

namespace {
bool sleepMode __attribute__((section(".noinit")));

/*
 * Set up clock generators necessary for sleep mode. We just use the
 * 32kHz low-power oscillator to run the EIC.
 */
void initSleepClocking()
{
    /* Reset the GCLK to its initial state. */
    GCLK->CTRL.bit.SWRST = 1;
    while (GCLK->STATUS.bit.SYNCBUSY) { }

    /* Use an x1 prescale. */
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(1);
    while (GCLK->STATUS.bit.SYNCBUSY) { }

    /*
     * Route the 32kHz low-power oscillator to
     * clock generator 1.
     */
    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_ID(1) |
        GCLK_GENCTRL_SRC_OSCULP32K |
        GCLK_GENCTRL_GENEN;
    while (GCLK->STATUS.bit.SYNCBUSY) { }

    /*
     * Route clock generator 1 to the EIC.
     */
    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_EIC |
        GCLK_CLKCTRL_GEN_GCLK1 |
        GCLK_CLKCTRL_CLKEN;
    
    while (GCLK->STATUS.bit.SYNCBUSY) { }

    /* Enable the APBA bus clock. */
    PM->APBAMASK.bit.GCLK_ = 1;
    PM->APBAMASK.bit.EIC_ = 1;
    PM->APBASEL.bit.APBADIV = 0;
}

/*
 * We use the keypad '*' as an external interrupt to wake the
 * processor from its sleep state.
 */
void initKeypadInterrupt()
{
    EIC->CTRL.bit.SWRST = 1;
    while (EIC->STATUS.bit.SYNCBUSY) { }

    EIC->CONFIG[0].reg = EIC_CONFIG_SENSE2_FALL;
    EIC->INTENSET.bit.EXTINT2 = 1;
    EIC->WAKEUP.bit.WAKEUPEN2 = 1;

    EIC->CTRL.bit.ENABLE = 1;
    while (EIC->STATUS.bit.SYNCBUSY) { }
    NVIC_EnableIRQ(EIC_IRQn);

    mcu::enableInterrupts();
}

}

extern "C" void EIC_Handler()
{
    mcu::reset();
}

void sleep::maybeSleep()
{
    auto why = mcu::getResetReason();
    if (why == mcu::reset_reason::PowerOn) {
        sleepMode = false;
        return;
    } else if (why != mcu::reset_reason::System || !sleepMode) {
        return;
    }
    sleepMode = false;

    initSleepClocking();

    gpio_set_pin_direction(KEYPADO2, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(KEYPADO2, false);
    gpio_set_pin_function(KEYPADO2, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(KEYPADI0, GPIO_DIRECTION_IN);
    gpio_set_pin_pull_mode(KEYPADI0, GPIO_PULL_UP);
    gpio_set_pin_function(KEYPADI0, GPIO_PIN_FUNCTION_A);

    initKeypadInterrupt();
    while (1) {
    }
}

void sleep::enterSleep()
{
    sleepMode = true;
    mcu::reset();
}

void sleep::resetTimer()
{
    static StaticTimer_t timer;
    static TimerHandle_t handle = xTimerCreateStatic(
        "Sleep timer",
        /* Timeout (ms) */ 60000,
        /* auto-reload */ false,
        /* context */ nullptr,
        (TimerCallbackFunction_t)&enterSleep,
        &timer
    );
    xTimerReset(handle, portMAX_DELAY);
}
