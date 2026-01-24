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
-- Project Name: BLE Sofa Application
-- Version: 0.1.0
-- File Name: hw_mgmt.h
-- Description: Hardware Management
--
-- Last update: 2025-12-17
--
-------------------------------------------------------------------------------*/

#ifndef HW_MGMT_H
#define HW_MGMT_H

#include "relay.h"
#include "ssd1306.h"

//------------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------------

// Relay pins
#define RELAY1_GPIO       7
#define RELAY2_GPIO       6

// OLED Display I2C pins
#define I2C_SDA_GPIO      16
#define I2C_SCL_GPIO      17

// SSD1306 I2C 7-bit address
#define SSD1306_I2C_ADDR  0x3C

//------------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------------

/**
 * @brief Initialize the hardware components
 */
void hw_init(void);

/**
 * @brief Clean up the hardware components
 */
void hw_deinit(void);

#endif // HW_MGMT_H
