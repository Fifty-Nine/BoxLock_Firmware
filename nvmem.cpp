#include "driver_init.h"
#include "nv.h"
#include "nv_storage.h"
#include "nvmem.h"

void nvmem::init(void)
{
    nv_init(&FLASH_0);
    nv_storage_init();
}

bool nvmem::read(ids id, void* data, size_t size)
{
    return nv_storage_read(id, 0, (uint8_t*)data, size) == ERR_NONE;
}

void nvmem::write(ids id, void *data, size_t size)
{
    nv_storage_write(id, 0, (uint8_t*)data, size);
}
