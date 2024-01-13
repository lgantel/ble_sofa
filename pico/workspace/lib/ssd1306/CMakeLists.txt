set(PROJECT ssd1306)

# Create library
add_library(${PROJECT} INTERFACE)

# List soruces
target_sources(${PROJECT} INTERFACE 
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/ascii_bitmap.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/ssd1306.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/ssd1306.c 
)

# Pull in dependencies
target_link_libraries(${PROJECT} INTERFACE 
  pico_stdlib
  hardware_i2c
)

# Add include files
target_include_directories(${PROJECT} INTERFACE 
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_BINARY_DIR}
)
