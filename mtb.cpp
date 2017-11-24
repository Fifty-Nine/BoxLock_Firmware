#include "mtb.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

struct mtb::registers_t mtb::regs __attribute__((section(".mtb")));

extern "C" void* memalign(size_t, size_t);

static intptr_t *mtb_buffer;

void mtb::init(size_t buffer_size)
{
    assert(!mtb_buffer);
    assert(__builtin_popcount(buffer_size) == 1 && buffer_size >= 16);
    mtb_buffer = (intptr_t*)memalign(
        buffer_size, buffer_size * sizeof(intptr_t)
    );
    assert(mtb_buffer);

    uint8_t mask = __builtin_ctz(buffer_size) - 4;
    assert(mask <= 0x1f);

    intptr_t buffer_address = (intptr_t)mtb_buffer;
    memset(mtb_buffer, 0, buffer_size * sizeof(intptr_t));
    regs.position = (buffer_address - regs.base) & 0xFFFFFFF8;
    regs.flow = (buffer_address + buffer_size * sizeof(intptr_t)) & 0xFFFFFFF8;
    regs.master = 0x80000000 | mask;
}

