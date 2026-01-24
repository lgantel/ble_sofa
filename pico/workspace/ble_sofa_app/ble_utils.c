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
-- File Name: ble_utils.c
-- Description: BLE Utilities
--
-- Last update: 2025-12-21
--
-------------------------------------------------------------------------------*/

#include "ble_utils.h"
#include "mygatt.h"

//----------------------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------------------

/** @brief Advertisements information */
const uint8_t adv_data[] = {
    2, BLUETOOTH_DATA_TYPE_FLAGS, 0x06, 
    9, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'b', 'l', 'e','-', 's', 'o', 'f', 'a',
    // Incomplete List of 16-bit Service Class UUIDs -- FF10 - only valid for testing!
    3, BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0x10, 0xff,
};
const uint8_t adv_data_len = sizeof(adv_data);

// Relay register: command the relays status:
//   - bit [0]: '0' = Relay1 OFF, '1' = Relay1 ON
//   - bit [1]: '0' = Relay2 OFF, '1' = Relay2 ON
uint8_t bu_relay_reg = 0x00;
int bu_relay_reg_len = 1; // Data length in bytes

// BLE connection status
bool bu_ble_connected = false;

//----------------------------------------------------------------------------------
// Bluetooth functions
//----------------------------------------------------------------------------------

/**
 * @name btstack_init
 * @file ble_utils.h
 */
int btstack_init(void) {
  // Initialize the Bluetooth stack
  if (cyw43_arch_init()) return -1;

  // Turn off the wireless LED
  cyw43_arch_gpio_put(WL_LED_GPIO, false);

  // Initialize the Logical Link Control and Adaptation Layer Protocol (L2CAP) layer
  l2cap_init();
  // Initialize Security Manager (SM)
  sm_init();
  // Initialize Attribute Protocol
  att_server_init(profile_data, att_read_callback, att_write_callback);

  return 0;
}

/**
 * @name att_packet_handler
 * @file ble_utils.h
 */
void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
  UNUSED(channel);
  UNUSED(size);

  //uint16_t conn_interval;
  hci_con_handle_t con_handle;

  if (packet_type != HCI_EVENT_PACKET) { return; }

  switch (hci_event_packet_get_type(packet)) {
    case BTSTACK_EVENT_STATE:
      // BTstack activated, get started
      if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
        //printf("> BLE Control - BTstack activated\n");
      }
      break;
    case HCI_EVENT_LE_META:
      switch (hci_event_le_meta_get_subevent_code(packet)) {
        case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
          // Print connection parameters (without using float operations)
          con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
          //conn_interval = hci_subevent_le_connection_complete_get_conn_interval(packet);
          //printf("LE Connection - Connection Interval: %u.%02u ms\n", conn_interval * 125 / 100, 25 * (conn_interval & 3));
          //printf("LE Connection - Connection Latency: %u\n", hci_subevent_le_connection_complete_get_conn_latency(packet));

          // Request min con_interval 15ms for iOS 11+
          //printf("LE Connection - Request 15 ms connection interval\n");
          gap_request_connection_parameter_update(con_handle, 12, 12, 0, 0x0048);
          break;
        case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
          // Print connection parameters (without using floating point operations)
          con_handle    = hci_subevent_le_connection_update_complete_get_connection_handle(packet);
          //conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
          //printf("LE Connection - Connection Param update - connection interval %u.%02u ms, latency %u\n", 
          //          conn_interval * 125 / 100,
          //          25 * (conn_interval & 3), hci_subevent_le_connection_update_complete_get_conn_latency(packet));
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
 * @name att_read_callback
 * @file ble_utils.h
 */
uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
    UNUSED(con_handle);

    //printf("> att_read_callback: att_handle %04x, offset %04x, buff size %04x\n", att_handle, offset, buffer_size);

    if (att_handle == ATT_CHARACTERISTIC_0000FF11_VALUE_HANDLE) {
        return att_read_callback_handle_blob((const uint8_t *)&bu_relay_reg, bu_relay_reg_len, offset, buffer, buffer_size);
    }

    return 0;
}

/**
 * @name att_write_callback
 * @file ble_utils.h
 */
int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    UNUSED(connection_handle);
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer_size);

    //printf("> att_write_callback: att_handle %04x, offset %04x, buff size %04x\n", att_handle, offset, buffer_size);
    if ((buffer == NULL) || (att_handle != ATT_CHARACTERISTIC_0000FF11_VALUE_HANDLE)) { return 0; }

    // Update bu_relay_reg
    bu_relay_reg = *buffer;
    //printf("> Update bu_relay_reg\t- bu_relay_reg = %02x\n", bu_relay_reg);
    
    return 0;
}

/**
 * @name att_packet_handler
 * @file ble_utils.h
 */
void att_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
  UNUSED(channel);
  UNUSED(size);
    
  if (packet_type != HCI_EVENT_PACKET) return;

  switch (hci_event_packet_get_type(packet)) {
    case ATT_EVENT_CONNECTED:
      bu_ble_connected = true;
      break;
    case ATT_EVENT_DISCONNECTED:
      bu_ble_connected = false;
      break;
    default:
      break;
  }
}
