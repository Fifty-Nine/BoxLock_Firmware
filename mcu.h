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

enum class reset_reason {
    Unknown,
    PowerOn,
    BrownOut12,
    BrownOut33,
    External,
    Watchdog,
    System
};
reset_reason getResetReason();

} /* namespace mcu */

#endif /* MCU_H */
