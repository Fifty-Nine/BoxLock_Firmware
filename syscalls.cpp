#include <cerrno>
#include <cstddef>
#include "hal_atomic.h"
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

extern intptr_t __heap_end__;
extern intptr_t __sram_end__;

extern "C"
void* _sbrk(intptr_t increment)
{
    static intptr_t *curr_end = &__heap_end__;
    intptr_t *prev_end = curr_end;
    intptr_t *new_end = curr_end;

    auto mod = increment % sizeof(intptr_t);
    increment /= 4;
    increment += mod ? 1 : 0;

    new_end = curr_end + increment;

    if (new_end > &__sram_end__) {
        errno = ENOMEM;
        return (void*)-1;
    }
    curr_end = new_end;
    return prev_end;
}

static hal_atomic_t malloc_lock;

extern "C"
void __malloc_lock()
{
    atomic_enter_critical(&malloc_lock);
}

extern "C"
void __malloc_unlock()
{
    atomic_leave_critical(&malloc_lock);
}
