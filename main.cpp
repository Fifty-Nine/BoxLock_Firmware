#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <hal_rtos.h>
#include <stdbool.h>
#include <stdio.h>
#include <timers.h>
#include <cerrno>

#include "app_tasks.h"
#include "lock_control.h"
#include "keypad.h"
#include "driver_init.h"
#include "mtb.h"
#include "persistent_storage_start.h"
#include "usb.h"

int main(void)
{
    mtb::init(32);
    system_init();
    persistent_storage_init();
    usb_init();
    lock::init();
    keypad::init();
    
    vTaskStartScheduler();
}

