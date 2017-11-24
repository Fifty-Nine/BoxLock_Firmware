#include <cstring>

#include "box_control.h"
#include "hal_gpio.h"
#include "atmel_start_pins.h"
#include "rtos_port.h"
#include "utility.h"
#include "nv_storage.h"


static const uint16_t pin_storage_id = 1;
static char pin[16] = { '1', '2', '3', '4' };

constexpr size_t lockout_wait_time = 5000;

void boxInit()
{
    if (nv_storage_item_exists(pin_storage_id)) {
        nv_storage_read(pin_storage_id, 0, (uint8_t*)&pin, 16);
    } else {
        setPin("1234");
    }
}

static bool checkPin(const char* guess)
{
    size_t i = 0;
    for (; i < 16 && guess[i] != '\0'; ++i) {
        if (pin[i] != mapToPhoneKeypad(guess[i])) {
            return false;
        }
    }
    
    return guess[i] == pin[i];
}

extern "C" void unlock(void)
{
    static const int charge_time = 200;
    static const int drive_time = 50;
    static const int hold_time = 2000;
    
    gpio_set_pin_level(PWM_EN, true);
    os_sleep(charge_time);
    gpio_set_pin_level(SOL_TRIG, true);
    os_sleep(drive_time);
    gpio_set_pin_level(PWM_EN, false);
    os_sleep(hold_time);
    gpio_set_pin_level(SOL_TRIG, false);
}

extern "C" bool tryUnlock(const char *guess)
{
    if (checkPin(guess)) {
        unlock();
        return true;
    } else {
        os_sleep(lockout_wait_time);
    }
    return false;
}

extern "C" void setPin(const char *newPin)
{
    for (size_t i = 0; i < 16 && newPin[i] != '\0'; ++i) {
        pin[i] = mapToPhoneKeypad(newPin[i]);
    }
    nv_storage_write(pin_storage_id, 0, (uint8_t*)pin, 16);
}

extern "C" bool trySetPin(const char *oldPin, const char *newPin)
{
    if (checkPin(oldPin)) {
        setPin(newPin);
        return true;
    } else {
        os_sleep(lockout_wait_time);
    }
    return false;
}
