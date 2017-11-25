#include <stdlib.h>
#include "driver_init.h"
#include "persistent_storage_start.h"
#include "usb.h"
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <hal_rtos.h>
#include <stdbool.h>
#include <stdio.h>
#include <timers.h>

#include "box_control.h"
#include "box_console.h"
#include "mtb.h"

static QueueHandle_t stdoutQueue;
static QueueHandle_t stdinQueue;
static QueueHandle_t keypadQueue;
static bool usb_rts = false;

extern "C"
int _read(int fd, uint8_t * const buf, size_t count)
{
	if (count == 0 || fd != 0) { return 0; }
	size_t rc = 0;
	(void)xQueueReceive(stdinQueue, &buf[0], portMAX_DELAY);
	
	rc++;
	while (rc < count && xQueueReceive(stdinQueue, &buf[rc], 0) == pdTRUE) {
		rc++;
	}
	return rc;
}

extern "C"
int _write(int fd, const char * const buf, size_t count)
{
	if (count == 0 || (fd != 1 && fd != 2)) { return 0; }
	if (!usb_rts) { return count; }
	size_t rc = 0;

	while (rc < count && xQueueSendToBack(stdoutQueue, &buf[rc], portMAX_DELAY) == pdTRUE) {
		rc++;
	}
	return rc;
}

extern "C"
int _isatty(int fd)
{
    return fd == 0 || fd == 1 || fd == 2;
}


static TaskHandle_t lockCtrlTaskHandle;
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
			if (tryUnlock(buffer)) {
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

static TaskHandle_t keypadScanTaskHandle;
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

static TaskHandle_t usbTaskHandle;
static void usb_task(void* ctxt)
{
	static char buffer[64];
	while (1) {
		while (xQueuePeek(stdoutQueue, &buffer[0], 100) != pdTRUE) { }
		
		if (!usb_rts) { continue; }
		
		int idx = 0;
		while (idx < 64 && xQueueReceive(stdoutQueue, &buffer[idx], 0) == pdTRUE) { idx++; }
		
		cdcdf_acm_write((uint8_t*)&buffer, idx);
	}
}

static uint8_t usb_recv_buffer[64];
static bool pending_read = false;
static void begin_read()
{
	if (!pending_read) {
		cdcdf_acm_read(usb_recv_buffer, sizeof(usb_recv_buffer));
		pending_read = true;
	}
}

static bool usb_read_callback(const uint8_t ep, const enum usb_xfer_code xc, const uint32_t count)
{
	if (xc == USB_XFER_DONE) {
		for (size_t i = 0; i < count; ++i) {
			BaseType_t woken;
			xQueueSendToBackFromISR(stdinQueue, &usb_recv_buffer[i], &woken);
		}
		pending_read = false;
		begin_read();
	}
	
	return false;
}


static TaskHandle_t consoleTaskHandle = NULL;

static bool usb_line_state_changed(usb_cdc_control_signal_t newState)
{
	static bool callbacks_registered = false;
	usb_rts = cdcdf_acm_is_enabled() && newState.rs232.RTS;
	bool usb_dtr = cdcdf_acm_is_enabled() && newState.rs232.DTR;
	
	if (cdcdf_acm_is_enabled() && !callbacks_registered) {
		callbacks_registered = true;
		cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)usb_read_callback);
	}
	
	if (usb_dtr) {
		begin_read();
		
		if (!consoleTaskHandle) {
            startConsoleTask();
		}
	} else if (!usb_dtr && consoleTaskHandle) {
        stopConsoleTask();
	}
	
	return false;
}

int main(void)
{
    mtb::init(32);
    system_init();
    persistent_storage_init();
    usb_init();
	boxInit();
	
	stdinQueue = xQueueCreate(64, sizeof(char));
	stdoutQueue = xQueueCreate(64, sizeof(char));
	keypadQueue = xQueueCreate(16, sizeof(char));
	
	xTaskCreate(&lock_ctrl_task, "Lock Control", 256, NULL, tskIDLE_PRIORITY+1, &lockCtrlTaskHandle);
	xTaskCreate(&keypad_scan_task, "Keypad Scanner", 256, NULL, tskIDLE_PRIORITY+1, &keypadScanTaskHandle);
	xTaskCreate(&usb_task, "USB Task", 256, NULL, tskIDLE_PRIORITY+1, &usbTaskHandle);
	cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)usb_line_state_changed);
	vTaskStartScheduler();
}

