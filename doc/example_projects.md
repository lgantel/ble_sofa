# Example Projects

## LED Control

<img src="../images/example_projects/led_control_diagram.png" alt="LED Control Diagram" width="50%" height="15%" title="LED Control Diagram">

```bash
cmake ..
make -j4 led_control
```

## BLE Control

```bash
cmake -DPICO_BOARD=pico_w ..
make -j4 ble_control
```

1. Test detection on Linux

Scan of the available BLE devices:
```bash
sudo hcitool lescan
LE Scan ...
43:43:A2:12:1F:AC ble-control
```

2. Connection in interactive mode to the BLE Control device:
```bash
sudo gatttool -b 43:43:A2:12:1F:AC -I
[43:43:A2:12:1F:AC][LE]> connect
Attempting to connect to 43:43:A2:12:1F:AC
Connection successful
```

3. Read the primary services provided by the BLE Control device. Our custom service has the UUID 0000ff10-0000-1000-8000-00805f9b34fb. It defines a characteristic that can be written to control the LED state (UUID 0000ff11-0000-1000-8000-00805f9b34fb):
```bash
> primary
attr handle: 0x0001, end grp handle: 0x0003 uuid: 00001800-0000-1000-8000-00805f9b34fb  <-- Generic Access Profile
attr handle: 0x0004, end grp handle: 0x0006 uuid: 0000ff10-0000-1000-8000-00805f9b34fb  <-- Our control service
> char-desc 0x0001
handle: 0x0001, uuid: 00002800-0000-1000-8000-00805f9b34fb  <-- Primary service
handle: 0x0002, uuid: 00002803-0000-1000-8000-00805f9b34fb  <-- Characteristic information
handle: 0x0003, uuid: 00002a00-0000-1000-8000-00805f9b34fb  <-- Generic Access service (name)
handle: 0x0004, uuid: 00002800-0000-1000-8000-00805f9b34fb  <-- Primary service
handle: 0x0005, uuid: 00002803-0000-1000-8000-00805f9b34fb  <-- Characteristic information
handle: 0x0006, uuid: 0000ff11-0000-1000-8000-00805f9b34fb  <-- LED control custom characteristic
```

4. The value of the characteristic can be read with the char-read-hnd command:
```bash
char-read-hnd 0x0006
Characteristic value/descriptor: 00
```

5. The characteristic being a write-without-response characteristic, we use the char-write-cmd command to write on it:
```bash
# Set the LED on
char-write-cmd 0x0006 01
# Set the LED off
char-write-cmd 0x0006 00
```

## Relay Control

### Pico Dual Channel Relay HAT

Information about the relay hat can be found here:

[https://learn.sb-components.co.uk/Pico-Dual-Channel-Relay-Hat]

and there:

[https://github.com/sbcshop/Pico-dual-channel-Relay-HAT]

### Known Issues with Relays:

[https://electronics.stackexchange.com/questions/631931/ac-current-on-relay-causes-pi-pico-to-reboot-continuously]

[https://forums.raspberrypi.com/viewtopic.php?t=322133]

[https://forums.raspberrypi.com/viewtopic.php?t=64595]

[https://forums.raspberrypi.com/viewtopic.php?f=91&t=83372&p=1225448#p1225448]

### Circuit for 29v to 5v converter:

[https://electronics.stackexchange.com/questions/588770/logic-level-converter-from-29v-to-5v-schematic]
