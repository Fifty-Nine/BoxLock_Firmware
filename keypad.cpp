#include "keypad.h"
#include <FreeRTOS.h>      // for StaticTask_t, StaticQueue_t, StaticTimer_t
#include <hal_gpio.h>      // for gpio_set_pin_direction, gpio_get_pin_level
#include <queue.h>         // for QueueHandle_t, xQueueCreateStatic, xQueueR...
#include <sys/_stdint.h>   // for uint8_t
#include <timers.h>        // for TimerHandle_t, xTimerCreateStatic, xTimerR...
#include <cstdio>          // for NULL, size_t
#include <cstring>         // for memset
#include "app_tasks.h"     // for namespace tasks
#include "lock_control.h"  // for tryUnlock
#include "pins.h"          // for KEYPADI0, KEYPADI1, KEYPADI2, KEYPADI3
#include "portmacro.h"     // for StackType_t, TickType_t, portMAX_DELAY
#include "projdefs.h"      // for pdFALSE
#include "sleep.h"         // for sleep::resetTimer
#include "task.h"          // for xTaskCreateStatic, TaskHandle_t, tskIDLE_P...

TaskHandle_t tasks::keypadControl;
TaskHandle_t tasks::keypadScan;

namespace {

QueueHandle_t keypadQueue;
TimerHandle_t timeoutTimer;
TimerHandle_t beepTimer;

void keypadTimeout(TimerHandle_t handle)
{
    BaseType_t woken;
    xQueueSendToBackFromISR(keypadQueue, "!", &woken);
}

void beep(int ticks)
{
    if (xTimerChangePeriod(beepTimer, ticks, ticks)) {
        gpio_set_pin_level(BUZZER, true);
    }
}

void keypadControlTask(void *ctxt)
{
    char buffer[16] = { 0 };
    size_t idx = 0;
    beep(100);
    while (1)
    {
        char c;
        xQueueReceive(keypadQueue, &c, portMAX_DELAY);

        if (c == '!') {
            memset(buffer, 0, 16);
            idx = 0;
            beep(700);
        } else if (c == '#') {
            if (idx != 16) {
                buffer[idx] = '\0';
            }
            xTimerStop(timeoutTimer, portMAX_DELAY);
            sleep::resetTimer();
            if (!lock::tryUnlock(buffer, /*lockout*/false)) {
                /*
                 * Beep angrily and ignore any keypad input for
                 * the next 5 seconds.
                 */
                beep(100);
                vTaskDelay(200);
                beep(100);
                vTaskDelay(5000);
                xQueueReset(keypadQueue);
            } else {
                beep(2250);
            }
            memset(buffer, 0, 16);
            idx = 0;
        } else if (c == '*') {
            beep(100);
            sleep::resetTimer();
        } else if (idx == 16) {
            memset(buffer, 0, 16);
            idx = 0;
            beep(700);
            xTimerStop(timeoutTimer, portMAX_DELAY);
            sleep::resetTimer();
        } else {
            buffer[idx++] = c;
            beep(100);
            xTimerReset(timeoutTimer, portMAX_DELAY);
            sleep::resetTimer();
        }
    }
}

void keypadScanTask(void *ctxt)
{
    static const int nColumns = 3;
    static const int nRows = 4;
    static uint8_t columns[3] = {
        KEYPADO2, KEYPADO1, KEYPADO0
    };
    static uint8_t rows[4] = {
        KEYPADI3, KEYPADI2, KEYPADI1, KEYPADI0
    };
    static char matrix[4][3] = {
        { '1', '2', '3' },
        { '4', '5', '6' },
        { '7', '8', '9' },
        { '*', '0', '#' },
    };
    static bool previous[4][3] = {};
    
    for (int i = 0; i < nColumns; ++i) {
        gpio_set_pin_direction(columns[i], GPIO_DIRECTION_OFF);
    }
    
    int j = 0;
    while (true)
    {
        for (int i = 0; i < nRows; ++i) {
            char c = matrix[i][j];
            bool pressed = !gpio_get_pin_level(rows[i]);
            bool held = previous[i][j];
            if (!pressed) {
                previous[i][j] = false;
            } else if (pressed) {
                if (!held) {
                    xQueueSendToBack(keypadQueue, &c, 0);
                }
                previous[i][j] = true;
            }
        }

        gpio_set_pin_direction(columns[j], GPIO_DIRECTION_OFF);

        j = (j + 1) % nColumns;
        gpio_set_pin_direction(columns[j], GPIO_DIRECTION_OUT);
        gpio_set_pin_level(columns[j], false);
        vTaskDelay(5);
    }
}

StaticTask_t lockTaskCtxt;
StackType_t lockTaskStack[64];
StaticTask_t keypadTaskCtxt;
StackType_t keypadTaskStack[64];
StaticQueue_t keypadQueueCtxt;
uint8_t keypadQueueBuffer[16];
StaticTimer_t timeoutTimerCtxt;
StaticTimer_t beepTimerCtxt;

} /* namespace */

void keypad::init()
{
    /* Configure keypad IO. */
    gpio_set_pin_direction(KEYPADO0, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(KEYPADO0, true);
    gpio_set_pin_function(KEYPADO0, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(KEYPADO1, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(KEYPADO1, true);
    gpio_set_pin_function(KEYPADO1, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(KEYPADO2, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(KEYPADO2, true);
    gpio_set_pin_function(KEYPADO2, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(KEYPADI0, GPIO_DIRECTION_IN);
    gpio_set_pin_pull_mode(KEYPADI0, GPIO_PULL_UP);
    gpio_set_pin_function(KEYPADI0, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(KEYPADI1, GPIO_DIRECTION_IN);
    gpio_set_pin_pull_mode(KEYPADI1, GPIO_PULL_UP);
    gpio_set_pin_function(KEYPADI1, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(KEYPADI2, GPIO_DIRECTION_IN);
    gpio_set_pin_pull_mode(KEYPADI2, GPIO_PULL_UP);
    gpio_set_pin_function(KEYPADI2, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(KEYPADI3, GPIO_DIRECTION_IN);
    gpio_set_pin_pull_mode(KEYPADI3, GPIO_PULL_UP);
    gpio_set_pin_function(KEYPADI3, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(BUZZER, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(BUZZER, false);
    gpio_set_pin_function(BUZZER, GPIO_PIN_FUNCTION_OFF);

    /* Set up monitoring tasks. */
    keypadQueue = xQueueCreateStatic(
        16,
        sizeof(char),
        keypadQueueBuffer,
        &keypadQueueCtxt
    );
    timeoutTimer = xTimerCreateStatic(
        "Keypad Delay Timer",
        2000,
        pdFALSE,
        NULL,
        keypadTimeout,
        &timeoutTimerCtxt
    );
    beepTimer = xTimerCreateStatic(
        "Beep timer",
        200,
        pdFALSE,
        NULL,
        [](TimerHandle_t) { gpio_set_pin_level(BUZZER, false); },
        &beepTimerCtxt
    );

    tasks::keypadControl = xTaskCreateStatic(
        &keypadControlTask,
        "Keypad Control",
        sizeof(lockTaskStack) / sizeof(StackType_t),
        nullptr,
        tskIDLE_PRIORITY + 1,
        lockTaskStack,
        &lockTaskCtxt
    );
    tasks::keypadScan = xTaskCreateStatic(
        &keypadScanTask,
        "Keypad Scanner",
        sizeof(keypadTaskStack) / sizeof(StackType_t),
        nullptr,
        tskIDLE_PRIORITY + 1,
        keypadTaskStack,
        &keypadTaskCtxt
    );

}
