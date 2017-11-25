#ifndef MTB_H
#define MTB_H

#include <cstddef>
#include <cstdint>

namespace mtb {

#pragma pack(push, 4)
extern struct registers_t
{
    uint32_t position;
    uint32_t master;
    uint32_t flow;
    uint32_t base;
} regs;
#pragma pack(pop)

void init(size_t buffer_size);

inline void stop_trace() {
    regs.master &= ~0x10000000;
}

inline void resume_trace() {
    regs.master |= ~0x10000000;
}

} /* namespace mtb */

#endif /* MTB_H */
