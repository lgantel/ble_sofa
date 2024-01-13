set(PROJECT relay)

# Create library
add_library(${PROJECT} INTERFACE)

# List sources
target_sources(${PROJECT} INTERFACE 
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/relay.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/relay.c 
)

# Pull in dependencies
target_link_libraries(${PROJECT} INTERFACE 
  hardware_gpio
)

# Add include files
target_include_directories(${PROJECT} INTERFACE 
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_BINARY_DIR}
)
