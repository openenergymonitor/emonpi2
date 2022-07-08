## Firmware

The emonPi2 firmware can be edited and compiled using the [Arduino IDE](https://www.arduino.cc/) with [DxCore installed](https://github.com/SpenceKonde/DxCore).
DxCore is an Arduino core for the AVR-DB microcontroller range, developed by SpenceKonde.

### Available Firmware

**[emonPi2FrontEndCM:](emonPi2FrontEndCM)** Single phase, 3 CT channel, continuous sampling, native RFM69 radio base-station firmware for the emonPi 2.

### Examples

The following examples test specific functionality and may be useful when developing new firmware or modifying existing.

**[rf69n_rxtx_test:](rf69n_rxtx_test)** Example of receiving and transmitting RFM69 native format radio data.

### Pre-compiled firmware:

Command line upload:

    sudo systemctl stop emonhub
    avrdude -Cavrdude.conf -v -pavr128db32 -carduino -D -P/dev/ttyAMA0 -b115200 -Uflash:w:emonPi2FrontEndCM.ino.hex:i 
    sudo systemctl start emonhub

### How to compile and upload firmware:

1\. Install DxCore<br>
https://github.com/SpenceKonde/DxCore/blob/master/Installation.md

2\. Download EmonLibCM library (avrdb branch)<br>
https://github.com/openenergymonitor/EmonLibCM/tree/avrdb

3\. Download emonEProm library (avrdb branch)<br>
https://github.com/openenergymonitor/emonEProm/tree/avrdb

4\. Download RF69n library (avrdb branch)<br>
https://github.com/openenergymonitor/rf69/tree/avrdb

5\. Install SSD1306Ascii OLED display library:<br>
https://www.arduino.cc/reference/en/libraries/ssd1306ascii



