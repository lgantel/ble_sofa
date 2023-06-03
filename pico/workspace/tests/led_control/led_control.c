/*--------------------------------------------------------------------------------  
--                          _               _       _ 
--                         | |__ _ __ _ _ _| |_ ___| |
--                         | / _` / _` | ' \  _/ -_) |
--                         |_\__, \__,_|_||_\__\___|_|
--                           |___/                                        
--
----------------------------------------------------------------------------------
--
-- Company: LGANTEL
-- Engineer: Laurent Gantel <laurent.gantel@gmail.com>
--
-- Project Name: RPi Pico Tests
-- Version: 0.1.0
-- File Name: led_control.c
-- Description: GPIO tests: leds and button control
--
-- Last update: 2023-06-02
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_TOGGLE_GPIO    2
#define LED_BUTTON_GPIO    3
#define BUTTON_GPIO        4

int main()
{
    bool led_toggle_val = false;
    bool led_button_val = false;

    stdio_init_all();
    printf("-- LED Control --\n");

    // Initialize the LED outputs
    gpio_init(LED_BUTTON_GPIO);
    gpio_set_dir(LED_BUTTON_GPIO, GPIO_OUT);
    gpio_init(LED_TOGGLE_GPIO);
    gpio_set_dir(LED_TOGGLE_GPIO, GPIO_OUT);

    // Initialize the button input
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);

    while(true) {
        // Button is active low
        if (!gpio_get(BUTTON_GPIO)) {
            gpio_put(LED_BUTTON_GPIO, led_button_val);
            led_button_val = !led_button_val;
        }
        sleep_ms(500);
        gpio_put(LED_TOGGLE_GPIO, led_toggle_val);
        led_toggle_val = !led_toggle_val;
    }

    return 0;
}
