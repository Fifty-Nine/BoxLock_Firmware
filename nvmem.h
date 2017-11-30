#ifndef NVMEM_H
#define NVMEM_H

#include <cstddef>
#include <cstdint>

namespace nvmem {

void init(void);

enum ids : uint16_t
{
    invalid = 0,
    pin_id,
    solenoid_params_id,
};

bool read(ids id, void* data, size_t size);
void write(ids id, void *data, size_t size);

}

#endif /* NVMEM_H */
