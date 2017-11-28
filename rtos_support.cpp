#include <FreeRTOS.h>     // for StaticTask_t
#include <cstdint>        // for uint32_t
#include "portmacro.h"    // for StackType_t

extern "C"
void vApplicationGetIdleTaskMemory(
    StaticTask_t **task,
    StackType_t **stack,
    uint32_t *stackSize)
{
    static StaticTask_t taskMem;
    static StackType_t stackMem[128];

    *task = &taskMem;
    *stack = stackMem;
    *stackSize = 128;
}

extern "C"
void vApplicationGetTimerTaskMemory(
    StaticTask_t **task,
    StackType_t **stack,
    uint32_t *stackSize)
{
    static StaticTask_t taskMem;
    static StackType_t stackMem[128];

    *task = &taskMem;
    *stack = stackMem;
    *stackSize = 128;
}
