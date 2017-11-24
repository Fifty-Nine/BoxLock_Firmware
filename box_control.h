#ifndef BOX_CONTROL_H_
#define BOX_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif


void boxInit(void);
void unlock();
bool tryUnlock(const char *pin);
void setPin(const char *pin);
bool trySetPin(const char *oldPin, const char *newPin);

#ifdef __cplusplus
}
#endif

#endif /* BOX_CONTROL_H_ */
