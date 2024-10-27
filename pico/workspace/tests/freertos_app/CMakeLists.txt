set(PROJECT freertos_app)

# Initialize the SDK
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")

add_executable(${PROJECT}
    freertos_app.c
)

target_include_directories(${PROJECT} PRIVATE
    inc
)

target_link_libraries(${PROJECT} PRIVATE
    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4
    pico_stdlib
)

if(PICO_CYW43_SUPPORTED)
    # For led support on pico_w
    target_link_libraries(${PROJECT} PRIVATE
        pico_cyw43_arch_none
    )
endif()

target_compile_definitions(${PROJECT} PRIVATE
    configNUMBER_OF_CORES=1
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)

# Create map/bin/hex etc.
pico_add_extra_outputs(${PROJECT})
