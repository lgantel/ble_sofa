from phew import server, connect_to_wifi
import machine
import json
from ssd1306 import SSD1306_I2C

WIDTH = 128 
HEIGHT = 32
i2c = machine.I2C(0, scl=machine.Pin(17), sda=machine.Pin(16), freq=200000)
oled = SSD1306_I2C(WIDTH, HEIGHT, i2c)

def print_oled(text, x, y, clear=False):
    if clear:
        oled.fill(0)
    oled.text(text, x, y)
    oled.show()

def timer_callback(timer):
    oled.fill(0)
    oled.show()
    oled.poweroff()

print_oled("> WiFi Sofa App ", 0, 0)

"""
WIFI_SSID = 'Livebox-81BC'
WIFI_PSWD = '9STVCtQGfH6Rhzkc5X'
"""
#
WIFI_SSID = 'Freebox-325237'
WIFI_PSWD = '4f7frqcf6bq4nrw5qvmf7v'

RELAY_UP_GPIO = 7
RELAY_DOWN_GPIO = 6

ip = connect_to_wifi(WIFI_SSID, WIFI_PSWD)
print("> Connected to IP", ip)
print_oled("> " + str(ip), 0, 8)

# Initialize the LED
led = machine.Pin("LED", machine.Pin.OUT)
led.value(0)

# Initialize the relays
relay_up = machine.Pin(RELAY_UP_GPIO, machine.Pin.OUT)
relay_down = machine.Pin(RELAY_DOWN_GPIO, machine.Pin.OUT)
relay_up.value(0)
relay_down.value(0)

print_oled("> IO Initialized", 0, 16)

# Route to control the LED status
@server.route("/api/led-control", methods=["POST"])
def ledCommand(request):
    led.value(request.data["led"])
    return json.dumps({"message": "Command sent successfully!"}), 200, {"Content-Type": "application/json"}

# Route to control the RELAY status
@server.route("/api/relay-control", methods=["POST"])
def relayCommand(request):
    print("request=", request)
    relay_val = request.data["relay"]
    relay_dir = request.data["dir"]
    if relay_dir == "UP":
        relay_up.value(relay_val)    
    elif relay_dir == "DOWN":
        relay_down.value(relay_val)
    else:
        return json.dumps({"message": "Incorrect relay dir!"}), 400, {"Content-Type": "application/json"}
    
    return json.dumps({"message": "Command sent successfully!"}), 200, {"Content-Type": "application/json"}

@server.catchall()
def catchall(request):
    return json.dumps({"message": "URL not found!"}), 404, {"Content-Type": "application/json"}

print_oled("> Server started", 0, 24)
# Turn-off OLED screen after 5 seconds
soft_timer = machine.Timer(mode=machine.Timer.ONE_SHOT, period=5000, callback=timer_callback)

server.run()
