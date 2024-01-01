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
-- File Name: ssd1306.h
-- Description: API to control the 128×32 I2C OLED LCD Display including a SSD1306 chip
--
-- Last update: 2024-01-01
--
-------------------------------------------------------------------------------*/

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "ascii_bitmap.h"

//----------------------------------------------------------------
// Constants
//----------------------------------------------------------------

/** @brief SSD1306 command byte to be sent over I2C bus */
#define SSD1306_I2C_CMD         0x00
/** @brief SSD1306 data byte to be sent over I2C bus */
#define SSD1306_I2C_DATA        0x01

// OLED characteristics
#define SSD1306_NB_DISPLAY_COL 128   // Number of display columns
#define SSD1306_NB_DISPLAY_ROW  32   // Number of display rows
#define SSD1306_NB_DISPLAY_PAGE  4   // Number of display memory pages

// SSD1306 commands
#define SSD1306_DISP_CONTRAST1          0x81
#define SSD1306_DISP_CONTRAST2          0x0F
#define SSD1306_SET_SCAN_DIR       0xC0
#define SSD1306_SET_LOWER_COL_ADDR  0xDA
#define SSD1306_LOWER_COL_ADDR     0x00
#define SSD1306_SET_MEM_ADDR           0x20
#define SSD1306_SET_DISP_START_LINE    0x40
/** @brief Remap segment column */
#define SSD1306_SET_SEG_REMAP          0xA0
#define SSD1306_SET_MUX_RATIO          0xA8
#define SSD1306_SET_COM_OUT_DIR        0xC0
#define SSD1306_SET_DISP_OFFSET        0xD3
#define SSD1306_SET_COM_PIN_CFG        0xDA
#define SSD1306_SET_DISP_CLK_DIV       0xD5
#define SSD1306_SET_PRECHARGE          0xD9
#define SSD1306_SET_VCOM_DESEL         0xDB
#define SSD1306_SET_CONTRAST           0x81
#define SSD1306_SET_ENTIRE_ON          0xA4
#define SSD1306_SET_NORM_INV           0xA6
#define SSD1306_SET_IREF_SELECT        0xAD
#define SSD1306_SET_CHARGE_PUMP        0x8D
/** @brief Display on command */
#define SSD1306_CMD_DISPLAYON          0xAF
/** @brief Display off command */
#define SSD1306_CMD_DISPLAYOFF         0xAE

//----------------------------------------------------------------
// Types
//----------------------------------------------------------------

/** @brief SSD1306 I2C configuration */
typedef struct {
  i2c_inst_t * inst;  /**> Pointer to an I2C instance */
  uint8_t addr;       /**> 7-bit I2C address */
  uint32_t baudrate;  /**> I2C baudrate in Hz */
  uint8_t scl_pin;    /**> I2C SCL pin */
  uint8_t sda_pin;    /**> I2C SDA pin */
} ssd1306_i2c_t;

/** @brief SSD1306 instance */
typedef struct {
  uint8_t gddram[SSD1306_NB_DISPLAY_COL * SSD1306_NB_DISPLAY_PAGE]; /**> SSD1306 OLED graphic RAM*/
  ssd1306_i2c_t i2c;    /**> I2C configuration */
} ssd1306_t;

//----------------------------------------------------------------
// Functions
//----------------------------------------------------------------

/**
 * @brief Initialize a ssd1306_t instance
 */

/**
 * @brief Initialize a ssd1306_t instance
 * @param ssd1306 A pointer to the ssd1306_t instance
 * @param i2c The I2C channel (i2c0 or i2c1)
 * @param i2c_addr The 7-bit I2C address of the instance
 * @param baudrate The I2C baudrate
 * @param scl_pin The GPIO pin used to output the I2C clock
 * @param sda_pin The GPIO pin used to output the I2C data
 */
void ssd1306_i2c_init(ssd1306_t * ssd1306, 
    i2c_inst_t * i2c_inst,
    const uint8_t i2c_addr,
    uint32_t i2c_baudrate,
    uint8_t i2c_scl_pin,
    uint8_t i2c_sda_pin
);
/**
 * @brief Write a byte to the SSD1306 I2C interface
 * 
 * @param ssd1306 A pointer to a ssd1306_t structure containing information about the I2C peripheral
 * @param byte The byte to be written
 * @param dnc D/C: dnc = 1: Data, dnc = 0: Command 
 * @return int The number of bytes written, or PICO_ERROR_GENERIC if address not acknowledged, no device present.
 */
int ssd1306_i2c_write_byte(ssd1306_t * ssd1306, const uint8_t byte, const uint8_t dnc);

/**
 * @brief Clear the buffer used to store the content to be displayed
 * @param ssd1306 A pointer to a ssd1306_t structure containing information about the I2C peripheral
 * @return int Always 0
 */
int ssd1306_clear_buffer(ssd1306_t * ssd1306);

/**
 * @brief Write the content of the internal buffer to the display memory
 * @param ssd1306  A pointer to the ssd1306_t instance
 * @return int Always 0
 */
int ssd1306_write_buffer(ssd1306_t * ssd1306);

/**
 * @brief Clear the display screen - clear the buffer and write it
 * @param ssd1306 A pointer to the ssd1306_t instance
 * @return int Always 0
 */
int ssd1306_clear_screen(ssd1306_t * ssd1306);

/**
 * @brief Power-on initialization sequence
 * @param ssd1306 A pointer to the ssd1306_t instance
 * @return int Always 0
 */
int ssd1306_poweron_init(ssd1306_t * ssd1306);

/**
 * @brief Power-on the OLED display
 * @param ssd1306 A pointer to the ssd1306_t instance
 * @return int Always 0
 */
int ssd1306_poweron(ssd1306_t * ssd1306);

/**
 * @brief Power-off the OLED display
 * @param ssd1306 A pointer to the ssd1306_t instance
 * @return int Always 0
 */
int ssd1306_poweroff(ssd1306_t * ssd1306);

/**
 * @brief Set a pixel value into the content buffer
 * @param ssd1306 A pointer to the ssd1306_t instance
 * @param row The pixel row
 * @param col The pixel column
 * @param val The pixel value (0 or 1)
 * @return int Always 0
 */
int ssd1306_set_pixel(ssd1306_t * ssd1306, uint32_t row, uint32_t col, uint16_t val);

/**
 * @brief 
 * @param ssd1306 
 * @param page 
 * @param col 
 * @param col_content 
 * @return int 
 */
int ssd1306_set_pagecol(ssd1306_t * ssd1306, uint32_t page, uint32_t col, uint8_t col_content);

/**
 * @brief 
 * @param ssd1306 
 * @param letter 
 * @param page 
 * @param col 
 * @return int 
 */
int ssd1306_write_letter(ssd1306_t * ssd1306, int8_t letter, unsigned int page, unsigned int col);

/**
 * @brief 
 * @param ssd1306 
 * @param str 
 * @param page 
 * @return int 
 */
int ssd1306_write_str(ssd1306_t * ssd1306, const char * str, unsigned int page);

#endif // SSD1306_H
