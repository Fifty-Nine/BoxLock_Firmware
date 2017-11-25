#ifndef MCU_H
#define MCU_H

namespace mcu {

void reset() __attribute__((noreturn));
void breakpoint();

} /* namespace mcu */

#endif /* MCU_H */
