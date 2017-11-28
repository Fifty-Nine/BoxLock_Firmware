#include "sleep.h"

#include <FreeRTOS.h>
#include <timers.h>                    // for StaticTimer_t

#include "mcu.h"                       // for mcu::getResetReason
#include "pins.h"

static bool sleepMode __attribute__((section(".noinit")));

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

    /* 
     * TODO: Implement actual sleep logic. This is just
     * a placeholder to make it clear that sleep mode was invoked
     */
    gpio_set_pin_direction(LED_OUT, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(LED_OUT, false);
    gpio_set_pin_function(LED_OUT, GPIO_PIN_FUNCTION_OFF);
    bool led = false;
    while (1) {
        int c = 0x80000;
        gpio_set_pin_level(LED_OUT, led = !led);
        while (--c) { asm(""); }
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
