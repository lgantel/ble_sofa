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
-- Last update: 2023-12-31
--
-------------------------------------------------------------------------------*/

#ifndef SSD1306_H
#define SSD1306_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ascii_bitmap.h"

//----------------------------------------------------------------
// Constants
//----------------------------------------------------------------

/** @brief SSD1306 I2C 7-bit address */
#define SSD1306_I2C_ADDR  0x3C

/** @brief SSD1306 command byte to be sent over I2C bus */
#define SSD1306_I2C_CMD         0x00
/** @brief SSD1306 data byte to be sent over I2C bus */
#define SSD1306_I2C_DATA        0x01

// OLED characteristics
#define OLED_NB_DISPLAY_COL 128   // Number of display columns
#define OLED_NB_DISPLAY_ROW  32   // Number of display rows
#define OLED_NB_DISPLAY_PAGE  4   // Number of display memory pages

// SSD1306 commands
#define OLED_DISP_CONTRAST1          0x81
#define OLED_DISP_CONTRAST2          0x0F
#define OLED_SET_SCAN_DIR       0xC0
#define OLED_SET_LOWER_COL_ADDR  0xDA
#define OLED_LOWER_COL_ADDR     0x00
#define OLED_SET_MEM_ADDR           0x20
#define OLED_SET_DISP_START_LINE    0x40
/** @brief Remap segment column */
#define OLED_SET_SEG_REMAP          0xA0
#define OLED_SET_MUX_RATIO          0xA8
#define OLED_SET_COM_OUT_DIR        0xC0
#define OLED_SET_DISP_OFFSET        0xD3
#define OLED_SET_COM_PIN_CFG        0xDA
#define OLED_SET_DISP_CLK_DIV       0xD5
#define OLED_SET_PRECHARGE          0xD9
#define OLED_SET_VCOM_DESEL         0xDB
#define OLED_SET_CONTRAST           0x81
#define OLED_SET_ENTIRE_ON          0xA4
#define OLED_SET_NORM_INV           0xA6
#define OLED_SET_IREF_SELECT        0xAD
#define OLED_SET_CHARGE_PUMP        0x8D
/** @brief Display on command */
#define OLED_CMD_DISPLAYON          0xAF
/** @brief Display off command */
#define OLED_CMD_DISPLAYOFF         0xAE

//----------------------------------------------------------------
// Types
//----------------------------------------------------------------

/** @brief SSD1306 I2C instance */
typedef struct {
  i2c_inst_t * i2c; /**> Pointer to an I2C instance */
  uin8_t addr;      /**> 7-bit I2C address */
  uint16_t gddram[OLED_NB_DISPLAY_COL * OLED_NB_DISPLAY_ROW]; /**> SSD1306 OLED graphic RAM*/
} ssd1306_i2c_t;

//----------------------------------------------------------------
// Functions
//----------------------------------------------------------------

/**
 * @brief Write a byte to the SSD1306 I2C interface
 * 
 * @param ssd1306 
 * @param byte 
 * @param ncmd 
 * @return int 
 */
int ssd1306_i2c_write_byte(ssd1306_i2c_t * ssd1306, const uint8_t byte, const uint8_t ncmd) {
    uint8_t buf[2] = { 0 };

    // Write data over I2C
    if (ncmd) { buf[0] = 0x40; } // Co=0, D/C#=1
    // Else write command over I2C
    else { buf[0] = 0x80; } // Co=1, D/C#=0

    buf[1] = byte;
    i2c_write_blocking(ssd1306->i2c, ssd1306->addr, buf, 2, false);

    return 0;
}

/**
 * @file oled_control.c
 * @name oled_clear_buffer
 */
int ssd1306_clear_buffer(ssd1306_i2c_t * ssd1306) {
  memset(ssd1306->gddram, 0x00, OLED_NB_DISPLAY_COL * OLED_NB_DISPLAY_PAGE);

  return 0;
}

/**
 * @brief 
 * 
 * @param ssd1306 
 * @return int 
 */
int ssd1306_write_buffer(ssd1306_i2c_t * ssd1306) {
  for (int ipag = 0; ipag < OLED_NB_DISPLAY_PAGE; ipag++) {
	  // Set the page address
	  ssd1306_i2c_write_byte(ssd1306, 0x22, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, ipag, SSD1306_I2C_CMD);

	  // Start at the left column
	  // - Lower nibble of column start address: 0x0<4 bits nibble>
    ssd1306_i2c_write_byte(ssd1306, 0x00, SSD1306_I2C_CMD);
	  // - Higher nibble of column start address: 0x1<4 bits nibble>
    ssd1306_i2c_write_byte(ssd1306, 0x10, SSD1306_I2C_CMD);

	  // Copy this memory page of display data (horizontal increment)
	  for (int col = 0; col < OLED_NB_DISPLAY_COL; col++) {
      ssd1306_i2c_write_byte(ssd1306, gddram[col + ipag*OLED_NB_DISPLAY_COL], SSD1306_I2C_DATA);
	  }
  }

  return 0;
}

/**
 * @brief 
 * 
 * @param ssd1306 
 * @return int 
 */
int ssd1306_clear_screen(ssd1306_i2c_t * ssd1306) {
    // Clear the internal buffer
	oled_clear_buffer();

    // Write the buffer into the graphic memory
	oled_write_buffer(ssd1306);

    return 0;
}

#endif // SSD1306_H
