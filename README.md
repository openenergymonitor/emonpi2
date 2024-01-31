# emonPi v2

The emonPi2 is an energy monitor that can monitor up to 6 AC circuits using CT sensors and the OpenEnergyMonitor emonVs voltage sensing unit. It consists of a measurement board that sits on top of a RaspberryPi. The measurement board features the Microchip AVR128DB348 microcontroller - responsible for sampling the current and voltage waveforms and calculating the power and energy values. The RaspberryPi provides full local data logging and visualisation capability. The emonPi2 can also be used as a basestation to receive data from other 433Mhz radio nodes.

![board](docs/img/emonPi2_board.png)

**Features**

- 6 CT sensor inputs (333mV voltage output CT's)
- RJ45: 1-3 phase voltage input
- Pulse and temperature sensing input
- Micro: AVR128DB48
- MCP1501 Precision reference
- RFM69CW radio
- USB-C
- I2C 128x64 SSD1306 Display
- RTC I2C Breakout
- Optional on-board UART

### [Hardware schematic and board files](hardware)

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
