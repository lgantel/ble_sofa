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
-- Last update: 2023-06-12
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

//----------------------------------------------------------------------------------
// GPIO variables
//----------------------------------------------------------------------------------

#define LED_GPIO 2
static bool led_val = false;

//----------------------------------------------------------------------------------
// Bluetooth variables
//----------------------------------------------------------------------------------

#define REPORT_INTERVAL_MS 3000
#define MAX_NR_CONNECTIONS 3 

/** @brief Advertisements information */
const uint8_t adv_data[] = {
    2, BLUETOOTH_DATA_TYPE_FLAGS, 0x06, 
    8, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'b', 'l', 'e','2', 'w', 'a', 'y',
    17, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS, 0x9e, 0xca, 0xdc, 0x24, 0xe, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x1, 0x0, 0x40, 0x6e,
};
const uint8_t adv_data_len = sizeof(adv_data);

/** @brief HCI registration callback */
static btstack_packet_callback_registration_t hci_event_callback_registration;

/** @brief Round robin sending */
static int connection_index;

/**
 * @brief Support for multiple clients
 */
typedef struct {
    char name;
    int le_notification_enabled;
    hci_con_handle_t connection_handle;
    int  counter;
    char test_data[200];
    int  test_data_len;
    uint32_t test_data_sent;
    uint32_t test_data_start;
    btstack_context_callback_registration_t send_request;
} nordic_spp_le_streamer_connection_t;

static nordic_spp_le_streamer_connection_t nordic_spp_le_streamer_connections[MAX_NR_CONNECTIONS];

//----------------------------------------------------------------------------------
// Streaming functions
//----------------------------------------------------------------------------------

/** 
 * @brief Initialize connection handles
 */
static void init_connections(void) {
    // Logic to track connections
    for (int i=0; i<MAX_NR_CONNECTIONS; i++) {
      nordic_spp_le_streamer_connections[i].connection_handle = HCI_CON_HANDLE_INVALID;
      nordic_spp_le_streamer_connections[i].name = 'A' + i;
    }
}

/** 
 * @brief Get connection handle
 */
static nordic_spp_le_streamer_connection_t * connection_for_conn_handle(hci_con_handle_t conn_handle) {
    for (int i=0; i<MAX_NR_CONNECTIONS; i++) {
      if (nordic_spp_le_streamer_connections[i].connection_handle == conn_handle) {
        return &nordic_spp_le_streamer_connections[i];
      }
    }

    return NULL;
}

/** 
 * @brief Get next connection index 
 */
static void next_connection_index(void){
    connection_index++;
    if (connection_index == MAX_NR_CONNECTIONS) {
        connection_index = 0;
    }
}

/**
 * @brief Reset the timer counter for performance test
 */
static void test_reset(nordic_spp_le_streamer_connection_t *context) {
  context->test_data_start = btstack_run_loop_get_time_ms();
  context->test_data_sent = 0;
}

static void test_track_sent(nordic_spp_le_streamer_connection_t *context, int bytes_sent) {
  context->test_data_sent += bytes_sent;

  // Evaluate
  uint32_t now = btstack_run_loop_get_time_ms();
  uint32_t time_passed = now - context->test_data_start;
  if (time_passed < REPORT_INTERVAL_MS) return;

  // Print speed
  int bytes_per_second = context->test_data_sent * 1000 / time_passed;
  printf("%c: %u bytes sent-> %u.%03u kB/s\n", context->name, context->test_data_sent, bytes_per_second / 1000, bytes_per_second % 1000);

  // Restart
  context->test_data_start = now;
  context->test_data_sent = 0;
}

/**
 * @brief Check if a notification can be sent now
 *        On each iteration, a single letter is increased and used as test data
 */
static void nordic_can_send(void *some_context) {
  UNUSED(some_context);

  // Find the next active streaming connection
  int old_connection_index = connection_index;

  while (true) {
    // Active connection found
    if ((nordic_spp_le_streamer_connections[connection_index].connection_handle != HCI_CON_HANDLE_INVALID) &&
        (nordic_spp_le_streamer_connections[connection_index].le_notification_enabled)) { break; }

    // Check next
    next_connection_index();

    // None found
    if (connection_index == old_connection_index) { return; }
  }

  // Get the context of the active connection
  nordic_spp_le_streamer_connection_t *context = &nordic_spp_le_streamer_connections[connection_index];

  // Create test data
  context->counter++;
  if (context->counter > 'Z') context->counter = 'A';
  memset(context->test_data, context->counter, context->test_data_len);

  /**********************************************************************************************/
  /****************** put your own function here to send notification to android app ************/
  /**********************************************************************************************/

  // Send
  nordic_spp_service_server_send(context->connection_handle, (uint8_t *) context->test_data, context->test_data_len);

  // Track
  test_track_sent(context, context->test_data_len);

  // Request next send event
  nordic_spp_service_server_request_can_send_now(&context->send_request, context->connection_handle);

  // Check next
  next_connection_index();
}

//----------------------------------------------------------------------------------
// Bluetooth static functions
//----------------------------------------------------------------------------------

/**
 * @brief Attribute Protocol (ATT) Packet Handler
 */
static void att_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
  UNUSED(channel);
  UNUSED(size);
    
  if (packet_type != HCI_EVENT_PACKET) return;

  int mtu;
  nordic_spp_le_streamer_connection_t * context;

  switch (hci_event_packet_get_type(packet)) {
    case ATT_EVENT_CONNECTED:
      // Setup new connection
      context = connection_for_conn_handle(HCI_CON_HANDLE_INVALID);
      if (!context) break;
      context->counter = 'A';
      context->test_data_len = ATT_DEFAULT_MTU - 4;
      context->connection_handle = att_event_connected_get_handle(packet);
      break;
    case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
      mtu = att_event_mtu_exchange_complete_get_MTU(packet) - 3;
      context = connection_for_conn_handle(att_event_mtu_exchange_complete_get_handle(packet));
      if (!context) { break; }
      context->test_data_len = btstack_min(mtu - 3, sizeof(context->test_data));
      printf("%c: ATT MTU = %u => use test data of len %u\n", context->name, mtu, context->test_data_len);
      break;
    case ATT_EVENT_DISCONNECTED:
      context = connection_for_conn_handle(att_event_disconnected_get_handle(packet));
      if (!context) break;
      // Free connection
      printf("%c: Disconnect\n", context->name);                    
      context->le_notification_enabled = 0;
      context->connection_handle = HCI_CON_HANDLE_INVALID;
      break;
    default:
      break;
  }
}

/**
 * @brief Host Controller Interface (HCI) Packet Handler
 */
static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
  UNUSED(channel);
  UNUSED(size);

  uint16_t conn_interval;
  hci_con_handle_t con_handle;

  if (packet_type != HCI_EVENT_PACKET) { return; }

  switch (hci_event_packet_get_type(packet)) {
    case BTSTACK_EVENT_STATE:
      // BTstack activated, get started
      if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
        printf("> To start the streaming, please run nRF Toolbox -> UART to connect\n");
      }
      break;
    case HCI_EVENT_LE_META:
      switch (hci_event_le_meta_get_subevent_code(packet)) {
        case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
          // Print connection parameters (without using float operations)
          con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
          conn_interval = hci_subevent_le_connection_complete_get_conn_interval(packet);
          printf("LE Connection - Connection Interval: %u.%02u ms\n", conn_interval * 125 / 100, 25 * (conn_interval & 3));
          printf("LE Connection - Connection Latency: %u\n", hci_subevent_le_connection_complete_get_conn_latency(packet));

          // Request min con_interval 15ms for iOS 11+
          printf("LE Connection - Request 15 ms connection interval\n");
          gap_request_connection_parameter_update(con_handle, 12, 12, 0, 0x0048);
          break;
        case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
          // Print connection parameters (without using floating point operations)
          con_handle    = hci_subevent_le_connection_update_complete_get_connection_handle(packet);
          conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
          printf("LE Connection - Connection Param update - connection interval %u.%02u ms, latency %u\n", 
                    conn_interval * 125 / 100,
                    25 * (conn_interval & 3), hci_subevent_le_connection_update_complete_get_conn_latency(packet));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/**
 * @brief Serial Port Profile (SPP) packet handler
 */
static void nordic_spp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
  hci_con_handle_t con_handle;
  nordic_spp_le_streamer_connection_t * context;

  switch (packet_type) {
    case HCI_EVENT_PACKET:
      if (hci_event_packet_get_type(packet) != HCI_EVENT_GATTSERVICE_META) break;
      switch (hci_event_gattservice_meta_get_subevent_code(packet)) {
        case GATTSERVICE_SUBEVENT_SPP_SERVICE_CONNECTED:
          con_handle = gattservice_subevent_spp_service_connected_get_con_handle(packet);
          context = connection_for_conn_handle(con_handle);
          if (!context) break;
          context->le_notification_enabled = 1;
          test_reset(context);
          context->send_request.callback = &nordic_can_send;
          nordic_spp_service_server_request_can_send_now(&context->send_request, context->connection_handle);
          break;
        case GATTSERVICE_SUBEVENT_SPP_SERVICE_DISCONNECTED:
          con_handle = HCI_CON_HANDLE_INVALID;
          context = connection_for_conn_handle(con_handle);
          if (!context) break;
          context->le_notification_enabled = 0;
          break;
        default:
          break;
      }
      break;

    /********************************************************************************/
    /***** put your own function here to process received data from android app *****/
    /********************************************************************************/      
    case RFCOMM_DATA_PACKET:
      if (packet[0] == 0x74) {
        gpio_put(LED_GPIO, led_val);
        led_val = !led_val;
      }
      context = connection_for_conn_handle((hci_con_handle_t) channel);
      if (!context) break;
      test_track_sent(context, size);
      break;
    default:
      break;
  }
}

//----------------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------------

/** @brief Main entry point */
int main()
{
  stdio_init_all();
  printf("\n-----------------\n");
  printf("-- BLE Control --\n");
  printf("-----------------\n\n");

  // Initialie the LED output
  gpio_init(LED_GPIO);
  gpio_set_dir(LED_GPIO, GPIO_OUT);
  gpio_put(LED_GPIO, 0);

  // Initialize the Bluetooth stack
  if (cyw43_arch_init()) return -1;

  // Register HCI event callback
  hci_event_callback_registration.callback = &hci_packet_handler;
  hci_add_event_handler(&hci_event_callback_registration);

  // Initialize the Logical Link Control and Adaptation Layer Protocol (L2CAP) layer
  l2cap_init();
  // Initialize Security Manager (SM)
  sm_init();
  // Initialize Attribute Protocol
  att_server_init(profile_data, NULL, NULL);
  nordic_spp_service_server_init(&nordic_spp_packet_handler);
  att_server_register_packet_handler(att_packet_handler);

  // TODO: To comment
  uint16_t adv_int_min = 0x0030;
  uint16_t adv_int_max = 0x0030;
  uint8_t adv_type = 0;
  bd_addr_t null_addr;
  memset(null_addr, 0, 6);

  gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
  gap_advertisements_set_data(adv_data_len, (uint8_t *) adv_data);
  gap_advertisements_enable(true);

  init_connections();
  hci_power_control(HCI_POWER_ON);

  // Endless loop
  btstack_run_loop_execute();

  return 0;
}
