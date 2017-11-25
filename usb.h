#ifndef USB_DEVICE_MAIN_H
#define USB_DEVICE_MAIN_H

#include "cdcdf_acm.h"
#include "cdcdf_acm_desc.h"

#include <FreeRTOS.h>
#include <queue.h>

/**
 * \brief Initialize USB
 */
void usb_init(void);
int usb_write(char *buffer, size_t count);
int usb_read(char *buffer, size_t count);

#endif // USB_DEVICE_MAIN_H
