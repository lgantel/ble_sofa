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
-- Project Name: OLED Screen Test Application
-- Version: 0.1.0
-- File Name: oled_control.c
-- Description: Control of the 128×32 I2C OLED LCD Display including SSD1306 chip
--
-- Last update: 2023-12-28
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

//----------------------------------------------------------------
// Constants
//----------------------------------------------------------------

#define LED_GPIO        2
#define I2C_SDA_PIN    16
#define I2C_SCL_PIN    17

// SSD1306 I2C address
static const uint8_t SSD1306_I2C_ADDR = 0x3C;

//----------------------------------------------------------------
// Functions
//----------------------------------------------------------------

/**
 * @brief Main entry point
 * @return int Endless loop, never returns
 */
int main(void)
{
    bool led_val = false;

    stdio_init_all();
    printf("-- OLED Control --\n");

    // Initialize the LED output
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);

    while(true) {
        sleep_ms(500);
        gpio_put(LED_GPIO, led_val);
        led_val = !led_val;
    }

    return 0;
}
