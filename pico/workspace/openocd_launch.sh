#!/bin/bash

sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program build/ble_sofa_app/ble_sofa_app.elf verify reset exit"
