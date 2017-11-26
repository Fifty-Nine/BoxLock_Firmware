#ifndef USB_DEVICE_MAIN_H
#define USB_DEVICE_MAIN_H

#include <cstddef>

namespace usb {

void init(void);
int write(char *buffer, size_t count);
int read(char *buffer, size_t count);

}

#endif // USB_DEVICE_MAIN_H
