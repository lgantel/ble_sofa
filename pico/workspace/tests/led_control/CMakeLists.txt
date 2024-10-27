cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(PROJECT led_control)

project(${PROJECT} C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Define the executable
add_executable(${PROJECT} ${PROJECT}.c)

# Pull in common dependencies
target_link_libraries(${PROJECT} pico_stdlib)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)

# Create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT})

# Add URL via pico_set_program_url
example_auto_set_url(${PROJECT})
