#include <FreeRTOS.h>
#include <hal_rtos.h>
#include <queue.h>
#include <timers.h>
#include <cstdio>
#include <cstring>

#include "atmel_start_pins.h"
#include "keypad.h"
#include "app_tasks.h"
#include "lock_control.h"

TaskHandle_t tasks::lockControl;
TaskHandle_t tasks::keypadScan;

namespace {

QueueHandle_t keypadQueue;
void keypadTimeout(TimerHandle_t handle)
{
    BaseType_t woken;
    xQueueSendToBackFromISR(keypadQueue, "!", &woken);
}

void lockControlTask(void *ctxt)
{
    TimerHandle_t timeout_timer = xTimerCreate(
        "Keypad Delay Timer",
        1000,
        pdFALSE,
        NULL,
        keypadTimeout   
    );

    char buffer[16] = { 0 };
    size_t idx = 0;
    while (1)
    {
        char c;
        xQueueReceive(keypadQueue, &c, portMAX_DELAY);
        
        if (c == '!') {
            memset(buffer, 0, 16);
            idx = 0;
            continue;
        } else if (c == '#') {
            if (idx != 16) {
                buffer[idx] = '\0';
            }
            if (lock::tryUnlock(buffer)) {
                memset(buffer, 0, 16);
                idx = 0;
                continue;
            }
        } else if (idx == 16) {
            memset(buffer, 0, 16);
            idx = 0;
            continue;
        }
        buffer[idx++] = c;
        xTimerReset(timeout_timer, portMAX_DELAY);
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
                if (held) {
                    xQueueSendToBack(keypadQueue, &c, 0);
                    printf("%c released\r\n", c);
                }
                } else if (pressed) {
                previous[i][j] = true;
                if (!held) {
                    printf("%c pressed\r\n", c);
                }
            }
        }

        gpio_set_pin_direction(columns[j], GPIO_DIRECTION_OFF);

        j = (j + 1) % nColumns;
        gpio_set_pin_direction(columns[j], GPIO_DIRECTION_OUT);
        gpio_set_pin_level(columns[j], false);
        os_sleep(5);
    }
}

} /* namespace */

void keypad::init()
{
    keypadQueue = xQueueCreate(16, sizeof(char));
    
    xTaskCreate(
        &lockControlTask,
        "Lock Control",
        64,
        NULL,
        tskIDLE_PRIORITY+1,
        &tasks::lockControl
    );
    xTaskCreate(
        &keypadScanTask,
        "Keypad Scanner",
        64,
        NULL,
        tskIDLE_PRIORITY+1,
        &tasks::keypadScan
    );

}
