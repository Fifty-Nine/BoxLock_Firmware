#include <cstdlib>
#include <malloc.h>

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
