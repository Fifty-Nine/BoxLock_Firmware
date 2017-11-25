#ifndef LOCK_CONTROL_H_
#define LOCK_CONTROL_H_

namespace lock {
void init(void);
void unlock();
bool tryUnlock(const char *pin);
void setPin(const char *pin);
bool trySetPin(const char *oldPin, const char *newPin);
} /* namespace lock */

#endif /* LOCK_CONTROL_H_ */
