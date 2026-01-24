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
-- File Name: ble_sofa_app.c
-- Description: Relays control via BLE to turn on/off two 29V DC sofa motors
--
-- Last update: 2025-12-23
--
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stddef.h>

#include "lwip/apps/httpd.h"
#include "lwip/apps/fs.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

#include "hw_mgmt.h"
#include "flash_utils.h"
#include "relay.h"
#include "ssd1306.h"

#if 0
#include "ble_utils.h"
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "pico_error_str.h"

// for #define's that configure lwIP stack (enable what we need and not more)
#include "lwipopts.h"

//----------------------------------------------------------------
// Constants
//----------------------------------------------------------------

/** @brief Software version */
#define SW_VERSION  "0.1.0"

// Which core to run on if configNUMBER_OF_CORES==1
#ifndef RUN_FREE_RTOS_ON_CORE
  #define RUN_FREE_RTOS_ON_CORE 0
#endif

// Whether to flash the led
#ifndef USE_LED
  #define USE_LED 1
#endif

// Delay between led blinking
#define LED_DELAY_MS 500

// Priorities of our threads - higher numbers are higher priority
#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define BLINK_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
#define WORKER_TASK_PRIORITY    ( tskIDLE_PRIORITY + 4UL )

// Stack sizes of our threads in words (4 bytes)
#define MAIN_TASK_STACK_SIZE 1024
#define BLINK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define WORKER_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

#define LED_GPIO          2

//----------------------------------------------------------------
// Global variables
//----------------------------------------------------------------

/** @brief Structure to control Relay1 */
extern relay_t relay1;

/** @brief Structure to control Relay2 */
extern relay_t relay2;

/** @brief Structure to control SSD1306 OLED display */
extern ssd1306_t ssd1306;

#if 0
/** @brief Advertisements information */
extern uint8_t adv_data[];
extern uint8_t adv_data_len;

/** @brief Relay register */
extern uint8_t bu_relay_reg;
extern int bu_relay_reg_len;

/** @brief BLE connection status */
extern bool bu_ble_connected;
#endif

extern uint32_t ADDR_PERSISTENT[];
#define ADDR_PERSISTENT_BASE_ADDR (ADDR_PERSISTENT)

//----------------------------------------------------------------
// FreeRTOS Static Functions
//----------------------------------------------------------------

#include "pico/async_context_freertos.h"
static async_context_freertos_t async_context_instance;

// Create an async context
static async_context_t *example_async_context(void) {
    async_context_freertos_config_t config = async_context_freertos_default_config();
    config.task_priority = WORKER_TASK_PRIORITY; // defaults to ASYNC_CONTEXT_DEFAULT_FREERTOS_TASK_PRIORITY
    config.task_stack_size = WORKER_TASK_STACK_SIZE; // defaults to ASYNC_CONTEXT_DEFAULT_FREERTOS_TASK_STACK_SIZE
    if (!async_context_freertos_init(&async_context_instance, &config))
        return NULL;
    return &async_context_instance.core;
}

#if USE_LED
// Turn led on or off
static void pico_set_led(bool led_on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}

// Initialise led
static void pico_init_led(void) {
    hard_assert(cyw43_arch_init() == PICO_OK);
    pico_set_led(false); // make sure cyw43 is started
}
#endif

#if 0
static void process_ble_connection(void) {
  if (bu_ble_connected) {
    ssd1306_write_str(&ssd1306, "-- Connected    ", 0);
  }
  else {
    ssd1306_write_str(&ssd1306, "-- Disconnected ", 0);
  } 
}

static void process_relays(void) {
  // Process relay_reg
  // - Bit [0] is used to set Relay1 on/off
  if (bu_relay_reg & 0x01) { 
    ssd1306_write_str(&ssd1306, "-- Relay 1 ON   ", 1);
    ssd1306_write_str(&ssd1306, "-- Relay 2 OFF  ", 2);
    relay_on(&relay1);
    relay_off(&relay2);
  // - Bit [1] is used to set Relay2 on/off
  } else if (bu_relay_reg & 0x02) { 
    ssd1306_write_str(&ssd1306, "-- Relay 1 OFF  ", 1);
    ssd1306_write_str(&ssd1306, "-- Relay 2 ON   ", 2);
    relay_off(&relay1);
    relay_on(&relay2);
  } else { 
    ssd1306_write_str(&ssd1306, "-- Relay 1 OFF  ", 1);
    ssd1306_write_str(&ssd1306, "-- Relay 2 OFF  ", 2);
    relay_off(&relay1);
    relay_off(&relay2);
  }
}
#endif

//----------------------------------------------------------------
// Functions
//----------------------------------------------------------------

/**
 * @brief FreeRTOS task: Blinking LED
 */
void blink_task(void *params) {
  bool on = false;
  pico_init_led();

  while(true) {
#if configNUMBER_OF_CORES > 1
    static int last_core_id = -1;
    if (portGET_CORE_ID() != last_core_id) {
        last_core_id = portGET_CORE_ID();
        //printf("blink task is on core %d\n", last_core_id);
    }
#endif
    pico_set_led(on);
    on = !on;
    sleep_ms(LED_DELAY_MS);
  }
}

#if 0
// async workers run in their own thread when using async_context_freertos_t with priority WORKER_TASK_PRIORITY
static void run_server(async_context_t *context, async_at_time_worker_t *worker) {
    async_context_add_at_time_worker_in_ms(context, worker, 10000);
    static uint32_t count = 0;
}
async_at_time_worker_t worker_timeout = { .do_work = run_server };
#endif

static const char *cgi_handler_test(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
  printf("cgi_handler_test\n");
  printf("iIndex: %d\n", iIndex);
  printf("iNumParams: %d\n", iNumParams);
  for (int i = 0; i < iNumParams; i++) {
    printf("pcParam[%d]: %s\n", i, pcParam[i]);  
  }
  
  if (iNumParams > 0) {
    if (strcmp(pcParam[0], "test") == 0) {
      return "/test.shtml";
    }
  }
  return "/index.html";
}

static tCGI cgi_handlers[] = {
  { "/", cgi_handler_test },
  { "/index.html", cgi_handler_test },
};

static const char *ssi_tags[] = {
  "status",
  "welcome"
};

// Note that the buffer size is limited by LWIP_HTTPD_MAX_TAG_INSERT_LEN, so use LWIP_HTTPD_SSI_MULTIPART to return larger amounts of data
u16_t ssi_example_ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
  size_t printed;

  switch (iIndex) {
    case 0: { // "status"
      printed = snprintf(pcInsert, iInsertLen, "Pass");
      break;
    }
    case 1: { // "welcome"
      printed = snprintf(pcInsert, iInsertLen, "Hello from Pico");
      break;
    }
    default: { // unknown tag
      printed = 0;
      break;
    }
  }

  return (u16_t) printed;
}

/**
 * @brief FreeRTOS task: Main task
 */
void main_task(__unused void *params) {
  //char buf[17];
  //uint32_t count = 0;
  //async_context_t *context = example_async_context();
  // start the worker running
  //async_context_add_at_time_worker_in_ms(context, &worker_timeout, 0);

  // Initialize the WiFi
  cyw43_arch_init();
  printf("WiFi initialized\n");

  // Enable station mode
  cyw43_arch_enable_sta_mode();

  // this seems to be the best we can do using the predefined `cyw43_pm_value` macro:
  // cyw43_wifi_pm(&cyw43_state, CYW43_PERFORMANCE_PM);
  // however it doesn't use the `CYW43_NO_POWERSAVE_MODE` value, so we do this instead:
  cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));

  int err;
  do {
    printf("> Attempt to connect to WiFi %s...\n", WIFI_SSID);
    err = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if (err != PICO_OK) {
      printf("# Failed to connect (%s)\n", pico_error_str(err));
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  } while(err != PICO_OK);
  printf("> WiFi connected\n");

  // Initialize the web server
  cyw43_arch_lwip_begin();
  httpd_init();
  http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
  http_set_ssi_handler(ssi_example_ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
  cyw43_arch_lwip_end();
  printf("> HTTP server initialized\n");

  printf("\nListening at %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
  vTaskDelete(NULL);

  //async_context_deinit(context);
}

#if 0
void vLaunch( void) {
    TaskHandle_t task;
    xTaskCreate(main_task, "MainThread", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &task);

#if configUSE_CORE_AFFINITY && configNUMBER_OF_CORES > 1
    // we must bind the main task to one core (well at least while the init is called)
    vTaskCoreAffinitySet(task, 1);
#endif

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}
#endif

/**
 * @brief Main entry point
 * @return int Endless loop, never returns
 */
int main(void)
{
  stdio_init_all();

  // Initialize the hardware components
  hw_init();

  // Wait a moment
  sleep_ms(1000);

  printf("\n-- BLE Sofa App --\n");
  printf("-- Version: %s\n\n", SW_VERSION);

  xTaskCreate(main_task, "MainThread", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);
  vTaskStartScheduler();

  //char buf[17] = {0x00};
  //uint32_t * p_persistent = fu_get_addr_persistent();

  //ssd1306_write_str(&ssd1306, "> Hw Init       ", 1);

  #if 0
    // Initialize the BLE stack
    btstack_init();

    // Setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t *) adv_data);
    gap_advertisements_enable(true);

    // Register HCI events callback
    hci_event_callback_registration.callback = &hci_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Register for ATT events
    att_server_register_packet_handler(att_packet_handler);

    // Initialize data
    bu_relay_reg = 0x00;
    bu_relay_reg_len = 1;

    hci_power_control(HCI_POWER_ON);

    // Turn on the LED to indicate that BLE is fully initialized
    cyw43_arch_gpio_put(WL_LED_GPIO, true);

    ssd1306_write_str(&ssd1306, "--Bluetooth On--", 1);

    // Endless loop
    //btstack_run_loop_execute();
    while (1) {
      process_ble_connection();
      process_relays();
    }
#endif

    // Deinitialize the hardware components
    hw_deinit();

    return 0;
}
