#!/bin/bash

function usage() {
    echo "> Usage: $0 <app_path>"
    echo "    app_path: Path to the app to flash from the build directory"
    exit 1
}

if [ "$#" -ne 1 ]; then
    usage
fi

APP_PATH=$1
if [ ! -d "$APP_PATH" ]; then
    echo "# $APP_PATH does not exist"
    usage
fi

APP_NAME=$(basename $APP_PATH)

echo "> APP_PATH: $APP_PATH"
echo "> APP_NAME: $APP_NAME"

sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program build/$APP_PATH/$APP_NAME.elf verify reset exit"

echo "> Done"
