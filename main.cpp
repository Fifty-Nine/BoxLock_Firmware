#include <FreeRTOS.h>
#include <task.h>                      // for vTaskStartScheduler
#include "driver_init.h"               // for system_init
#include "keypad.h"                    // for init
#include "lock_control.h"              // for init
#include "mtb.h"                       // for init
#include "persistent_storage_start.h"  // for persistent_storage_init
#include "usb.h"                       // for usb_init

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

