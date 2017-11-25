#ifndef APP_TASKS_H
#define APP_TASKS_H

#include <FreeRTOS.h>
#include <task.h>

namespace tasks {
extern TaskHandle_t lockControl;
extern TaskHandle_t keypadScan;
extern TaskHandle_t console;
}

#endif /* APP_TASKS_H */
