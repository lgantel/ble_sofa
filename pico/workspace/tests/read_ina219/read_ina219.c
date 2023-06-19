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
-- File Name: read_ina219.c
-- Description: INA219 tests: read current measure
--
-- Last update: 2023-06-19
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_SDA_PIN    16
#define I2C_SCL_PIN    17

// INA219 I2C address
static const uint8_t INA219_ADDR = 0x40;

// Registers
static const uint8_t REG_CONFIG = 0x00;
static const uint8_t REG_SHUNTVOLTAGE = 0x01;
static const uint8_t REG_BUSVOLTAGE = 0x02;
static const uint8_t REG_POWER = 0x03;
static const uint8_t REG_CURRENT = 0x04;
static const uint8_t REG_CALIBRATION = 0x05;

// Prototypes
int reg_write(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes);

int reg_read(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes);

// Functions
int reg_write(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes) {
    uint8_t msg[nbytes + 1];

    // Check to make sure caller is sending 1 or more bytes
    if (nbytes < 1) {
        return -1;
    }

    // Append register address to front of data packet
    msg[0] = reg;
    for (int i = 0; i < nbytes; i++) {
        msg[i + 1] = buf[i];
    }

    // Write data to register(s) over I2C
    i2c_write_blocking(i2c, addr, msg, (nbytes + 1), false);

    return 0;
}

int reg_read(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes) {
    int num_bytes_read = 0;

    // Check to make sure caller is asking for 1 or more bytes
    if (nbytes < 1) {
        return -1;
    }

    // Read data from register(s) over I2C
    i2c_write_blocking(i2c, addr, &reg, 1, true);
    num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

    return num_bytes_read;
}

float get_vshunt(i2c_inst_t *i2c) {
    uint8_t data[2];
    int16_t reg_value = 0x0000;
    int16_t sign = 1;

    reg_read(i2c, INA219_ADDR, REG_SHUNTVOLTAGE, data, 2);
    reg_value = (data[0] << 8) | data[1];

    if (reg_value > 32768) { // negative value
        sign = -1;
        for (int i=0; i<16; i++) {
            reg_value = (reg_value ^ (1 << i));
        }
    }

    // VShunt LSB equals 10 uV   
    return ((float)reg_value) * 10 * sign;
}

float get_vbus(i2c_inst_t *i2c) {
    uint8_t data[2];
    uint16_t reg_value = 0x0000;

    reg_read(i2c, INA219_ADDR, REG_BUSVOLTAGE, data, 2);
    reg_value = (data[0] << 8) | data[1];

    // VBus is stored on bits [15:3]
    reg_value >>= 3;

    // LSB equals 4mV
    return ((float)(reg_value) * 0.004);
}

// Main entry point
int main()
{
    // Ports
    i2c_inst_t *i2c = i2c0;

    // Buffer to store raw reads
    uint8_t data[6];

    // Voltage, current and power
    float v, i, p;

    // Initialize chosen serial port
    stdio_init_all();
    sleep_ms(1000);
    
    printf("-- Read INA219 --\n");

    //Initialize I2C port at 100 kHz
    i2c_init(i2c, 100 * 1000);

    // Initialize I2C pins
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    // Read device configuration to make sure that we can communicate with the INA219
    reg_read(i2c, INA219_ADDR, REG_CONFIG, data, 2);
    // Must read 0x399F
    if ((data[0] != 0x39) || (data[1] != 0x9F)) {
        printf("INA219> Read 0x%02x%02x [FAILED]\n", data[0], data[1]);
        while (true) {;}
    }
    printf("INA219> Read 0x%02x%02x [OK]\n", data[0], data[1]);

    while(true) {
        printf("Measure:\n");

        v = get_vbus(i2c0);
        sleep_ms(10);

        i = get_vshunt(i2c0);
        sleep_ms(10);

        p = i * v;
        printf("v = %.3f V, i = %.3f uV, P = %.3f uW\n", v, i, p);

        sleep_ms(1000);
    }

    return 0;
}
