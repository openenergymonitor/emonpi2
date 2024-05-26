---
github_url: "https://github.com/openenergymonitor/emonpi2/blob/main/docs/firmware.md"
---

# Firmware

**Updated: 14th May 2024**<br>

There are currently 5 pre-compiled firmware options for the emonPi2 available via both the Emoncms firmware upload tool and the command line `emonupload.py` firmware tool. 

**To choose the right firmware, there are 2 main questions:**

1. Is the application 1-phase, 3-phase or current only?<br>

2. Are you using a 6 channel expander for a total of 12 CT channels?<br>

*Note: Temperature sensing is handled by the RaspberryPi separately and so is not limited by firmware choice.*

Select the right firmware from the list that matches the requirements of your application:

| ID | Firmware | Voltage | CTs | Pulse|
| --- | --- | --- | --- | --- |
| 1 | emonPi2_DB_6CT_1phase | 1-ph | 6 | 1* |
| 2 | emonPi2_DB_6CT_3phase | 3-ph | 6 | 1* |
| 3 | emonPi2_DB_12CT_1phase | 1-ph | 12 | 3* |
| 4 | emonPi2_DB_12CT_3phase | 3-ph | 12 | 3* |
| 5 | emonPi2_CM_6CT | 1-ph or current only | 6 | 1 |

*The pulse sensor * denote standard firmware support for the number of pulse sensors given. Firmware modification can extend the number of pulse sensors to 3 on all emonLibDB based firmwares.*

- All firmwares include serial configuration of radio, voltage and current channel calibration values.

- **_DB:** EmonLib**DB** electricity monitoring library.<br>

- **_CM:**  EmonLib**CM** electricity monitoring library.

- **3-phase firmwares phase allocation:** follows the following pattern: 

  - CT1: phase 1
  - CT2: phase 2
  - CT3: phase 3
  - CT4: phase 1
  - CT5: phase 2
  - CT6: phase 3
  - continued for 12 CT expansion board...

- **Current only:** Supported by firmware 5 only, can be used with installations without an emonVs voltage sensor. These firmwares uses the older emonLibCM electricity monitoring library that has a fallback option to an assumed RMS voltage value if no AC voltage signal is detected.

- **Temperature sensing:** Is handled by the RaspberryPi separately via the emonHub software and so is not limited by firmware choice. While there is no specific limit on the number of temperature sensors that can be connected we have only tested up to 6 temperature sensors at any one time.

- **Pulse sensing** is available on the 3 pin analog input terminal as default for all 6 CT firmwares but is disabled by default on the 12 CT firmwares as the analog input is used for CT 12. If temperature sensing is not required it is possible to configure pulse sensing on the terminal blocks connections typically used for temperature sensing.

- **Analog input:** Reading from the analog input can be enabled for all firmware variants via a `#define` compile option when compiling from source.

**Base firmwares**<br>
These are now built from a set of base firmware's common to all AVR-DB hardware variants (emonTx4, emonTx5 and emonPi2) available in the [avrdb_firmware repository](https://github.com/openenergymonitor/avrdb_firmware/)

- Firmwares 1 & 2 compiled from the `emon_DB_6CT` base firmware.
- Firmwares 3 & 4 are compiled from the `emon_DB_12CT` base firmware.
- Firmware 5 is compiled from the `emon_CM_6CT_temperature` base firmware.

---

## Updating firmware

The easiest way of updating the emonPi2 firmware is to use the firmware upload tool.

1. In your local emonPi2 web interface, navigate to: `Setup > Admin > Update > Firmware`.
2. Select serial port `ttyAMA0` and then select `emonPi2` from hardware.
3. Select firmware variant as required.

![emonPi2_emoncms_firmware_update.png](img/emonPi2_emoncms_firmware_update.png)

## Upload pre-compiled using EmonScripts emonupload2 tool 

On the emonPi/emonBase ensure EmonScripts is updated to latest version then run emonupload2 tool 

    cd /opt/openenergymonitor/EmonScripts
    ./emonupload2.py

Select hardware then firmware version

```
Select hardware:
  1. emonTx5
  2. emonPi2
  3. emonTx4
  4. emonPi
  5. emonTx3
  6. rfm69pi
  7. rfm12pi
  8. emonTH2
  9. JeeLink
Enter number:2

Select firmware:
1. emonPi2_DB_6CT_1phase                   2.1.0      (Standard LowPowerLabs)
2. emonPi2_DB_6CT_3phase                   2.1.0      (Standard LowPowerLabs)
3. emonPi2_DB_12CT_1phase                  1.2.0      (Standard LowPowerLabs)
4. emonPi2_DB_12CT_3phase                  1.2.0      (Standard LowPowerLabs)
5. emonPi2_CM_6CT                          1.6.0      (Standard LowPowerLabs)
Enter number:

```

emonupload2 tool can also be run on any other linux computer by cloning the EmonScripts repo then running the emonupload2.py python script. Python3 required 

    git clone https://github.com/openenergymonitor/EmonScripts

## Upload pre-compiled manually using avrdude

Alternatively to upload the same pre-compiled firmware via command line on emonPi / emonBase: 

    avrdude -C/opt/openenergymonitor/EmonScripts/update/avrdude.conf -v -pavr128db48 -carduino -D -P/dev/ttyUSB0 -b115200 -Uflash:w:emonPi2_DB_6CT_1phase_v2_1_0.ino.hex:i 

Or using different computer, ensure `avrdude.conf` has `avr128db48` entry i.e DxCore see below instructions 

    avrdude -Cavrdude.conf -v -pavr128db48 -carduino -D -P/dev/ttyUSB0 -b115200 -Uflash:w:emonPi2_DB_6CT_1phase_v2_1_0.ino.hex:i 
    
You will need avrdude installed (tested on version 6.3-2017) and the custom DxCore avrdude.conf. This can be downloaded here: [DxCore avrdude.conf](https://raw.githubusercontent.com/openenergymonitor/EmonScripts/master/update/avrdude.conf).

## How to compile and upload firmware

### Compile and Upload using PlatformIO (recommended)

Clone the `avrdb_firmware` repo 

    git clone https://github.com/openenergymonitor/avrdb_firmware
    cd avrdb_firmware/emon_DB_6CT
    
Open `emon_DB_6CT.ino` in an editor and change `#define EMONTX5` to `#define EMONPI2`. 

Change any other compile options as required e.g:

- to enable analog read uncomment `#define ENABLE_ANALOG`.
    
Install PlatformIO core then to compile and upload:

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

**Open and edit the AVR-DB firmware**

Clone the `avrdb_firmware` repo 

    git clone https://github.com/openenergymonitor/avrdb_firmware

Open the base firmware that you wish to use in the Arduino IDE e.g `emon_DB_6CT.ino`.

Change `#define EMONTX5` to `#define EMONPI2`. 

Change any other compile options as required e.g:

- to enable analog read uncomment `#define ENABLE_ANALOG`.

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

