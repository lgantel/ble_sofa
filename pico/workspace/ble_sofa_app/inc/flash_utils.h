
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
-- File Name: flash_utils.h
-- Description: Flash utilities for persistent storage
--
-- Last update: 2025-12-14
--
-------------------------------------------------------------------------------*/

#ifndef FLASH_UTILS_H
#define FLASH_UTILS_H

#include <stdint.h>

inline uint32_t *fu_get_addr_persistent() {
    extern uint32_t ADDR_PERSISTENT[];
    return ADDR_PERSISTENT;
}

#endif // FLASH_UTILS_H
