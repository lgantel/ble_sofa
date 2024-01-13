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
-- File Name: ssd1306.c
-- Description: API to control the 128×32 I2C OLED LCD Display including a SSD1306 chip
--
-- Last update: 2024-01-01
--
-------------------------------------------------------------------------------*/

#include "ssd1306.h"

/**
 * @file ssd1306.h
 * @name ssd1306_i2c_init
 */
void ssd1306_i2c_init(ssd1306_t * ssd1306, 
    i2c_inst_t * i2c_inst,
    const uint8_t i2c_addr,
    uint32_t i2c_baudrate,
    uint8_t i2c_scl_pin,
    uint8_t i2c_sda_pin
) {
    ssd1306->i2c.inst = i2c_inst;
    ssd1306->i2c.addr = i2c_addr;
    ssd1306->i2c.baudrate = i2c_baudrate;
    ssd1306->i2c.scl_pin = i2c_scl_pin;
    ssd1306->i2c.sda_pin = i2c_sda_pin;

    // Initialize I2C port
    i2c_init(ssd1306->i2c.inst, ssd1306->i2c.baudrate);

    // Initialize I2C pins
    gpio_set_function(ssd1306->i2c.scl_pin, GPIO_FUNC_I2C);
    gpio_set_function(ssd1306->i2c.sda_pin, GPIO_FUNC_I2C);
}

/**
 * @file ssd1306.h
 * @name ssd1306_i2c_write_byte
 */
int ssd1306_i2c_write_byte(ssd1306_t * ssd1306, const uint8_t byte, const uint8_t dnc) {
    uint8_t buf[2] = { 0 };

    // Write data over I2C
    if (dnc) { buf[0] = 0x40; } // Co=0, D/C#=1
    // Else write command over I2C
    else { buf[0] = 0x80; } // Co=1, D/C#=0

    buf[1] = byte;
    return i2c_write_blocking(ssd1306->i2c.inst, ssd1306->i2c.addr, buf, 2, false);
}

/**
 * @file ssd1306.h
 * @name ssd1306_clear_buffer
 */
int ssd1306_clear_buffer(ssd1306_t * ssd1306) {
  memset(ssd1306->gddram, 0x00, SSD1306_NB_DISPLAY_COL * SSD1306_NB_DISPLAY_PAGE * sizeof(uint8_t));

  return 0;
}

/**
 * @file ssd1306.h
 * @name ssd1306_write_buffer
 */
int ssd1306_write_buffer(ssd1306_t * ssd1306) {
  for (int ipag = 0; ipag < SSD1306_NB_DISPLAY_PAGE; ipag++) {
	  // Set the page address
	  ssd1306_i2c_write_byte(ssd1306, 0x22, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, ipag, SSD1306_I2C_CMD);

	  // Start at the left column
	  // - Lower nibble of column start address: 0x0<4 bits nibble>
    ssd1306_i2c_write_byte(ssd1306, 0x00, SSD1306_I2C_CMD);
	  // - Higher nibble of column start address: 0x1<4 bits nibble>
    ssd1306_i2c_write_byte(ssd1306, 0x10, SSD1306_I2C_CMD);

	  // Copy this memory page of display data (horizontal increment)
	  for (int col = 0; col < SSD1306_NB_DISPLAY_COL; col++) {
        ssd1306_i2c_write_byte(ssd1306, ssd1306->gddram[col + ipag*SSD1306_NB_DISPLAY_COL], SSD1306_I2C_DATA);
	  }
  }

  return 0;
}

/**
 * @file ssd1306.h
 * @name ssd1306_clear_screen
 */
int ssd1306_clear_screen(ssd1306_t * ssd1306) {
  // Clear the internal buffer
	ssd1306_clear_buffer(ssd1306);

  // Write the buffer into the graphic memory
	ssd1306_write_buffer(ssd1306);

  return 0;
}

/**
 * @file ssd1306.h
 * @name ssd1306_poweron_init
 */
int ssd1306_poweron_init(ssd1306_t * ssd1306) {
    // Wait for at least 1ms for reset to complete -> 2ms
    sleep_ms(2);

    // Set Display Off
    ssd1306_i2c_write_byte(ssd1306, SSD1306_CMD_DISPLAYOFF, SSD1306_I2C_CMD);
    // Wait for at least 2*3us for reset to operate -> 6ms to be sure 
    sleep_ms(6);

    // Charge pump and Pre-Charge period
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_CHARGE_PUMP, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0x14, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0xD9, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0xF1, SSD1306_I2C_CMD);

    // Wait for 100 ms for VBAT stabilization
    sleep_ms(100);

    // Set memory address
    ssd1306_i2c_write_byte(ssd1306, SSD1306_CMD_DISPLAYOFF, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0x00, SSD1306_I2C_CMD);
    // Set resolution and layout
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_DISP_START_LINE | 0x00, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_SEG_REMAP | 0x01, SSD1306_I2C_CMD); // Map column 127 to SEG0
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_MUX_RATIO, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 31, SSD1306_I2C_CMD);    // Height - 1
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_COM_OUT_DIR | 0x08, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_DISP_OFFSET, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0x00, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_COM_PIN_CFG, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0x02, SSD1306_I2C_CMD);
    // Timing and driving scheme
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_DISP_CLK_DIV, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0x80, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_PRECHARGE, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0xF1, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_VCOM_DESEL, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0x30, SSD1306_I2C_CMD);  // 0.83 * VCC
    // Display
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_CONTRAST, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0xFF, SSD1306_I2C_CMD);  // Maximum
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_ENTIRE_ON, SSD1306_I2C_CMD); // Output follows RAM contents
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_NORM_INV, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, SSD1306_SET_IREF_SELECT, SSD1306_I2C_CMD);
    ssd1306_i2c_write_byte(ssd1306, 0x30, SSD1306_I2C_CMD);  // Enable internal IREF during display on

    // Send Display On command
    ssd1306_i2c_write_byte(ssd1306, SSD1306_CMD_DISPLAYON, SSD1306_I2C_CMD);
    sleep_ms(1000);

    return 0;
}

/**
 * @file ssd1306.h
 * @name ssd1306_poweron
 */
int ssd1306_poweron(ssd1306_t * ssd1306) {
    // Initialize the OLED Controller
    ssd1306_poweron_init(ssd1306);

    // Clear screen
    ssd1306_clear_screen(ssd1306);

    return 0;
}

/**
 * @file ssd1306.h
 * @name ssd1306_poweroff
 */
int ssd1306_poweroff(ssd1306_t * ssd1306) {
    // Clear the screen
    ssd1306_clear_screen(ssd1306);

    // Send Display Off command
    ssd1306_i2c_write_byte(ssd1306, SSD1306_CMD_DISPLAYOFF, SSD1306_I2C_CMD);

    return 0;
}

/**
 * @file ssd1306.h
 * @name ssd1306_set_pixel
 */
int ssd1306_set_pixel(ssd1306_t * ssd1306, uint32_t row, uint32_t col, uint16_t val) {
  int page = row / 8;
  uint8_t col_content = ssd1306->gddram[col + page*SSD1306_NB_DISPLAY_COL];
  int pixel_pos = row - 8*page;

  if (val) { col_content |= (0x01 << pixel_pos); }
  else { col_content &= ~(0x01 << pixel_pos);	}

  ssd1306->gddram[col + page*SSD1306_NB_DISPLAY_COL] = col_content;

  return 0;
}

/**
 * @file ssd1306.h
 * @name ssd1306_set_pagecol
 */
int ssd1306_set_pagecol(ssd1306_t * ssd1306, uint32_t page, uint32_t col, uint8_t col_content) {
  if ((page > SSD1306_NB_DISPLAY_PAGE) || (col > SSD1306_NB_DISPLAY_COL)) {
	  return -1;
  }

  ssd1306->gddram[col + page * SSD1306_NB_DISPLAY_COL] = col_content;
  return 0;
}

/**
 * @file ssd1306.h
 * @name ssd1306_write_letter
 */
int ssd1306_write_letter(ssd1306_t * ssd1306, int8_t letter, unsigned int page, unsigned int col) {
  int status;

  bitmap_char_t ascii_letter = ascii_bitmap_lut[letter];
  for (int i=0; i<8; i++) {
	status = ssd1306_set_pagecol(ssd1306, page, col+i, ascii_letter.col[i]);
	if (status != 0) { return status; }
  }

  return 0;
}

/**
 * @file ssd1306.h
 * @name ssd1306_write_str
 */
int ssd1306_write_str(ssd1306_t * ssd1306, const char * str, unsigned int page) {
  if ((str == NULL) || (page > SSD1306_NB_DISPLAY_PAGE)) { return -1; }

  for (int i=0; i<SSD1306_NB_DISPLAY_COL/8; i++) {
	if (str[i] == '\0') { break; }
	ssd1306_write_letter(ssd1306, str[i], page, i*8);
  }
  ssd1306_write_buffer(ssd1306);

  return 0;
}
