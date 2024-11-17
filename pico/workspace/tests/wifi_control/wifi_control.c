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
-- File Name: wifi_control.c
-- Description: WiFi tests: leds and button control through web server
--
-- Last update: 2024-11-17
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"

#include "hardware/clocks.h"


//----------------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------------

/**
 * @brief Print scanning results
 */
static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    if (result) {
        printf("ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %u\n",
            result->ssid, result->rssi, result->channel,
            result->bssid[0], result->bssid[1], result->bssid[2], result->bssid[3], result->bssid[4], result->bssid[5],
            result->auth_mode);
    }
    return 0;
}

/**
 * @brief Main entry point
 */
int main()
{
  bool led_val = true;

  stdio_init_all();

  //----------------------------------------------------------------------------------
  // Initialize the WiFi stack
  printf("> Initializing the WiFi stack\n");
  if (cyw43_arch_init()) {
    printf("# Failed to initialize the WiFi stack\n");
    return -1;
  }

  // Set LED on
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);

  // Enable station mode
  cyw43_arch_enable_sta_mode();

  // Scanning loop
  absolute_time_t scan_time = nil_time;
  bool scan_in_progress = false;

  while (true) {
    if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0) {
      if (!scan_in_progress) {
        cyw43_wifi_scan_options_t scan_options = {0};
        int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
        if (err == 0) {
          printf("\n> Performing WiFi scan\n");
          scan_in_progress = true;
        } else {
          printf("# Failed to start scan: %d\n", err);
          scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
        }
      }
      else if (!cyw43_wifi_scan_active(&cyw43_state)) {
        scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
        scan_in_progress = false; 
      }
    }

    sleep_ms(500);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_val);
    led_val = !led_val;
  }

  cyw43_arch_deinit();

  return 0;
}
