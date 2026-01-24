set(PROJECT ble_sofa_app)

# Initialize the SDK
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(${PROJECT} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_PLATFORM rp2040)
set(PICO_COMPILER "pico_arm_gcc")
set(CMAKE_BUILD_TYPE Release)

pico_sdk_init()

#-----------------------------------------------------------
# Setup Perl script for generating makefsdata
#-----------------------------------------------------------

find_package(Perl)
if(NOT PERL_FOUND)
    message(FATAL_ERROR "Perl is needed for generating the fsdata.c file")
endif()

#set(MAKE_FS_DATA_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/external/makefsdata.pl)
#if (NOT EXISTS ${MAKE_FS_DATA_SCRIPT})
#  file(COPY ${CMAKE_SOURCE_DIR}/scripts/makefsdata.pl DESTINATION ${CMAKE_CURRENT_LIST_DIR}/external)
#endif()

#if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${PROJECT}/my_fsdata.c)
  #message("Removing my_fsdata.c from source tree")
  #file(REMOVE ${CMAKE_SOURCE_DIR}/${PROJECT}/my_fsdata.c)

#  message("Running makefsdata script")
#  execute_process(COMMAND
#    perl ${MAKE_FS_DATA_SCRIPT}
#    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/content
#    ECHO_OUTPUT_VARIABLE
#    ECHO_ERROR_VARIABLE
#  )
#  file(RENAME content/fsdata.c my_fsdata.c)
#endif()

#-----------------------------------------------------------
# Setup WiFi credentials
#-----------------------------------------------------------

if(EXISTS ${CMAKE_SOURCE_DIR}/scripts/credentials.cmake)
    # copy it over from cmake/credentials.cmake.example
    include(${CMAKE_SOURCE_DIR}/scripts/credentials.cmake)
else()
    message(WARNING "Credentials file not found, using default values!")
    set(WIFI_SSID "SSID" CACHE INTERNAL "WiFi SSID for examples")
    set(WIFI_PASSWORD "password" CACHE INTERNAL "WiFi password for examples")
endif()

#-----------------------------------------------------------
# Define the executable
#-----------------------------------------------------------
add_executable(${PROJECT} 
  hw_mgmt.c
  pico_error_str.c
  fsdata_custom.c
  #ble_utils.c
  ${PROJECT}.c
)

# Set include directories
target_include_directories(${PROJECT} PRIVATE
    inc
)

# Set custom linker script
pico_set_linker_script(${PROJECT} ${CMAKE_SOURCE_DIR}/lscript/memmap_custom.ld)

# Set link flags
set_property(TARGET ${PROJECT} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")

# Pull in dependencies
include_directories(${CMAKE_SOURCE_DIR}/lib/relay/inc)
include_directories(${CMAKE_SOURCE_DIR}/lib/ssd1306/inc)

target_link_libraries(${PROJECT} PRIVATE
  pico_cyw43_arch_lwip_sys_freertos
  FreeRTOS-Kernel-Heap4
  pico_httpd_content
  pico_lwip_http
  pico_stdlib
  #pico_btstack_ble
  #pico_btstack_cyw43
  #pico_cyw43_arch_none
  relay
  ssd1306
)

target_compile_definitions(${PROJECT} PRIVATE
  configNUMBER_OF_CORES=1
  NO_SYS=0
  WIFI_SSID=\"${WIFI_SSID}\"
  WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

# Include GATT header
#pico_btstack_make_gatt_header(${PROJECT} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/mygatt.gatt")

# Enable usb output, Enable uart output
pico_enable_stdio_usb(${PROJECT} TRUE)
pico_enable_stdio_uart(${PROJECT} TRUE)

# Create map/bin/hex etc.
pico_add_extra_outputs(${PROJECT})

# Add URL via pico_set_program_url
example_auto_set_url(${PROJECT})

# HTTPD library content
pico_add_library(pico_httpd_content NOFLAG)
pico_set_lwip_httpd_content(pico_httpd_content INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/fs/404.html
  ${CMAKE_CURRENT_LIST_DIR}/fs/index.html
  #${CMAKE_CURRENT_LIST_DIR}/content/ssi.shtml
  #${CMAKE_CURRENT_LIST_DIR}/content/relays.shtml
)
