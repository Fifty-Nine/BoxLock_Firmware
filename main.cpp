#include <FreeRTOS.h>
#include <task.h>                      // for vTaskStartScheduler
#include "driver_init.h"               // for system_init
#include "keypad.h"                    // for init
#include "lock_control.h"              // for init
#include "mtb.h"                       // for init
#include "nvmem.h"                     // for nvmem::init
#include "usb.h"                       // for usb_init

int main(void)
{
    mtb::init(32);
    system_init();
    nvmem::init();
    usb::init();
    lock::init();
    keypad::init();
    
    vTaskStartScheduler();
}

