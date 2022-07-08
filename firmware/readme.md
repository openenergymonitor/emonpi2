## Firmware

The emonPi2 firmware can be edited and compiled using the [Arduino IDE](https://www.arduino.cc/) with [DxCore installed](https://github.com/SpenceKonde/DxCore).
DxCore is an Arduino core for the AVR-DB microcontroller range, developed by SpenceKonde.

### Available Firmwares

**[emonPi2FrontEndCM:](emonPi2FrontEndCM)** Single phase, 3 CT channel, continuous sampling, native RFM69 radio base-station firmware for the emonPi 2.

### Firmware examples

The following firmwares test specific functionality and may be useful when developing new firmware or modifying existing.

**[rf69n_rxtx_test:](rf69n_rxtx_test)** Example of receiving and transmitting RFM69 native format radio data.

### Pre-compiled firmware:

Command line upload:

    sudo systemctl stop emonhub
    avrdude -Cavrdude.conf -v -pavr128db32 -carduino -D -P/dev/ttyAMA0 -b115200 -Uflash:w:emonPi2FrontEndCM.ino.hex:i 
    sudo systemctl start emonhub

### How to compile and upload firmware:

https://github.com/SpenceKonde/DxCore/blob/master/Installation.md


