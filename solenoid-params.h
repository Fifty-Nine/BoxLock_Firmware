#ifndef SOLENOID_PARAMS_H
#define SOLENOID_PARAMS_H

namespace solenoid {

struct params
{
    unsigned charge_time;
    unsigned drive_time;
    unsigned hold_time;
};

params getParams();
void setParams(params p);

};

#endif /* SOLENOID_PARAMS_H */
