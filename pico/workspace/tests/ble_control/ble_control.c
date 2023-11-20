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
-- Last update: 2023-06-28
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

//----------------------------------------------------------------------------------
// Bluetooth variables
//----------------------------------------------------------------------------------

#define REPORT_INTERVAL_MS 3000
#define MAX_NR_CONNECTIONS 3 

/** @brief LED Command characteristic */
#define ATT_CHARACTERISTIC_0000FF11_VALUE_HANDLE 0x0006

/** @brief Advertisements information */
const uint8_t adv_data[] = {
    2, BLUETOOTH_DATA_TYPE_FLAGS, 0x06, 
    12, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'b', 'l', 'e','-', 'c', 'o', 'n', 't', 'r', 'o', 'l',
    // Incomplete List of 16-bit Service Class UUIDs -- FF10 - only valid for testing!
    3, BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0x10, 0xff,
};
const uint8_t adv_data_len = sizeof(adv_data);

/** @brief HCI registration callback */
static btstack_packet_callback_registration_t hci_event_callback_registration;

// Data: command the LED status:
//   - bit [0]: '0' = OFF, '1' = ON
static uint8_t data = 0x00;
static int data_len = 1; // Data length in byte

//----------------------------------------------------------------------------------
// Bluetooth static functions
//----------------------------------------------------------------------------------

// Prototypes
static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static void att_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

/**
 * @brief Host Controller Interface (HCI) Packet Handler
 * 
 * @param packet_type 
 * @param channel 
 * @param packet 
 * @param size 
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
        printf("> BLE Control - BTstack activated\n");
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
 * @brief ATT client read callback
 * 
 * @param connection_handle 
 * @param att_handle 
 * @param offset 
 * @param buffer 
 * @param buffer_size 
 * @return uint16_t 
 */
static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
    UNUSED(connection_handle);

    //printf("> att_read_callback: att_handle %04x, offset %04x, buff size %04x\n", att_handle, offset, buffer_size);

    if (att_handle == ATT_CHARACTERISTIC_0000FF11_VALUE_HANDLE) {
        return att_read_callback_handle_blob((const uint8_t *)&data, data_len, offset, buffer, buffer_size);
    }

    return 0;
}

/**
 * @brief ATT client write callback
 * 
 * @param connection_handle 
 * @param att_handle 
 * @param transaction_mode 
 * @param offset 
 * @param buffer 
 * @param buffer_size 
 * @return int 
 */
static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    UNUSED(connection_handle);
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer_size);

    //printf("> att_write_callback: att_handle %04x, offset %04x, buff size %04x\n", att_handle, offset, buffer_size);
    if ((buffer == NULL) || (att_handle != ATT_CHARACTERISTIC_0000FF11_VALUE_HANDLE)) { return 0; }

    // Update data
    data = *buffer;
    //printf("> Update data\t- data = %02x\n", data);

    // Process data
    if (data & 0x01) { // Set LED on
      gpio_put(LED_GPIO, true);
    }
    else { // Set LED off
      gpio_put(LED_GPIO, false);
    }
    
    return 0;
}

/**
 * @brief Attribute Protocol (ATT) Packet Handler
 * 
 * @param packet_type 
 * @param channel 
 * @param packet 
 * @param size 
 */
static void att_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
  UNUSED(channel);
  UNUSED(size);
    
  if (packet_type != HCI_EVENT_PACKET) return;

  switch (hci_event_packet_get_type(packet)) {
    case ATT_EVENT_CONNECTED:
      printf("Connected\n");
      break;
    case ATT_EVENT_DISCONNECTED:
      printf("Disconnected\n");
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

  //----------------------------------------------------------------------------------
  // Initialie the LED output
  gpio_init(LED_GPIO);
  gpio_set_dir(LED_GPIO, GPIO_OUT);
  gpio_put(LED_GPIO, false);

  // Initialize the Bluetooth stack
  if (cyw43_arch_init()) return -1;

  // Initialize the Logical Link Control and Adaptation Layer Protocol (L2CAP) layer
  l2cap_init();
  // Initialize Security Manager (SM)
  sm_init();
  // Initialize Attribute Protocol
  att_server_init(profile_data, att_read_callback, att_write_callback);

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
  data = 0x00;
  data_len = 1;

  hci_power_control(HCI_POWER_ON);

  // Endless loop
  btstack_run_loop_execute();

  return 0;
}
