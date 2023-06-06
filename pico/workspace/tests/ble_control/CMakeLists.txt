set(PROJECT ble_control)

# Initialize the SDK
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(${PROJECT} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_PLATFORM rp2040)
set(PICO_COMPILER "pico_arm_gcc")
set(CMAKE_BUILD_TYPE Release)

pico_sdk_init()

# Define the executable
add_executable(${PROJECT} ${PROJECT}.c)

# Pull in common dependencies
target_link_libraries(${PROJECT}  
  pico_stdlib
  pico_btstack_ble
  pico_btstack_cyw43
  pico_cyw43_arch_none
)

# Add include files
target_include_directories(${PROJECT} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Include GATT header
pico_btstack_make_gatt_header(${PROJECT} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/mygatt.gatt")

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)

# Create map/bin/hex etc.
pico_add_extra_outputs(${PROJECT})

# Add URL via pico_set_program_url
example_auto_set_url(${PROJECT})
