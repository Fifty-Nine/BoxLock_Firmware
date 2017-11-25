#ifndef BOX_CONTROL_H_
#define BOX_CONTROL_H_

void boxInit(void);
void unlock();
bool tryUnlock(const char *pin);
void setPin(const char *pin);
bool trySetPin(const char *oldPin, const char *newPin);

#endif /* BOX_CONTROL_H_ */
