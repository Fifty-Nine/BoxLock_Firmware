#ifndef DEBUG_H
#define DEBUG_H

#include "mcu.h"
#include "optimization.h"

namespace debug {

inline void assert(bool value)
{
#ifndef NDEBUG
    if (unlikely(!value)) {
        mcu::breakpoint();
    }
#endif
}

} /* namespace */

#endif /* DEBUG_H */
