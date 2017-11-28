#include "usb.h"

#include <hpl_gclk_base.h>          // for _gclk_enable_channel
#include <hpl_pm_base.h>       // for PM_BUS_APBB
#include <hpl_usb.h>           // for usb_xfer_code, usb_xfer_code::USB_XFER...
#include <sys/_stdint.h>       // for uint8_t, uint32_t
#include <utils.h>             // for FUNC_PTR

#include "app_tasks.h"         // for tasks::usb
#include "cdcdf_acm.h"         // for cdcdf_acm_is_enabled, cdcdf_acm_regist...
#include "cdcdf_acm_desc.h"    // for CDCD_ACM_DESCES_LS_FS
#include "debug.h"             // for debug::assert
#include "FreeRTOS.h"          // required for task.h, queue.h
#include "console.h"           // for startTask, stopTask
#include "portmacro.h"         // for TickType_t, portMAX_DELAY, BaseType_t
#include "pins.h"              // for USB_DM, USB_DP
#include "projdefs.h"          // for pdTRUE
#include "queue.h"             // for xQueueReceive, QueueHandle_t, xQueueCr...
#include "task.h"              // for xTaskCreate, TaskHandle_t, tskIDLE_PRI...
#include "usb_protocol_cdc.h"  // for usb_cdc_control_signal_t, usb_cdc_cont...
#include "usbd_config.h"       // for CONF_USBD_HS_SP
#include "usbdc.h"             // for usbdc_attach, usbdc_init, usbdc_start

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
    
    console::stopTask();
    if (usb_dtr) {
        begin_read();
        console::startTask();
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

namespace {

StaticTask_t usbTaskCtxt;
StackType_t usbStack[0xc0];
StaticQueue_t stdinQueueCtxt;
uint8_t stdinBuffer[64];
StaticQueue_t stdoutQueueCtxt;
uint8_t stdoutBuffer[64];

}

TaskHandle_t tasks::usb;
void usb::init(void)
{
    /* Set up USB clocking. */
    _pm_enable_bus_clock(PM_BUS_APBB, USB);
    _pm_enable_bus_clock(PM_BUS_AHB, USB);
    _gclk_enable_channel(USB_GCLK_ID, CONF_GCLK_USB_SRC);

    /* Initialize the peripheral. */
    usb_d_init();

    /* Configure USB pins. */
    gpio_set_pin_direction(USB_DM, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(USB_DM, false);
    gpio_set_pin_pull_mode(USB_DM, GPIO_PULL_OFF);
    gpio_set_pin_function(USB_DM, PINMUX_PA24G_USB_DM);

    gpio_set_pin_direction(USB_DP, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(USB_DP, false);
    gpio_set_pin_pull_mode(USB_DP, GPIO_PULL_OFF);
    gpio_set_pin_function(USB_DP, PINMUX_PA25G_USB_DP);

    /* Start the middleware. */
    cdc_device_acm_init();

    /* Set up monitoring task. */
    stdinQueue = xQueueCreateStatic(
        sizeof(stdinBuffer),
        sizeof(char),
        stdinBuffer,
        &stdinQueueCtxt
    );
    stdoutQueue = xQueueCreateStatic(
        sizeof(stdoutBuffer),
        sizeof(char),
        stdoutBuffer,
        &stdoutQueueCtxt
    );
    tasks::usb = xTaskCreateStatic(
        &usb_task,
        "USB Task",
        sizeof(usbStack) / sizeof(StackType_t),
        nullptr,
        tskIDLE_PRIORITY+1,
        usbStack,
        &usbTaskCtxt
    );
    debug::assert(tasks::usb);

    /* Register callbacks. */
    cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)usb_line_state_changed);
}

int usb::read(char* buf, size_t count)
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

int usb::write(char *buf, size_t count)
{
    if (!usb_rts) { return count; } 
    size_t rc = 0;

    while (rc < count && xQueueSendToBack(stdoutQueue, &buf[rc], portMAX_DELAY) == pdTRUE) {
        rc++;
    }
    return rc;
}
