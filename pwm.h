#ifndef PWM_H
#define PWM_H

#include <cstdint>

namespace pwm {

void init();
void enable(uint32_t period, uint32_t duty);
void disable();

} /* namespace pwm */

#endif /* PWM_H */
