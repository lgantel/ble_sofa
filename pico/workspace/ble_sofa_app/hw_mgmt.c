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
-- File Name: hw_mgmt.c
-- Description: Hardware Management
--
-- Last update: 2025-12-17
--
-------------------------------------------------------------------------------*/

#include "hw_mgmt.h"

/** @brief Structure to control Relay1 */
relay_t relay1;

/** @brief Structure to control Relay2 */
relay_t relay2;

/** @brief Structure to control SSD1306 OLED display */
ssd1306_t ssd1306;

/**
 * @brief Initialize the relays
 */
static void relays_init(void) {
  // Initialize the relays output
  relay_init(&relay1, RELAY1_GPIO);
  relay_init(&relay2, RELAY2_GPIO);

  // Turn off relay 1
  relay_off(&relay1);
  // Turn off relay 2
  relay_off(&relay2); 
}

/**
 * @brief Clean up the relays
 */
static void relays_deinit(void) {
  // Turn off relay 1
  relay_off(&relay1);
  // Turn off relay 2
  relay_off(&relay2); 
}

/**
 * @brief Initialize the OLED display
 */
static void oled_display_init(void) {
  // Initialize the OLED display
  ssd1306_i2c_init(&ssd1306, 
    i2c0, 
    SSD1306_I2C_ADDR, 
    100 * 1000, // 100 kHz
    I2C_SCL_GPIO,
    I2C_SDA_GPIO
  );
}

/**
 * @brief Clean up the OLED display
 */
static void oled_display_deinit(void) {
  // Power-off OLED display
  ssd1306_poweroff(&ssd1306);
}

/**
 * @brief Initialize the hardware components
 */
void hw_init(void) {
  // Initialize the relays
  relays_init();
  
  // Initialize the OLED display
  oled_display_init();
  // Power-on OLED display
  ssd1306_poweron(&ssd1306);
  ssd1306_write_str(&ssd1306, "--  Power-On  --", 0);
}

/**
 * @brief Clean up the hardware components
 */
void hw_deinit(void) {
  // Clean up the relays
  relays_deinit();
  
  // Clean up the OLED display
  oled_display_deinit();
}
