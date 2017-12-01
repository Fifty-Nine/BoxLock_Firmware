#include "pwm.h"

#include <hal_pwm.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>
#include <hpl_tcc.h>

#include "pins.h"

static struct pwm_descriptor pwm_inst;

void pwm::init()
{
    gpio_set_pin_direction(CPU_PWM, GPIO_DIRECTION_OUT);
    gpio_set_pin_function(CPU_PWM, GPIO_PIN_FUNCTION_E);
    _pm_enable_bus_clock(PM_BUS_APBC, TCC0);
    _gclk_enable_channel(TCC0_GCLK_ID, GCLK_CLKCTRL_GEN_GCLK0_Val);
    pwm_init(&pwm_inst, TCC0, _tcc_get_pwm());

    gpio_set_pin_direction(PWM_EN, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(PWM_EN, false);
    gpio_set_pin_function(PWM_EN, GPIO_PIN_FUNCTION_OFF);
}

void pwm::enable(uint32_t period, uint32_t duty)
{
    gpio_set_pin_level(PWM_EN, true);
    pwm_set_parameters(&pwm_inst, period, duty);
    pwm_enable(&pwm_inst);
}

void pwm::disable()
{
    gpio_set_pin_level(PWM_EN, false);
    pwm_disable(&pwm_inst);
}
