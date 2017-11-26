#include "driver_init.h"
#include "nv.h"
#include "nv_storage.h"
#include "nvmem.h"

void nvmem::init(void)
{
    nv_init(&FLASH_0);
    nv_storage_init();
}

