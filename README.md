# ESP8266 Oven Controller
### Overview
This is an oven controller based on the ESP8266 module and Arduino framework.
It uses a rotary encoder for input and 128x64 OLED as a display.

### Configuration (config.h)
Copy `config_example.h` to `config.h` in the same directory as the sketches and modify at least `SSID` and `KEY`.

### Other options
| Name (in config.h) | Default | Type | Description |
|--------------------|---------|------|-------------|
| SSID               |"your_ssid" | String | Wifi Network name (SSID) |
| KEY                |"your_sup3r_s3cur3_p4ssw0rd" | String | WiFi Password/Key |
| OTA_PASSWORD       | "your_ota_password" | String | OTA password used to update from Arduino IDE over the air |
| OTA_NAME           | "ESP8266Oven" | String | Name of device (should show up in Arduino IDE) |
| USE_SSD1306        | -       | Define | Use SSD1306 display, recommended. Display code stubbed out if not defined |
| RINGBUF_LEN        | 480 | Integer | Length of Ring Buffer for measurements (depends on available RAM, default was found to be sensible) |
| SAMPLE_INTERVAL    |15000 | Integer | Milliseconds between taking measurements (Default 15s) |
| STATE_TIMEOUT      |300000| Integer | State timeout (ms), go back to status screen after this time (Default 30s) |
| PID_INTERVAL       | 1000 | Integer | Time (ms) between PID loop executions (Default 1s) |
| START_TEMP         | 20   | Integer | Default starting temperature (Celsius) when setting via rotary encode (Default 20°C) |
| DEBOUNCE_TIME      | 50   | Integer | Debounce time (ms), this should be fine as default |
| BUTTON             | D0   | Arduino data pin | Button attached to this arduino pin |
| RELAY              | D8   | Arduino data pin | Element relay pin |
| TC_DO              | D5   | Arduino data pin | Thermocouple data pin |
| TC_CS              | D6   | Arduino data pin | Thermocouple chip select |
| TC_CLK             | D7   | Arduino data pin | Thermocouple clk pin |
| BANDGAP            | 5    | Integer | Hysteresis for on/off switching |
| HEAT_OFFSET        | 15   | Integer | If temp goes below setpoint-HEAT_OFFSET, activate element|
| PREHEAT_DIFF_THRESHOLD |35| Integer | If within PREHEAT_DIFF_THRESHOLD degrees of setpoint switch to PID mode, was an attempt to speed up pre-heating |


#### Hardware
- ESP8266 module [Eg: NodeMCU v3](https://www.aliexpress.com/item/ESP8266-CH340G-CH340-G-NodeMcu-V3-Lua-Wireless-WIFI-Module-Connector-Development-Board-CP2102-Based-ESP/32893723423.html)
- SSD1306 based OLED display [Eg: SSD1306 128x64 display](https://www.aliexpress.com/item/0-96-inch-IIC-Serial-Yellow-Blue-OLED-Display-Module-128X64-I2C-SSD1306-12864-LCD-Screen/32902463963.html)
- Rotary encoder [Eg: 20 step rotary encoder](http://www.communica.co.za/Catalog/Details/P0539677265)
- MAX31855 + K-Type Thermocouple [Eg: MAX31855 Module K Type Thermocouple](https://www.aliexpress.com/item/MAX31855-Module-K-Type-Thermocouple-temp-Sensor-new-0-800-Degrees-Temperature-measurement-module/32747807971.html)

#### Features
- Display time and temperature on status screen
- Temperature control
- Timer (turn off heating element after set amount of minutes)
- Basic web interface showing set point and current temperature
- REST interface to set PID parameters if used, and switch between "dumb" and PID mode.
- Simple rotary encoder controlled based UI
