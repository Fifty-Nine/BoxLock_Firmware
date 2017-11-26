#include <FreeRTOS.h>
#include <task.h>                      // for vTaskStartScheduler
#include "keypad.h"                    // for keypad::init
#include "lock_control.h"              // for lock::init
#include "mcu.h"                       // for mcu::init
#include "mtb.h"                       // for mtb::init
#include "nvmem.h"                     // for nvmem::init
#include "usb.h"                       // for usb_init

int main(void)
{
    mtb::init(256);
    mcu::init();
    nvmem::init();
    usb::init();
    lock::init();
    keypad::init();
    
    vTaskStartScheduler();
}

