#ifndef USB_DEVICE_MAIN_H
#define USB_DEVICE_MAIN_H

#include "cdcdf_acm.h"
#include "cdcdf_acm_desc.h"

void cdc_device_acm_init(void);

/**
 * \brief Initialize USB
 */
void usb_init(void);

#endif // USB_DEVICE_MAIN_H
