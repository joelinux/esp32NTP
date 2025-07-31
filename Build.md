
### Usage Guidelines and Liability
This project is designed for hobbyists and developers to experiment with ESP32, GPS, and OLED display technologies.  It is not intended for use in critical applications (e.g., medical, aviation, or automotive systems) without professional validation.

- **Safety**: Ensure proper assembly to avoid electrical hazards, such as short circuits or overheating.
- **Regulatory Compliance**: The ESP32’s Wi-Fi and Bluetooth features may require certification (e.g., FCC, CE) if used in a commercial product. Users are responsible for verifying compliance.
- **Privacy**: GPS functionality must comply with local privacy laws. Do not use this project to track individuals without their consent.
- **No Liability**: The project creators are not responsible for any consequences arising from the use of this project, including hardware failures, legal violations, or damages.

# Various builds

## Recommended Setup
This is the easist option as there is minimal soldering. You will need to solder the header to the GPS module. Recommended as it makes the parts easy to swap out if something breaks. 

![ESP32 WROOM DEVKIT](/images/esp32_wroom_devkit_circuit_image.png)

## Just for fun, an esp32S2
It's easier to solder the header to the chip and then use dupont wires to connect to supporting chips. Note that the S2 is a single core but seems to work fine.
![ESP32S2](/images/esp32S2_circuit_image.png)

## esp32 D1 Mini
Small board. Same Wroom processor. It might be easier to solder the header to the chip than doing the wires.

![ESP32-D1-Mini](/images/esp32-d1-mini-circuit_image.png)
