#include "nvmem.h"

#include <hal_flash.h>
#include <hpl_pm_base.h>

#include "nv.h"
#include "nv_storage.h"

flash_descriptor flash_desc;

void nvmem::init(void)
{
    _pm_enable_bus_clock(PM_BUS_APBB, NVMCTRL);
    flash_init(&flash_desc, NVMCTRL);
    nv_init(&flash_desc);
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
