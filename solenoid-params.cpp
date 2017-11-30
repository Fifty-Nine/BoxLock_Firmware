#include "solenoid-params.h"

#include "nvmem.h"

namespace {

solenoid::params defaultParams()
{
    return { 200, 50, 2000 };
}

} /* namespace */

solenoid::params solenoid::getParams()
{
    params result;
    if (!nvmem::read(
            nvmem::solenoid_params_id,
            (void*)&result,
            sizeof(result))) 
    {
        result = defaultParams();
        setParams(result);
    }
    return result;
}

void solenoid::setParams(solenoid::params p)
{
    nvmem::write(nvmem::solenoid_params_id, (void*)&p, sizeof(p));
}
