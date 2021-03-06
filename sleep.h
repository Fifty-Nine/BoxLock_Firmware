#ifndef SLEEP_H
#define SLEEP_H

namespace sleep {

void maybeSleep();
void enterSleep();
void resetTimer();
void inhibit(bool inhibit);

} /* namespace sleep */

#endif /* SLEEP_H */
