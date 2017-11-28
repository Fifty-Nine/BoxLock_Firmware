#ifndef MCU_H
#define MCU_H

namespace mcu {

void init();
void reset() __attribute__((noreturn));
inline void breakpoint()
{
#ifndef NDEBUG
    asm volatile("BKPT #0");
#endif
}

} /* namespace mcu */

#endif /* MCU_H */
