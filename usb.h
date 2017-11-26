#ifndef USB_DEVICE_MAIN_H
#define USB_DEVICE_MAIN_H

#include <cstddef>

/**
 * \brief Initialize USB
 */
void usb_init(void);
int usb_write(char *buffer, size_t count);
int usb_read(char *buffer, size_t count);

#endif // USB_DEVICE_MAIN_H
