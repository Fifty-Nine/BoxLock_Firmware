#include <FreeRTOS.h>     // for StaticTask_t
#include <malloc.h>       // for mallinfo
#include <cstdint>        // for uint16_t
#include <cstdlib>        // for free, malloc, size_t
#include "portmacro.h"    // for StackType_t

extern "C"
void *pvPortMalloc(size_t xWantedSize)
{
    return malloc(xWantedSize);
}

extern "C"
void vPortFree(void *pv)
{
    return free(pv);
}

extern "C"
void vPortInitialiseBlocks(void)
{ }

extern "C"
size_t xPortGetFreeHeapSize(void)
{
    return mallinfo().fordblks;
}

extern "C"
void vApplicationGetIdleTaskMemory(
    StaticTask_t **task,
    StackType_t **stack,
    uint16_t *stackSize)
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
    uint16_t *stackSize)
{
    static StaticTask_t taskMem;
    static StackType_t stackMem[128];

    *task = &taskMem;
    *stack = stackMem;
    *stackSize = 128;
}
