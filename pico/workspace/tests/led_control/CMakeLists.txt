cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(led_control)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Define the executable
add_executable(led_control led_control.c)

# Pull in common dependencies
target_link_libraries(led_control pico_stdlib)

# Enable usb output, disable uart output
pico_enable_stdio_usb(led_control 1)
pico_enable_stdio_uart(led_control 0)

# Create map/bin/hex file etc.
pico_add_extra_outputs(led_control)

# Add URL via pico_set_program_url
example_auto_set_url(led_control)
