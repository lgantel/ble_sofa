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
-- Last update: 2024-01-01
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "ssd1306.h"

//----------------------------------------------------------------
// Constants
//----------------------------------------------------------------

#define LED_GPIO        2
#define I2C_SDA_GPIO    16
#define I2C_SCL_GPIO    17

#define I2C_CMD         0x00
#define I2C_DATA        0x01

// SSD1306 I2C 7-bit address
#define SSD1306_I2C_ADDR 0x3C

//----------------------------------------------------------------
// Funtions
//----------------------------------------------------------------

/**
 * @brief Main entry point
 * @return int Endless loop, never returns
 */
int main(void)
{
    // Debug LED
    bool led_val = false;

    // OLED display
    ssd1306_t ssd1306;

    // STDIO
    stdio_init_all();
    sleep_ms(1000);

    printf("-- OLED Control --\n");

    // Initialize the LED output
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);

    // Initialize the OLED display
    ssd1306_i2c_init(&ssd1306, 
      i2c0, 
      SSD1306_I2C_ADDR, 
      100 * 1000, // 100 kHz
      I2C_SCL_GPIO,
      I2C_SDA_GPIO
    );

    gpio_put(LED_GPIO, true);
    ssd1306_poweron(&ssd1306);
    sleep_ms(1000);

    ssd1306_write_str(&ssd1306, "ABCDEFGHIJKLMNOP", 0);
    ssd1306_write_str(&ssd1306, "QRSTUVWXYZ012345", 1);
    ssd1306_write_str(&ssd1306, "6789abcdefghijkl", 2);
    ssd1306_write_str(&ssd1306, "mnopqrstuvwxyz\0", 3);
    sleep_ms(5000);

    ssd1306_poweroff(&ssd1306);
    gpio_put(LED_GPIO, false);

    while(true) {
        // Heartbeat for debug purpose
        sleep_ms(500);
        led_val = !led_val;
        gpio_put(LED_GPIO, led_val);
    }

    return 0;
}
