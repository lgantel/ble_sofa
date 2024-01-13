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
-- File Name: relay.c
-- Description: Control relay through GPIO
--
-- Last update: 2023-07-09
--
-------------------------------------------------------------------------------*/

#include "relay.h"

//----------------------------------------------------------------
// Functions
//----------------------------------------------------------------

/**
 * @file relay.h
 * @name relay_init
 */
void relay_init(relay_t * relay, uint relay_gpio) {
    // Store associated GPIO
    relay->gpio = relay_gpio;
    // Init relay
    gpio_init(relay->gpio);
    gpio_set_dir(relay->gpio, GPIO_OUT);
}

/**
 * @file relay.h
 * @name relay_on
 */
void relay_on(relay_t * relay) {
    gpio_put(relay->gpio, true);
}

/**
 * @file relay.h
 * @name relay_off
 */
void relay_off(relay_t * relay) {
    gpio_put(relay->gpio, false);
}
