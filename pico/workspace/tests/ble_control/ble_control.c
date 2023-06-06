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
-- File Name: ble_control.c
-- Description: BLE tests: leds and button control through GATT server
--
-- Last update: 2023-06-06
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "btstack_run_loop.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "btstack_event.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "btstack.h"
#include "ble/gatt-service/nordic_spp_service_server.h"
#include "mygatt.h"

#define LED_GPIO 2

int main()
{
  bool led_val = false;

  stdio_init_all();
  printf("-- BLE Control --\n");

  // Initialie the LED output
  gpio_init(LED_GPIO);
  gpio_set_dir(LED_GPIO, GPIO_OUT);

  while(true) {
    sleep_ms(500);
    led_val = !led_val;
    gpio_put(LED_GPIO, led_val);
  }

  return 0;
}
