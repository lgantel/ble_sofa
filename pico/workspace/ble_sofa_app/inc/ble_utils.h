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
-- File Name: ble_utils.h
-- Description: BLE Utilities
--
-- Last update: 2025-12-21
--
-------------------------------------------------------------------------------*/

#ifndef BLE_UTILS_H
#define BLE_UTILS_H

#include "btstack_run_loop.h"
#include "btstack_event.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "btstack.h"
#include "ble/gatt-service/nordic_spp_service_server.h"

//----------------------------------------------------------------------------------
// Bluetooth variables
//----------------------------------------------------------------------------------

#define REPORT_INTERVAL_MS 3000
#define MAX_NR_CONNECTIONS 3 

/** @brief LED Command characteristic */
#define ATT_CHARACTERISTIC_0000FF11_VALUE_HANDLE 0x0006

/** @brief HCI registration callback */
static btstack_packet_callback_registration_t hci_event_callback_registration;

#define WL_LED_GPIO       0

//----------------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------------

/**
 * @brief BTstack initialization
 * 
 * @return -1 if initialization failed, 0 otherwise
 */
int btstack_init(void);

/**
 * @brief HCI packet handler
 * 
 * @param packet_type The BLE packet type: HCI_EVENT_PACKET expected 
 * @param channel Unused
 * @param packet The event packet
 * @param size Unused
 */
void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

/**
 * @brief ATT read callback
 * 
 * @param con_handle Unused
 * @param att_handle The attribute handle
 * @param offset The data offset
 * @param buffer The buffer used to store the read data
 * @param buffer_size The buffer size
 */
uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);

/**
 * @brief ATT write callback
 * 
 * @param con_handle Unused
 * @param att_handle The attribute handle
 * @param transaction_mode Unused
 * @param offset The data offset
 * @param buffer The buffer containing the data to be written
 * @param buffer_size The buffer size
 */
int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

/**
 * @brief ATT packet handler
 * 
 * @param packet_type The BLE packet type: HCI_EVENT_PACKET expected
 * @param channel Unused
 * @param packet The event packet
 * @param size Unused
 */
void att_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

#endif // BLE_UTILS_H
