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
-- Last update: 2024-12-23
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
// Constants
//----------------------------------------------------------------------------------

/** @brief Software version */
#define SW_VERSION  "0.1.0"

/** @brief Maximum number of command line arguments */
#define CLI_MAX_ARGS  1

/** @brief Maximum number of scan results */
#define CLI_MAX_SCAN_RESULTS 10


//----------------------------------------------------------------------------------
// Types
//----------------------------------------------------------------------------------

/**
 * @brief Useful network information for connection
 */
typedef struct {
  uint8_t ssid_len;   /**> Length of wlan access point name */
  char ssid[32];    /**> Wlan access point name */
  uint8_t auth_mode;  /**> WiFi auth mode \ref CYW43_AUTH_ */
  uint8_t pswd_len;   /**> Length of WiFi password */
  char pswd[32];    /**> WiFi password */
} wifi_network_t;


//----------------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------------

/** @brief List of scanned WiFi networks */
cyw43_ev_scan_result_t network_list[CLI_MAX_SCAN_RESULTS];
/** @brief Index of the current scanned WiFi network */
unsigned int network_idx = 0;


//----------------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------------

/**
 * @brief Print help message
 */
void print_usage() {
  printf("\nAvailable commands:\n");
  printf("  scan                    Scan for WiFi networks\n");
  printf("  networks                List scanned WiFi networks\n");
  printf("  connect <network_id>    Connect to WiFi network\n");
  printf("  disconnect              Disconnect from WiFi network\n");
  printf("  exit                    Exit the command line interface\n");
}

/**
 * @brief Get a line from the user
 * @param line A pointer to the line buffer
 * @param max_len The maximum length of the line
 * @param secret A flag indicating whether to hide the user input
 * @return The number of character get from the user
 */
static uint8_t get_line(char * line, int max_len, bool secret) {
  uint8_t cidx = 0;

  memset(line, 0, max_len);
  while ((fgets(&line[cidx], 2, stdin) != NULL) 
          && (cidx < max_len - 1)
          && (line[cidx] != '\r') && (line[cidx] != '\n')
  ) {
    if (secret) { printf("*"); }
    else { printf("%c", line[cidx]); }
    cidx++;
  };
  printf("\n");

  return cidx;
}

/**
 * @brief Print network information
 */
void print_network_info(cyw43_ev_scan_result_t *network_info, unsigned int network_id) {
  if (network_info->ssid_len == 0) { return; }

  printf("[%02d] ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %u\n",
    network_id,
    network_info->ssid, network_info->rssi, network_info->channel,
    network_info->bssid[0], network_info->bssid[1], network_info->bssid[2], network_info->bssid[3], network_info->bssid[4], network_info->bssid[5],
    network_info->auth_mode);
}

/**
 * @brief Print list of scanned networks
 */
void print_network_list() {
  printf("\nScanned networks:\n");
  for (int i = 0; i < CLI_MAX_SCAN_RESULTS; i++) {
    print_network_info(&network_list[i], i);
  }
}

/**
 * @brief Find if a network is in the list
 */
static bool network_in_list(cyw43_ev_scan_result_t *network_list, const char *ssid) {
  for (int i = 0; i < CLI_MAX_SCAN_RESULTS; i++) {
    if (strcmp((char *)network_list[i].ssid, ssid) == 0) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Add a network to the list
 */
static void network_add(cyw43_ev_scan_result_t *network_list, const cyw43_ev_scan_result_t * network_info) {
  memcpy(&network_list[network_idx], network_info, sizeof(cyw43_ev_scan_result_t));
  if (network_idx >= CLI_MAX_SCAN_RESULTS) { network_idx = 0; }
  else { network_idx++; }
}

/**
 * @brief Callback for storing scan results
 */
static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
  if (result) {
    if (network_in_list(network_list, (char *)result->ssid)) {
      return 0;
    }
    else {
      network_add(network_list, result);
    }
  }

  return 0;
}

/**
 * @brief Scan for WiFi networks
 */
static int scan_wifi_networks(void) {
  cyw43_wifi_scan_options_t scan_options = {0};
  int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
  return err;
}

/**
 * @brief Extract network information from scan result
 */
static wifi_network_t *get_network_info(cyw43_ev_scan_result_t *network_info) {
  wifi_network_t *network = (wifi_network_t *)malloc(sizeof(wifi_network_t));
  network->ssid_len = network_info->ssid_len;
  network->auth_mode = network_info->auth_mode;
  memset(network->ssid, 0, sizeof(network->ssid));
  memset(network->pswd, 0, sizeof(network->pswd));
  memcpy(network->ssid, network_info->ssid, network_info->ssid_len);

  return network;
}

/**
 * @brief Connect to WiFi network
 */
static bool connect_wifi_network(wifi_network_t *network_info) {
  // Ask user for password
  printf("> Enter WiFi password:\n");
  network_info->pswd_len = get_line((char *)network_info->pswd, sizeof(network_info->pswd), true);
  if (network_info->pswd[network_info->pswd_len] == '\r') {
    network_info->pswd[network_info->pswd_len] = '\0';
  }

  // Connect to WiFi network
  int status = cyw43_arch_wifi_connect_timeout_ms(network_info->ssid, network_info->pswd, CYW43_AUTH_WPA2_AES_PSK, 10000);

  return (status == 0);
}

/**
 * @brief Main entry point
 */
int main()
{
  char cli_input[33];
  char cmd[33];
  char args[CLI_MAX_ARGS][33];
  int arg_count = 0;
  int err;

  stdio_init_all();
  sleep_ms(1000);

  //----------------------------------------------------------------------------------
  printf("\n-- WiFi Control App --\n");
  printf("-- Version: %s\n\n", SW_VERSION);

  // Initialize the WiFi stack
  if (cyw43_arch_init()) {
    printf("# Failed to initialize the WiFi stack\n");
    return -1;
  }

  // Set LED on
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);

  // Enable station mode
  cyw43_arch_enable_sta_mode();

  // CLI loop
  while (true) {
    // Get user input
    memset(cli_input, 0, sizeof(cli_input));
    memset(cmd, 0, sizeof(cmd));
    arg_count = 0;
    printf("\r\n$ ");

    get_line(cli_input, sizeof(cli_input), false);

    // Parse user input
    char *str_token = strtok(cli_input, " ");
    // - Get command
    if (str_token != NULL) {
      strcpy(cmd, str_token);
      if (cmd[strlen(cmd) - 1] == '\r') {
        cmd[strlen(cmd) - 1] = '\0';
      }
      str_token = strtok(NULL, " ");
    }
    
    // - Get arguments
    while ((str_token != NULL) && (arg_count < CLI_MAX_ARGS)) {
      printf("> %s\n", str_token);
      strcpy(args[arg_count], str_token);
      if (args[arg_count][strlen(args[arg_count]) - 1] == '\r') {
        args[arg_count][strlen(args[arg_count]) - 1] = '\0';
      }
      arg_count++;
      str_token = strtok(NULL, " ");
    }

    // Process command
    if (strcmp(cmd, "help") == 0) {
      print_usage();
    }
    else if (strcmp(cmd, "scan") == 0) {
      printf("> Performing WiFi scan\n");
      if ((err = scan_wifi_networks()) != 0) {
        printf("# Failed to start scan: %d\n", err);
        continue;
      }
      while (cyw43_wifi_scan_active(&cyw43_state)) { sleep_ms(1000); }
      printf("> Scan complete\n");
    }
    else if (strcmp(cmd, "networks") == 0) {
      print_network_list();
    }
    else if (strcmp(cmd, "connect") == 0) {
      if (arg_count < 1) {
        printf("# Missing network index\n");
        continue;
      }
      int network_id = atoi(args[0]);
      wifi_network_t *network_info = get_network_info(&network_list[network_id]);
      if (network_info->ssid_len == 0) {
        printf("# Invalid network index\n");
        if (network_info != NULL) { free(network_info); }
        continue;
      }
      printf("> Connecting to WiFi network %s...\n", network_info->ssid);
      if (!connect_wifi_network(network_info)) {
        printf("# Failed to connect to network\n");
        continue;
      }
      printf("> Connected to WiFi network\n");
      if (network_info != NULL) { free(network_info); }
    }
    else if (strcmp(cmd, "disconnect") == 0) {
      cyw43_arch_disable_sta_mode();
      printf("> Disconnected from WiFi network\n");
    }
    else if (strcmp(cmd, "exit") == 0) {
      break;
    }
  }

#if 0
  // Scanning loop
  bool led_val = true;
  absolute_time_t scan_time = nil_time;
  bool scan_in_progress = false;

  while (true) {
    if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0) {
      if (!scan_in_progress) {
        scan_in_progress = scan_wifi_networks();
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
#endif

  cyw43_arch_deinit();

  printf("\n> System halted\n");
  while (true){;}

  return 0;
}
