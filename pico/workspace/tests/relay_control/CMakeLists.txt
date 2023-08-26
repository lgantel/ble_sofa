# Define the executable
add_executable(relay_control 
  relay.h relay.c 
  relay_control.c
)

# Pull in common dependencies
target_link_libraries(relay_control pico_stdlib)

# Enable usb output, disable uart output
pico_enable_stdio_usb(relay_control 1)
pico_enable_stdio_uart(relay_control 0)

# Create map/bin/hex file etc.
pico_add_extra_outputs(relay_control)

# Add URL via pico_set_program_url
example_auto_set_url(relay_control)
