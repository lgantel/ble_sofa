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
-- Last update: 2023-12-29
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

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
static const uint8_t SSD1306_I2C_ADDR = 0x3C;

#define OLED_DISP_CONTRAST1          0x81
#define OLED_DISP_CONTRAST2          0x0F
#define OLED_SET_SCAN_DIR       0xC0
#define OLED_SET_LOWER_COL_ADDR  0xDA
#define OLED_LOWER_COL_ADDR     0x00

#define OLED_SET_MEM_ADDR           0x20
#define OLED_SET_DISP_START_LINE    0x40
#define OLED_SET_SEG_REMAP          0xA0 // Remap segment column
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


#define OLED_NB_DISPLAY_COL 128   // Number of display columns
#define OLED_NB_DISPLAY_ROW  32   // Number of display rows
#define OLED_NB_DISPLAY_PAGE  4   // Number of display memory pages

#define OLED_CMD_DISPLAYON       0xAF // Display on command
#define OLED_CMD_DISPLAYOFF      0xAE // Display off command

//----------------------------------------------------------------
// Global variables
//----------------------------------------------------------------

uint16_t gddram[OLED_NB_DISPLAY_COL * OLED_NB_DISPLAY_ROW];

//----------------------------------------------------------------
// Static Functions
//----------------------------------------------------------------

static int reg_write(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t * buf, const uint8_t nbytes) {
    uint8_t msg[nbytes + 1];

    // Check to make sure caller is sending 1 or more bytes
    if (nbytes < 1) { return -1; }

    // Append register address to front of data packet
    msg[0] = reg;
    for (int i = 0; i < nbytes; i++) {
        msg[i + 1] = buf[i];
    }

    // Write data to register(s) over I2C
    i2c_write_blocking(i2c, addr, msg, (nbytes + 1), false);

    return 0;
}

//----------------------------------------------------------------
// OLED function
//----------------------------------------------------------------

/**
 * @brief 
 * 
 * @param i2c 
 * @param addr 
 * @param byte 
 * @param ncmd  Not Command - Set to I2C_CMD (0x00) to send a command, I2C_DATA (0x01) to send data
 * @return int 
 */
int oled_write_byte(i2c_inst_t *i2c, const uint addr, const uint8_t byte, const uint8_t ncmd) {
    uint8_t buf[2] = { 0 };

    if (ncmd) {
        // Write data over I2C
        buf[0] = 0x40;  // Co=0, D/C#=1
        buf[1] = byte;
        i2c_write_blocking(i2c, addr, buf, 2, false);
    }
    else {
        // Write command over I2C
        buf[0] = 0x80;  // Co=1, D/C#=0
        buf[1] = byte;
        i2c_write_blocking(i2c, addr, buf, 2, false);
    }

    return 0;
}

/**
 * @file oled_control.c
 * @name oled_clear_buffer
 */
int oled_clear_buffer(void) {
  memset(gddram, 0x00, OLED_NB_DISPLAY_COL * OLED_NB_DISPLAY_PAGE);

  return 0;
}

/**
 * @file oled_control.c
 * @name oled_write_buffer
 */
int oled_write_buffer(i2c_inst_t *i2c, const uint addr) {
  for (int ipag = 0; ipag < OLED_NB_DISPLAY_PAGE; ipag++) {
	  // Set the page address
	  oled_write_byte(i2c, addr, 0x22, I2C_CMD);
    oled_write_byte(i2c, addr, ipag, I2C_CMD);

	  // Start at the left column
	  // - Lower nibble of column start address: 0x0<4 bits nibble>
    oled_write_byte(i2c, addr, 0x00, I2C_CMD);
	  // - Higher nibble of column start address: 0x1<4 bits nibble>
    oled_write_byte(i2c, addr, 0x10, I2C_CMD);

	  // Copy this memory page of display data (horizontal increment)
	  for (int col = 0; col < OLED_NB_DISPLAY_COL; col++) {
      oled_write_byte(i2c, addr, gddram[col + ipag*OLED_NB_DISPLAY_COL], I2C_DATA);
	  }
  }

  return 0;
}

int oled_clear_screen(i2c_inst_t *i2c, const uint addr) {
    // Clear the internal buffer
	oled_clear_buffer();

    // Write the buffer into the graphic memory
	oled_write_buffer(i2c, addr);

    return 0;
}

int oled_poweron_init(i2c_inst_t *i2c, const uint addr) {
    // Wait for at least 1ms for reset to complete -> 2ms
    sleep_ms(2);

    // Set Display Off
    oled_write_byte(i2c, addr, OLED_CMD_DISPLAYOFF, I2C_CMD);
    // Wait for at least 2*3us for reset to operate -> 6ms to be sure 
    sleep_ms(6);

    // Charge pump and Pre-Charge period
    oled_write_byte(i2c, addr, OLED_SET_CHARGE_PUMP, I2C_CMD);
    oled_write_byte(i2c, addr, 0x14, I2C_CMD);
    oled_write_byte(i2c, addr, 0xD9, I2C_CMD);
    oled_write_byte(i2c, addr, 0xF1, I2C_CMD);

    // Wait for 100 ms for VBAT stabilization
    sleep_ms(100);

    // Set memory address
    oled_write_byte(i2c, addr, OLED_CMD_DISPLAYOFF, I2C_CMD);
    oled_write_byte(i2c, addr, 0x00, I2C_CMD);
    // Set resolution and layout
    oled_write_byte(i2c, addr, OLED_SET_DISP_START_LINE | 0x00, I2C_CMD);
    oled_write_byte(i2c, addr, OLED_SET_SEG_REMAP | 0x01, I2C_CMD); // Map column 127 to SEG0
    oled_write_byte(i2c, addr, OLED_SET_MUX_RATIO, I2C_CMD);
    oled_write_byte(i2c, addr, 31, I2C_CMD);    // Height - 1
    oled_write_byte(i2c, addr, OLED_SET_COM_OUT_DIR | 0x08, I2C_CMD);
    oled_write_byte(i2c, addr, OLED_SET_DISP_OFFSET, I2C_CMD);
    oled_write_byte(i2c, addr, 0x00, I2C_CMD);
    oled_write_byte(i2c, addr, OLED_SET_COM_PIN_CFG, I2C_CMD);
    oled_write_byte(i2c, addr, 0x02, I2C_CMD);
    // Timing and driving scheme
    oled_write_byte(i2c, addr, OLED_SET_DISP_CLK_DIV, I2C_CMD);
    oled_write_byte(i2c, addr, 0x80, I2C_CMD);
    oled_write_byte(i2c, addr, OLED_SET_PRECHARGE, I2C_CMD);
    oled_write_byte(i2c, addr, 0xF1, I2C_CMD);
    oled_write_byte(i2c, addr, OLED_SET_VCOM_DESEL, I2C_CMD);
    oled_write_byte(i2c, addr, 0x30, I2C_CMD);  // 0.83 * VCC
    // Display
    oled_write_byte(i2c, addr, OLED_SET_CONTRAST, I2C_CMD);
    oled_write_byte(i2c, addr, 0xFF, I2C_CMD);  // Maximum
    oled_write_byte(i2c, addr, OLED_SET_ENTIRE_ON, I2C_CMD); // Output follows RAM contents
    oled_write_byte(i2c, addr, OLED_SET_NORM_INV, I2C_CMD);
    oled_write_byte(i2c, addr, OLED_SET_IREF_SELECT, I2C_CMD);
    oled_write_byte(i2c, addr, 0x30, I2C_CMD);  // Enable internal IREF during display on

    // Send Display On command
    oled_write_byte(i2c, addr, OLED_CMD_DISPLAYON, I2C_CMD);

    return 0;
}

int oled_poweron(i2c_inst_t *i2c, const uint addr) {
    // Initialize the OLED Controller
    oled_poweron_init(i2c, addr);

    // Clear screen
    oled_clear_screen(i2c, addr);

    return 0;
}

int oled_poweroff(i2c_inst_t *i2c, const uint addr) {
    // Send Display Off command
    oled_write_byte(i2c, addr, OLED_CMD_DISPLAYOFF, I2C_CMD);

    return 0;
}

/**
 * @file oled_control.c
 * @name oled_set_pixel
 */
int oled_set_pixel(uint32_t row, uint32_t col, uint16_t val) {
  int page = row / 8;
  uint8_t col_content = gddram[col + page*OLED_NB_DISPLAY_COL];
  int pixel_pos = row - 8*page;

  if (val) { col_content |= (0x01 << pixel_pos); }
  else { col_content &= ~(0x01 << pixel_pos);	}

  gddram[col + page*OLED_NB_DISPLAY_COL] = col_content;

  return 0;
}

/**
 * @file oled_control.c
 * @name oled_set_pagecol
 */
int oled_set_pagecol(uint32_t page, uint32_t col, uint8_t col_content) {
  if ((page > OLED_NB_DISPLAY_PAGE) || (col > OLED_NB_DISPLAY_COL)) {
	  return -1;
  }

  gddram[col + page*OLED_NB_DISPLAY_COL] = col_content;
  return 0;
}

/**
 * @file oled_control.c
 * @name oled_write_letter
 */
int oled_write_letter(int8_t letter, unsigned int page, unsigned int col) {
  int status;

  bitmap_char_t ascii_letter = ascii_bitmap_lut[letter];
  for (int i=0; i<8; i++) {
	status = oled_set_pagecol(page, col+i, ascii_letter.col[i]);
	if (status != 0) { return status; }
  }

  return 0;
}

/**
 * @file oled_control.c
 * @name oled_write_str
 */
int oled_write_str(i2c_inst_t *i2c, const uint addr, const char * str, unsigned int page) {
	if ((str == NULL) || (page > OLED_NB_DISPLAY_PAGE)) { return -1; }

  for (int i=0; i<OLED_NB_DISPLAY_COL/8; i++) {
	if (str[i] == '\0') { break; }
	oled_write_letter(str[i], page, i*8);
  }
  oled_write_buffer(i2c, addr);

  return 0;
}

/**
 * @brief Main entry point
 * @return int Endless loop, never returns
 */
int main(void)
{
    // Debug LED
    bool led_val = false;

    // I2C 
    i2c_inst_t *i2c = i2c0;

    // STDIO
    stdio_init_all();
    sleep_ms(1000);

    printf("-- OLED Control --\n");

    // Initialize the LED output
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);

    // Initialize I2C port at 100 kHz
    i2c_init(i2c, 100 * 1000);

    // Initialize I2C pins
    gpio_set_function(I2C_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_GPIO, GPIO_FUNC_I2C);

    oled_poweron(i2c, SSD1306_I2C_ADDR);
    gpio_put(LED_GPIO, true);

    oled_write_str(i2c, SSD1306_I2C_ADDR, "ABCDEFGHIJKLMNOP", 0);
    oled_write_str(i2c, SSD1306_I2C_ADDR, "QRSTUVWXYZ012345", 1);
    oled_write_str(i2c, SSD1306_I2C_ADDR, "6789abcdefghijkl", 2);
    oled_write_str(i2c, SSD1306_I2C_ADDR, "mnopqrstuvwxyz\0", 3);

    sleep_ms(5000);

    oled_poweroff(i2c, SSD1306_I2C_ADDR);
    gpio_put(LED_GPIO, false);

    while(true) {
        // Heartbeat for debug purpose
        sleep_ms(500);
        led_val = !led_val;
        gpio_put(LED_GPIO, led_val);
    }

    return 0;
}
