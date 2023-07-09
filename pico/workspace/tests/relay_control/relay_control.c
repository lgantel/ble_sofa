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
--              Control 12V fans through relays
--
-- Last update: 2023-07-09
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include "pico/stdlib.h"

#include "relay.h"

//----------------------------------------------------------------
// Constants
//----------------------------------------------------------------

#define RELAY1_GPIO 6
#define RELAY2_GPIO 7

//----------------------------------------------------------------
// Functions
//----------------------------------------------------------------

/**
 * @brief Main entry point
 * @return int Endless loop, never returns
 */
int main(void)
{
    relay_t relay1, relay2;

    stdio_init_all();
    sleep_ms(1000);

    printf("-- Relay Control --\n");

    // Initialize the relays outputs
    relay_init(&relay1, RELAY1_GPIO);
    relay_init(&relay2, RELAY2_GPIO);

    // Turn off relays
    relay_off(&relay1);
    relay_off(&relay2);

    // Wait a moment
    sleep_ms(2000);

    while(true) {
        // Turn on relay 1
        relay_on(&relay1);
        // Run for 5 seconds
        sleep_ms(5000);
        // Turn off relay 1
        relay_off(&relay1);

        // Wait for 500 ms
        sleep_ms(500);

        // Turn on relay 2
        relay_on(&relay2);
        // Run for 5 seconds
        sleep_ms(5000);
        // Turn off relay 2
        relay_off(&relay2);

        // print tick for debug purpose
        printf(".");

        // Wait for 2 seconds
        sleep_ms(2000);
    }

    return 0;
}
