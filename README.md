# esp32NTP

This project creates a NTP (Network Time Protocol) server using various 
ESP32 chipsets and provides feedback with various displays like SSD1306, ST7789, or 
Cheap Yellow Display.

The NTP server can be used by your local devices to get the date and time without
having to pull it from an Internet NTP server. 

An ATGM336H GPS module is used to get the time from GPS satellites. This GPS module supply a PPS signal that allows for time to be set within milliseconds. 

The system will report as a Stratum 10 server with the first time sample from the satellites, but
will report as a Stratum 2 when it gets a PPS lock from the GPS Module.

It can't really be called a Stratum 1 server because the ESP32 connects over wifi.
This is convenient, but it lowers the accuracy. But the time results are 
usually in the low milliseconds. 

## Why do this?
Good question. I hope to have an answer here shortly.

## Features

- Connects ESP32 to Wi-Fi
- Retrieves current time from GPS module 
- Displays and/or uses synchronized time in your application using NTP protocol
- Has custom commands to get stats and toggle display from CLI using ntpclient
- Has a Web interface to toggle display, reboot and flash firmware.


## Requirements

- A supported ESP32 chip. 
- VS Code with PlatformIO to compile program
- Wi-Fi credentials
- Ability to pick up GPS satellites. Might not work in some buildings.

## Reccommended Hardware
- [Teyleten Robot ATGM336H GPS+BDS Dual-Mode Module](https://www.amazon.com/dp/B09LQDG1HY)
- [ELEGOO 3PCS 0.96 Inch OLED](https://www.amazon.com/dp/B0D2RMQQHR)
- [ESP-WROOM-32 ESP32 ESP-32S Development Board](https://www.amazon.com/dp/B08D5ZD528)

## Sample Hardware
Below are some sample hardware that has worked at the time of development. 
- [Teyleten Robot ATGM336H GPS+BDS Dual-Mode Module](https://www.amazon.com/dp/B09LQDG1HY)
- [OLED I2C IIC Display Module 12864](https://www.amazon.com/dp/B09C5K91H7)
- [D1 Mini ESP32 ESP-WROOM-32](https://www.amazon.com/dp/B0D8Y6B9C5)

## Getting Started

1. Clone this repository:
    ```sh
    git clone https://github.com/joelinux/esp32NTP.git
    ```
2. Open the project in VS Code
3. Compile the code
3. Upload to your ESP32 board

## Usage

On initial boot on a new chip, it will be in Access Point mode to allow you to configure the wifi. Do the following:

1. On a PC or tablet, connect to NTPS1 using password of 12345678
2. Use a browser to go to http://192.168.5.1
3. Put in your Wifi SSID,  password, and create an admin password.
4. The ESP32 will reboot and if entered correctly, will connect to your Wifi.

If you entered wrong information, you can Reset the device using the Reset PIN.
The pin is different for each chip. We will document a few different setups.

If everything works properly, the ESP32 will connect to your Wi-Fi and 
try to synchronize time with GPS sattellites
automatically if it can pick up a GPS signal. The display will provide you with
some infomation. If all you see is "Starting System", it cannot pick up a satellite.
You can get some debug information from the USB serial port to see what it is
doing.


## License
This project is licensed under the GNU General Public License v2. See the [LICENSE](LICENSE) file for details.
