#include <cerrno>
#include "usb.h"

extern "C"
int _read(int fd, const char *const buf, size_t count)
{
    if (fd != 0) {
        errno = EBADF;
        return 0;
    }
    return usb_read((char*)buf, count);
}

extern "C"
int _write(int fd, const char *const buf, size_t count)
{
	if (fd != 1 && fd != 2) {
        errno = EBADF;
        return 0;
    }
    return usb_write((char*)buf, count);
}

extern "C"
int _isatty(int fd)
{
    if (fd == 0 || fd == 1 || fd == 2) {
        return 1;
    }
    errno = EBADF;
    return 0;
}


