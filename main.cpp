#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <hal_rtos.h>
#include <stdbool.h>
#include <stdio.h>
#include <timers.h>
#include <cerrno>

#include "app_tasks.h"
#include "lock_control.h"
#include "driver_init.h"
#include "mtb.h"
#include "persistent_storage_start.h"
#include "usb.h"

static QueueHandle_t keypadQueue;

TaskHandle_t tasks::lockControl;
static void lock_task_timer_callback(TimerHandle_t handle)
{
	BaseType_t woken;
	xQueueSendToBackFromISR(keypadQueue, "!", &woken);
}

static void lock_ctrl_task(void *ctxt)
{
	TimerHandle_t timeout_timer = xTimerCreate(
		"Keypad Delay Timer",
		1000,
		pdFALSE,
		NULL,
		lock_task_timer_callback
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

TaskHandle_t tasks::keypadScan;
static void keypad_scan_task(void *ctxt)
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

int main(void)
{
    mtb::init(32);
    system_init();
    persistent_storage_init();
    usb_init();
    lock::init();
	
	keypadQueue = xQueueCreate(16, sizeof(char));
	
	xTaskCreate(
        &lock_ctrl_task,
        "Lock Control",
        64,
        NULL,
        tskIDLE_PRIORITY+1,
        &tasks::lockControl
    );
	xTaskCreate(
        &keypad_scan_task,
        "Keypad Scanner",
        64,
        NULL,
        tskIDLE_PRIORITY+1,
        &tasks::keypadScan
    );

	vTaskStartScheduler();
}

