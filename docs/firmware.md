---
github_url: "https://github.com/openenergymonitor/emonpi2/blob/main/docs/firmware.md"
---

# Firmware

The emonPi2 firmware can be edited and compiled using [PlatformIO](https://platfomrio.org/) (recommended) or [Arduino IDE](https://www.arduino.cc/) with [DxCore installed](https://github.com/SpenceKonde/DxCore). DxCore is an Arduino core for the AVR-DB microcontroller range, developed by SpenceKonde.

## Available Firmware

**[EmonPi2_DB_6CT_1phase:](https://github.com/openenergymonitor/emonpi2/tree/main/firmware/EmonPi2_DB_6CT_1phase)** Single phase, 6 CT channel, continuous sampling using the emonLibDB library, cumulative energy persisted to EEPROM, serial configuration and data output.

Pre-compiled hex: [https://github.com/openenergymonitor/emonpi2/tree/main/firmware/EmonPi2_DB_6CT_1phase/compiled](https://github.com/openenergymonitor/emonpi2/tree/main/firmware/EmonPi2_DB_6CT_1phase/compiled)

**[EmonPi2_DB_6CT_3phase:](https://github.com/openenergymonitor/emonpi2/tree/main/firmware/EmonPi2_DB_6CT_3phase)** Three phase, 6 CT channel, continuous sampling using the emonLibDB library, cumulative energy persisted to EEPROM, serial configuration and data output.

Pre-compiled hex: [https://github.com/openenergymonitor/emonpi2/tree/main/firmware/EmonPi2_DB_6CT_3phase/compiled](https://github.com/openenergymonitor/emonpi2/tree/main/firmware/EmonPi2_DB_6CT_3phase/compiled)

*Note: Firmware's EmonPi2_DB_6CT_1phase & EmonPi2_DB_6CT_3phase is the same underlying firmware as EmonTxV4_DB_3phase_6CT.*

---

**[EmonPi2_CM_3x_temperature_transmitter:](https://github.com/openenergymonitor/emonpi2/tree/main/firmware/EmonPi2_CM_3x_temperature_transmitter)** Single phase, 6 CT channel, continuous sampling using the emonLibCM library, cumulative energy persisted to EEPROM, serial configuration and data output. With temperature sensing support for use in standalone transmitter mode (not connected to a RaspberryPi that otherwise can handle temperature sensing).

Please see [EmonTx4 DS18B20 Temperature sensing & firmware release 1.5.7](https://community.openenergymonitor.org/t/emonpi2-ds18b20-temperature-sensing-firmware-release-1-5-7/23496/2) for note on performance implications.

## Updating firmware using an emonPi/emonBase (recommended)

The easiest way of updating the emonPi2 firmware is to connect it to an emonBase with a USB cable and then use the firmware upload tool available at `Setup > Admin > Update > Firmware`.

```{warning} 

**System update may be required:** If you don not see the latest firmware version as listed above in the firmware list a full system update is required first.
```

**Note: Upload via USB-C will only work if connected in the right orientation. Try turning the USB-C connector around if upload fails.** Some USB-C connectors have a smooth side on one side and jagged connection of the metal fold on the other. On the cables we have here, the smooth side should be facing up towards the top/front face of the emonPi2:

![usbc_orientation1.jpeg](img/usbc_orientation1.jpeg)

![usbc_orientation2.jpeg](img/usbc_orientation2.jpeg)

Refresh the update page after connecting the USB cable. You should now see port `ttyUSB0` appear in the 'Select port` list.

![emonsd_firmware_upload.png](img/emonsd_firmware_upload2.png)

Double check the serial port, this is likely to be 'ttyUSB0' when plugged in via USB device connected. Select 'emonPi2' from hardware.

The standard radio format is 'LowPowerLabs', if you wish to use the emonPi2 with an existing system running JeeLib classic radio format you can select the JeeLib classic radio format here.

## Upload pre-compiled using EmonScripts emonupload2 tool 

On the emonPi/emonBase ensure EmonScripts is updated to latest version then run emonupload2 tool 

    /opt/openenergymonitor/EmonScripts/emonupload2.py

Select hardware then firmware version

```
Select hardware:
  1. emonPi2
  2. emonPi2
  3. emonPi
  4. emonTx3
  5. rfm69pi
  6. rfm12pi
  7. emonTH2
  
Enter number:1

Select firmware:
1. emonPi2_DB_6CT                          1.0.1      (Standard LowPowerLabs)
```

emonupload2 tool can also be run on any other linux computer by cloning the EmonScripts repo then running the emonupload2.py python script. Python3 required 

    git clone https://github.com/openenergymonitor/EmonScripts

## Upload pre-compiled manually using avrdude

Alternatively to upload the same pre-compiled firmware via command line on emonPi / emonBase: 

    avrdude -C/opt/openenergymonitor/EmonScripts/update/avrdude.conf -v -pavr128db48 -carduino -D -P/dev/ttyUSB0 -b115200 -Uflash:w:EmonTxV4_LPL.hex:i 

Or using differant computer, ensure `avrdude.conf` has `avr128db48` entry i.e DxCore see below instructions 

    avrdude -Cavrdude.conf -v -pavr128db48 -carduino -D -P/dev/ttyUSB0 -b115200 -Uflash:w:EmonTxV4_LPL.hex:i 
    
You will need avrdude installed (tested on version 6.3-2017) and the custom DxCore avrdude.conf. This can be downloaded here: [DxCore avrdude.conf](https://raw.githubusercontent.com/openenergymonitor/emonpi2/main/firmware/avrdude.conf).

## How to compile and upload firmware

### Compile and Upload using PlatformIO (recommended)

Clone the emonTx V4 repo 

    git clone https://github.com/openenergymonitor/emonpi2
    cd emonpi2/firmware/EmonPi2_DB_6CT
    
Install PlatformIO core then to compile and upload  

    pio run -t upload

On first run PlatformIO will download automatically all the required libraries. You can also use the PlatformIO GUI. 

### Compile and Upload using Arduino IDE 

If you don’t already have the Arduino IDE it can be downloaded from here:<br>
[https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

Once you have the IDE installed, you then need to install [Spence Konde’s DxCore](https://github.com/SpenceKonde/DxCore). This can be done by first pasting the following board manager URL in Arduino IDE > File > Preferences:

    http://drazzy.com/package_drazzy.com_index.json

Then navigating to: *Tools > Boards > Boards Manager*, Select “DxCore by Spence Konde” and click Install. 

![install_dxcore.png](img/install_dxcore.png)

For more information on DxCore installation see: [https://github.com/SpenceKonde/DxCore/blob/master/Installation.md](https://github.com/SpenceKonde/DxCore/blob/master/Installation.md).

**Libraries**

Locate or create your Arduino Sketchbook directory (location found in Arduino preferences). If it doesnt already exist, create a directory called libraries in the Sketchbook directory and install the following libraries:


1\. Download EmonLibCM library (avrdb branch)<br>
[https://github.com/openenergymonitor/EmonLibCM/tree/avrdb](https://github.com/openenergymonitor/EmonLibCM/tree/avrdb)

2\. Download EmonLibDB library (main branch)<br>
[https://github.com/openenergymonitor/emonLibDB](https://github.com/openenergymonitor/emonLibDB)

2\. Download emonEProm library (avrdb branch)<br>
[https://github.com/openenergymonitor/emonEProm/tree/avrdb](https://github.com/openenergymonitor/emonEProm/tree/avrdb)

3\. Download RFM69_LPL library (main branch)<br>
[https://github.com/openenergymonitor/RFM69_LPL](https://github.com/openenergymonitor/RFM69_LPL)

4\. Download RFM69_JeeLib library (avrdb branch)<br>
[https://github.com/openenergymonitor/RFM69_JeeLib/tree/avrdb](https://github.com/openenergymonitor/RFM69_JeeLib/tree/avrdb)

5\. Download DxCore SpenceKonde OneWire library:<br>
[https://github.com/SpenceKonde/OneWire](https://github.com/SpenceKonde/OneWire)

**Compilation settings:**

With DxCore and the libraries installed the firmware should then compile. 

Under Tools, select the following configuration options:

- Select Board "AVR DB-series (Optiboot)"
- Select Chip: AVR128DB48
- Clock Speed: 24 MHz Crystal
- Bootloader serial port: UART3: TXPB0, RXPB1

Select Board "AVR DB-series (Optiboot)"

![firmware_dxcore_option.png](img/firmware_dxcore_option.png)

Select Chip: AVR128DB48

![firmware_core_option.png](img/firmware_core_option.png)

Bootloader serial port: UART3: TXPB0, RXPB1

![firmware_uart_option.png](img/firmware_uart_option.png)

