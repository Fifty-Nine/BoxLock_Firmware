/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
 */
#include "usb.h"
#include "box_console.h"
#include "task.h"

namespace {

#if CONF_USBD_HS_SP
uint8_t single_desc_bytes[] = {
    /* Device descriptors and Configuration descriptors list. */
    CDCD_ACM_HS_DESCES_LS_FS};
uint8_t single_desc_bytes_hs[] = {
    /* Device descriptors and Configuration descriptors list. */
    CDCD_ACM_HS_DESCES_HS};
#else
uint8_t single_desc_bytes[] = {
    /* Device descriptors and Configuration descriptors list. */
    CDCD_ACM_DESCES_LS_FS};
#endif

struct usbd_descriptors single_desc[]
    = {{single_desc_bytes, single_desc_bytes + sizeof(single_desc_bytes)}
#if CONF_USBD_HS_SP
       ,
       {single_desc_bytes_hs, single_desc_bytes_hs + sizeof(single_desc_bytes_hs)}
#endif
};

/** Ctrl endpoint buffer */
uint8_t ctrl_buffer[64];
QueueHandle_t stdinQueue = nullptr;
QueueHandle_t stdoutQueue = nullptr;
TaskHandle_t usbTaskHandle;
bool usb_rts;
uint8_t usb_recv_buffer[64];
bool pending_read = false;

} /* namespace */


/**
 * \brief CDC ACM Init
 */
void cdc_device_acm_init(void)
{
	/* usb stack init */
	usbdc_init(ctrl_buffer);

	/* usbdc_register_funcion inside */
	cdcdf_acm_init();

	usbdc_start(single_desc);
	usbdc_attach();
}

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

static bool usb_line_state_changed(usb_cdc_control_signal_t newState)
{
	static bool callbacks_registered = false;
	usb_rts = cdcdf_acm_is_enabled() && newState.rs232.RTS;
	bool usb_dtr = cdcdf_acm_is_enabled() && newState.rs232.DTR;
	
	if (cdcdf_acm_is_enabled() && !callbacks_registered) {
		callbacks_registered = true;
		cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)usb_read_callback);
	}
	
    stopConsoleTask();
	if (usb_dtr) {
		begin_read();
        startConsoleTask();
	}
	
	return false;
}

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

void usb_init(void)
{
	stdinQueue = xQueueCreate(64, sizeof(char));
	stdoutQueue = xQueueCreate(64, sizeof(char));
	cdc_device_acm_init();
	xTaskCreate(&usb_task, "USB Task", 256, NULL, tskIDLE_PRIORITY+1, &usbTaskHandle);
	cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)usb_line_state_changed);
}

int usb_read(char* buf, size_t count)
{
    if (count == 0) {
        return 0;
    }
	size_t rc = 0;
	(void)xQueueReceive(stdinQueue, &buf[0], portMAX_DELAY);
	
	rc++;
	while (rc < count && xQueueReceive(stdinQueue, &buf[rc], 0) == pdTRUE) {
		rc++;
	}
	return rc;
}

int usb_write(char *buf, size_t count)
{
    if (!usb_rts) { return count; } 
	size_t rc = 0;

	while (rc < count && xQueueSendToBack(stdoutQueue, &buf[rc], portMAX_DELAY) == pdTRUE) {
		rc++;
	}
	return rc;
}
