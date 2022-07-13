# emonPi2

The emonPi2 is an energy monitor that can monitor up to 3 AC circuits using CT sensors and the OpenEnergyMonitor voltage sensing unit. It consists of a measurement board that sits on top of a RaspberryPi. The measurement board features the Microchip AVR128DB32 microcontroller - responsible for sampling the current and voltage waveforms and calculating the power and energy values. These are then passed on to the RaspberryPi for either local data logging and visualisation or remote connectivity. The emonPi2 can also be used as a basestation to receive data from other 433Mhz radio nodes.

![board](emonPi2w.jpg)

**Features**

- 3 CT sensor inputs (333mV voltage output CT's)
- RJ11: 1-3 phase voltage input
- RJ45: Pulse and DS18B20 temperature sensing input
- Microcontroller: AVR128DB32
- MCP1501 Precision reference
- RFM69CW 433 Mhz radio
- USB-C (Required for power supply)
- I2C SSD1306 128x32 Display and push button
- RTC I2C Breakout
- Optional on-board UART

### [Hardware schematic and board files](v2.0.0-beta)

### [Firmware](firmware)

### Community & Support

- [OpenEnergyMonitor Forums](https://community.openenergymonitor.org)

OpenEnergyMonitor Shop Support: support@openenergymonitor.zendesk.com

### License

- The hardware designs (schematics and CAD files) are licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.
- The firmware is released under the GNU GPL V3 license The documentation is subject to GNU Free Documentation License
- The hardware designs follow the terms of the OSHW (Open-source hardware) Statement of Principles 1.0.

### Disclaimer

```
OUR PRODUCTS AND ASSEMBLY KITS MAY BE USED BY EXPERIENCED, SKILLED USERS, AT THEIR OWN RISK. TO THE FULLEST EXTENT PERMISSIBLE BY THE APPLICABLE LAW, WE HEREBY DISCLAIM ANY AND ALL RESPONSIBILITY, RISK, LIABILITY AND DAMAGES ARISING OUT OF DEATH OR PERSONAL INJURY RESULTING FROM ASSEMBLY OR OPERATION OF OUR PRODUCTS.

Your safety is your own responsibility, including proper use of equipment and safety gear, and determining whether you have adequate skill and experience. OpenEnergyMonitor and Megni registered partnership disclaims all responsibility for any resulting damage, injury, or expense. It is your responsibility to make sure that your activities comply with applicable laws, including copyright. Always check the webpage associated with each unit before you get started. There may be important updates or corrections. All use of the instructions, kits, projects and suggestions given both by megni.co.uk, openenergymonitor.org and shop.openenergymonitor.org are to be used at your own risk. The technology (hardware , firmware and software) are constantly changing, documentation (including build guide and instructions) may not be complete or correct.

If you feel uncomfortable with assembling or using any part of the system, return it to us for a full refund.
```
