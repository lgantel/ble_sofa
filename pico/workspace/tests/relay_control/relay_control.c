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
-- File Name: relay_control.c
-- Description: Pico Dual Channel Relay Hat tests: 
--              Control 12V fan through relay 1
--
-- Last update: 2023-09-07
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include "pico/stdlib.h"

#include "relay.h"

//----------------------------------------------------------------
// Constants
//----------------------------------------------------------------

#define RELAY1_GPIO 6

//----------------------------------------------------------------
// Functions
//----------------------------------------------------------------

/**
 * @brief Main entry point
 * @return int Endless loop, never returns
 */
int main(void)
{
    relay_t relay1;

    // Initialize the relay output
    relay_init(&relay1, RELAY1_GPIO);

    // Turn off relay 1
    relay_off(&relay1);

    // Wait a moment
    sleep_ms(2000);

    while(true) {
        // Turn on relay 1
        relay_on(&relay1);
        // Run for 5 seconds
        sleep_ms(5000);
        // Turn off relay 1
        relay_off(&relay1);
        // Wait for 3 seconds
        sleep_ms(3000);
    }

    return 0;
}
