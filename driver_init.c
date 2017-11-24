/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"

#include <hal_init.h>
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hal_init.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>

struct flash_descriptor FLASH_0;

struct pwm_descriptor PWM_0;

void FLASH_0_CLOCK_init(void)
{

    _pm_enable_bus_clock(PM_BUS_APBB, NVMCTRL);
}

void FLASH_0_init(void)
{
    FLASH_0_CLOCK_init();
    flash_init(&FLASH_0, NVMCTRL);
}

void PWM_0_PORT_init(void)
{

    gpio_set_pin_function(CPU_PWM, PINMUX_PA04E_TCC0_WO0);
}

void PWM_0_CLOCK_init(void)
{
    _pm_enable_bus_clock(PM_BUS_APBC, TCC0);
    _gclk_enable_channel(TCC0_GCLK_ID, CONF_GCLK_TCC0_SRC);
}

void PWM_0_init(void)
{
    PWM_0_CLOCK_init();
    PWM_0_PORT_init();
    pwm_init(&PWM_0, TCC0, _tcc_get_pwm());
}

void USB_DEVICE_INSTANCE_PORT_init(void)
{

    gpio_set_pin_direction(PA24,
                           // <y> Pin direction
                           // <id> pad_direction
                           // <GPIO_DIRECTION_OFF"> Off
                           // <GPIO_DIRECTION_IN"> In
                           // <GPIO_DIRECTION_OUT"> Out
                           GPIO_DIRECTION_OUT);

    gpio_set_pin_level(PA24,
                       // <y> Initial level
                       // <id> pad_initial_level
                       // <false"> Low
                       // <true"> High
                       false);

    gpio_set_pin_pull_mode(PA24,
                           // <y> Pull configuration
                           // <id> pad_pull_config
                           // <GPIO_PULL_OFF"> Off
                           // <GPIO_PULL_UP"> Pull-up
                           // <GPIO_PULL_DOWN"> Pull-down
                           GPIO_PULL_OFF);

    gpio_set_pin_function(PA24,
                          // <y> Pin function
                          // <id> pad_function
                          // <i> Auto : use driver pinmux if signal is imported by driver, else turn off function
                          // <PINMUX_PA24G_USB_DM"> Auto
                          // <GPIO_PIN_FUNCTION_OFF"> Off
                          // <GPIO_PIN_FUNCTION_A"> A
                          // <GPIO_PIN_FUNCTION_B"> B
                          // <GPIO_PIN_FUNCTION_C"> C
                          // <GPIO_PIN_FUNCTION_D"> D
                          // <GPIO_PIN_FUNCTION_E"> E
                          // <GPIO_PIN_FUNCTION_F"> F
                          // <GPIO_PIN_FUNCTION_G"> G
                          // <GPIO_PIN_FUNCTION_H"> H
                          PINMUX_PA24G_USB_DM);

    gpio_set_pin_direction(PA25,
                           // <y> Pin direction
                           // <id> pad_direction
                           // <GPIO_DIRECTION_OFF"> Off
                           // <GPIO_DIRECTION_IN"> In
                           // <GPIO_DIRECTION_OUT"> Out
                           GPIO_DIRECTION_OUT);

    gpio_set_pin_level(PA25,
                       // <y> Initial level
                       // <id> pad_initial_level
                       // <false"> Low
                       // <true"> High
                       false);

    gpio_set_pin_pull_mode(PA25,
                           // <y> Pull configuration
                           // <id> pad_pull_config
                           // <GPIO_PULL_OFF"> Off
                           // <GPIO_PULL_UP"> Pull-up
                           // <GPIO_PULL_DOWN"> Pull-down
                           GPIO_PULL_OFF);

    gpio_set_pin_function(PA25,
                          // <y> Pin function
                          // <id> pad_function
                          // <i> Auto : use driver pinmux if signal is imported by driver, else turn off function
                          // <PINMUX_PA25G_USB_DP"> Auto
                          // <GPIO_PIN_FUNCTION_OFF"> Off
                          // <GPIO_PIN_FUNCTION_A"> A
                          // <GPIO_PIN_FUNCTION_B"> B
                          // <GPIO_PIN_FUNCTION_C"> C
                          // <GPIO_PIN_FUNCTION_D"> D
                          // <GPIO_PIN_FUNCTION_E"> E
                          // <GPIO_PIN_FUNCTION_F"> F
                          // <GPIO_PIN_FUNCTION_G"> G
                          // <GPIO_PIN_FUNCTION_H"> H
                          PINMUX_PA25G_USB_DP);
}

/* The USB module requires a GCLK_USB of 48 MHz ~ 0.25% clock
 * for low speed and full speed operation. */
#if (CONF_GCLK_USB_FREQUENCY > (48000000 + 48000000 / 400)) || (CONF_GCLK_USB_FREQUENCY < (48000000 - 48000000 / 400))
#warning USB clock should be 48MHz ~ 0.25% clock, check your configuration!
#endif

void USB_DEVICE_INSTANCE_CLOCK_init(void)
{

    _pm_enable_bus_clock(PM_BUS_APBB, USB);
    _pm_enable_bus_clock(PM_BUS_AHB, USB);
    _gclk_enable_channel(USB_GCLK_ID, CONF_GCLK_USB_SRC);
}

void USB_DEVICE_INSTANCE_init(void)
{
    USB_DEVICE_INSTANCE_CLOCK_init();
    usb_d_init();
    USB_DEVICE_INSTANCE_PORT_init();
}

void system_init(void)
{
    init_mcu();
    // GPIO on PA03

    // Set pin direction to output
    gpio_set_pin_direction(SOL_TRIG, GPIO_DIRECTION_OUT);

    gpio_set_pin_level(SOL_TRIG,
                       // <y> Initial level
                       // <id> pad_initial_level
                       // <false"> Low
                       // <true"> High
                       false);

    gpio_set_pin_function(SOL_TRIG, GPIO_PIN_FUNCTION_OFF);

    // GPIO on PA05

    // Set pin direction to output
    gpio_set_pin_direction(PWM_EN, GPIO_DIRECTION_OUT);

    gpio_set_pin_level(PWM_EN,
                       // <y> Initial level
                       // <id> pad_initial_level
                       // <false"> Low
                       // <true"> High
                       false);

    gpio_set_pin_function(PWM_EN, GPIO_PIN_FUNCTION_OFF);

    // GPIO on PA10

    // Set pin direction to output
    gpio_set_pin_direction(LED_OUT, GPIO_DIRECTION_OUT);

    gpio_set_pin_level(LED_OUT,
                       // <y> Initial level
                       // <id> pad_initial_level
                       // <false"> Low
                       // <true"> High
                       false);

    gpio_set_pin_function(LED_OUT, GPIO_PIN_FUNCTION_OFF);

    // GPIO on PA14

    // Set pin direction to output
    gpio_set_pin_direction(KEYPADO0, GPIO_DIRECTION_OUT);

    gpio_set_pin_level(KEYPADO0,
                       // <y> Initial level
                       // <id> pad_initial_level
                       // <false"> Low
                       // <true"> High
                       true);

    gpio_set_pin_function(KEYPADO0, GPIO_PIN_FUNCTION_OFF);

    // GPIO on PA15

    // Set pin direction to output
    gpio_set_pin_direction(KEYPADO1, GPIO_DIRECTION_OUT);

    gpio_set_pin_level(KEYPADO1,
                       // <y> Initial level
                       // <id> pad_initial_level
                       // <false"> Low
                       // <true"> High
                       true);

    gpio_set_pin_function(KEYPADO1, GPIO_PIN_FUNCTION_OFF);

    // GPIO on PA16

    // Set pin direction to output
    gpio_set_pin_direction(KEYPADO2, GPIO_DIRECTION_OUT);

    gpio_set_pin_level(KEYPADO2,
                       // <y> Initial level
                       // <id> pad_initial_level
                       // <false"> Low
                       // <true"> High
                       true);

    gpio_set_pin_function(KEYPADO2, GPIO_PIN_FUNCTION_OFF);

    // GPIO on PA17

    // Set pin direction to input
    gpio_set_pin_direction(KEYPADI0, GPIO_DIRECTION_IN);

    gpio_set_pin_pull_mode(KEYPADI0,
                           // <y> Pull configuration
                           // <id> pad_pull_config
                           // <GPIO_PULL_OFF"> Off
                           // <GPIO_PULL_UP"> Pull-up
                           // <GPIO_PULL_DOWN"> Pull-down
                           GPIO_PULL_UP);

    gpio_set_pin_function(KEYPADI0, GPIO_PIN_FUNCTION_OFF);

    // GPIO on PA18

    // Set pin direction to input
    gpio_set_pin_direction(KEYPADI1, GPIO_DIRECTION_IN);

    gpio_set_pin_pull_mode(KEYPADI1,
                           // <y> Pull configuration
                           // <id> pad_pull_config
                           // <GPIO_PULL_OFF"> Off
                           // <GPIO_PULL_UP"> Pull-up
                           // <GPIO_PULL_DOWN"> Pull-down
                           GPIO_PULL_UP);

    gpio_set_pin_function(KEYPADI1, GPIO_PIN_FUNCTION_OFF);

    // GPIO on PA19

    // Set pin direction to input
    gpio_set_pin_direction(KEYPADI2, GPIO_DIRECTION_IN);

    gpio_set_pin_pull_mode(KEYPADI2,
                           // <y> Pull configuration
                           // <id> pad_pull_config
                           // <GPIO_PULL_OFF"> Off
                           // <GPIO_PULL_UP"> Pull-up
                           // <GPIO_PULL_DOWN"> Pull-down
                           GPIO_PULL_UP);

    gpio_set_pin_function(KEYPADI2, GPIO_PIN_FUNCTION_OFF);

    // GPIO on PA22

    // Set pin direction to input
    gpio_set_pin_direction(KEYPADI3, GPIO_DIRECTION_IN);

    gpio_set_pin_pull_mode(KEYPADI3,
                           // <y> Pull configuration
                           // <id> pad_pull_config
                           // <GPIO_PULL_OFF"> Off
                           // <GPIO_PULL_UP"> Pull-up
                           // <GPIO_PULL_DOWN"> Pull-down
                           GPIO_PULL_UP);

    gpio_set_pin_function(KEYPADI3, GPIO_PIN_FUNCTION_OFF);

    FLASH_0_init();

    PWM_0_init();

    USB_DEVICE_INSTANCE_init();
}
