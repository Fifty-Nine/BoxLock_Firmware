#ifndef MCU_H
#define MCU_H

namespace mcu {

void init();
void reset() __attribute__((noreturn));
void breakpoint();
void assert(bool value);

} /* namespace mcu */

#endif /* MCU_H */
