# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Define the executable
add_executable(read_ina219 read_ina219.c)

# Pull in common dependencies
target_link_libraries(read_ina219 
  pico_stdlib
  hardware_i2c
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(read_ina219 1)
pico_enable_stdio_uart(read_ina219 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(read_ina219)

# Add URL via pico_set_program_url
example_auto_set_url(read_ina219)
