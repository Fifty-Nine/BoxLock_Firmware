#include <cstring>

#include "lock_control.h"
#include "hal_gpio.h"
#include "atmel_start_pins.h"
#include "rtos_port.h"
#include "utility.h"
#include "nv_storage.h"

namespace {

constexpr uint16_t pin_storage_id = 1;
char pin[16] = { '1', '2', '3', '4' };
constexpr size_t lockout_wait_time = 5000;

bool checkPin(const char* guess)
{
	size_t i = 0;
	for (; i < 16 && guess[i] != '\0'; ++i) {
		if (pin[i] != mapToPhoneKeypad(guess[i])) {
			return false;
		}
	}
	
	return guess[i] == pin[i];
}

} /* namespace */

void lock::init()
{
	if (nv_storage_item_exists(pin_storage_id)) {
		nv_storage_read(pin_storage_id, 0, (uint8_t*)&pin, 16);
	} else {
		setPin("1234");
	}
}

void lock::unlock(void)
{
	static const int charge_time = 200;
	static const int drive_time = 50;
	static const int hold_time = 2000;
	
    gpio_set_pin_level(LED_OUT, true);
	gpio_set_pin_level(PWM_EN, true);
	os_sleep(charge_time);
	gpio_set_pin_level(SOL_TRIG, true);
	os_sleep(drive_time);
	gpio_set_pin_level(PWM_EN, false);
	os_sleep(hold_time);
	gpio_set_pin_level(SOL_TRIG, false);
    gpio_set_pin_level(LED_OUT, false);
}

bool lock::tryUnlock(const char *guess)
{
	if (checkPin(guess)) {
		unlock();
		return true;
	} else {
		os_sleep(lockout_wait_time);
	}
	return false;
}

void lock::setPin(const char *newPin)
{
	for (size_t i = 0; i < 16 && newPin[i] != '\0'; ++i) {
		pin[i] = mapToPhoneKeypad(newPin[i]);
	}
	nv_storage_write(pin_storage_id, 0, (uint8_t*)pin, 16);
}

bool lock::trySetPin(const char *oldPin, const char *newPin)
{
	if (checkPin(oldPin)) {
		setPin(newPin);
		return true;
	} else {
		os_sleep(lockout_wait_time);
	}
	return false;
}
