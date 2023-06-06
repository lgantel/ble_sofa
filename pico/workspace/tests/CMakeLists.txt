if (NOT PICO_NO_HARDWARE)
    add_subdirectory(led_control)
    add_subdirectory(ble_control)
endif ()
