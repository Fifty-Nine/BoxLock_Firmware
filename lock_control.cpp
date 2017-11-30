#include <cstdio>
#include <cstring>

#include "app_tasks.h"
#include "hal_gpio.h"
#include "lock_control.h"
#include "managed_task.h"
#include "nvmem.h"
#include "pins.h"
#include "solenoid-params.h"
#include "utility.h"

TaskHandle_t tasks::lockControl = nullptr;
namespace {

char pin[16] = { '1', '2', '3', '4' };
constexpr size_t lockout_wait_time = 5000;

struct lock_controller : public tasks::managed_task
{
    enum cmds {
        CMD_Unlock = event_msg::CMD_User,
    };
    lock_controller() :
        managed_task(
            "Lock controller",
            stack,
            sizeof(stack) / sizeof(StackType_t),
            msgQueue,
            sizeof(msgQueue) / sizeof(event_msg)
        )
    {
        tasks::lockControl = handle();
    }

    void unlock()
    {
        post(event_msg { CMD_Unlock, {} });
    }

private:
    void process(event_msg& msg) override
    {
        if (msg.cmd == CMD_Unlock) {
            auto params = solenoid::getParams();
            gpio_set_pin_level(LED_OUT, true);
            gpio_set_pin_level(PWM_EN, true);
            vTaskDelay(params.charge_time);
            gpio_set_pin_level(SOL_TRIG, true);
            vTaskDelay(params.drive_time);
            gpio_set_pin_level(PWM_EN, false);
            vTaskDelay(params.hold_time);
            gpio_set_pin_level(SOL_TRIG, false);
            gpio_set_pin_level(LED_OUT, false);
        }
    }

    StackType_t stack[0x80];
    event_msg msgQueue[2];
} controller;


bool checkPin(const char* guess)
{
    size_t i = 0;
    for (; i < 16 && guess[i] != '\0'; ++i) {
        if (pin[i] != mapToPhoneKeypad(guess[i])) {
            return false;
        }
    }
    
    return (i == 16) || (guess[i] == pin[i]);
}

} /* namespace */

void lock::init()
{
    gpio_set_pin_direction(SOL_TRIG, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(SOL_TRIG, false);
    gpio_set_pin_function(SOL_TRIG, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(PWM_EN, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(PWM_EN, false);
    gpio_set_pin_function(PWM_EN, GPIO_PIN_FUNCTION_OFF);

    gpio_set_pin_direction(LED_OUT, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(LED_OUT, false);
    gpio_set_pin_function(LED_OUT, GPIO_PIN_FUNCTION_OFF);

    if (!nvmem::read(nvmem::pin_id, &pin, 16)) {
        setPin("1234");
    }
}

void lock::unlock(void)
{
    controller.unlock();
}

bool lock::tryUnlock(const char *guess, bool lockout)
{
    if (checkPin(guess)) {
        unlock();
        return true;
    } else if (lockout) {
        vTaskDelay(lockout_wait_time);
    }
    return false;
}

void lock::setPin(const char *newPin)
{
    for (size_t i = 0; i < 16 && newPin[i] != '\0'; ++i) {
        pin[i] = mapToPhoneKeypad(newPin[i]);
    }
    nvmem::write(nvmem::pin_id, pin, 16);
}

bool lock::trySetPin(const char *oldPin, const char *newPin)
{
    if (checkPin(oldPin)) {
        setPin(newPin);
        return true;
    } else {
        vTaskDelay(lockout_wait_time);
    }
    return false;
}
