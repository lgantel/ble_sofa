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
-- File Name: relay.h
-- Description: Control relay through GPIO
--
-- Last update: 2023-07-09
--
-------------------------------------------------------------------------------*/

#include "hardware/gpio.h"

//----------------------------------------------------------------
// Types
//----------------------------------------------------------------

typedef struct {
    uint gpio;  /**> GPIO associated to the relay */
} relay_t;

//----------------------------------------------------------------
// Functions
//----------------------------------------------------------------

/**
 * @brief Initialze the relay
 * 
 * @param relay The relay structure
 * @param relay_gpio The GPIO associated to the relay
 */
void relay_init(relay_t * relay, uint relay_gpio);

/**
 * @brief Set the relay ON
 * 
 * @param relay The relay structure
 */
void relay_on(relay_t * relay);

/**
 * @brief Set the relay OFF
 * 
 * @param relay The relay structure
 */
void relay_off(relay_t * relay);
