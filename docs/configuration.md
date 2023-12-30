---
github_url: "https://github.com/openenergymonitor/emonpi2/blob/main/docs/configuration.md"
---

# Configuration

CT calibration is usually pre-configured in the shop as part of the order process but can be re-configured at any point using the Serial Config tool available from the emonPi2 local Emoncms interface.

- Navigate to `Setup > Admin > Serial Config`
- Click on `Stop EmonHub` to temporarily stop the EmonHub service while we perform calibration.
- Select serial port `/dev/ttyAMA0` and click `Connect`.
- Select firmware version `emonLibDB`.
- After a couple of seconds the emonPi2 will print out its current configuration which will populate the interface (if it does not do this type `l` and click `Send` to reload the calibration details from the emonPi2 measurement board).
- Adjust the  CT rating to match the CT sensor that you have installed on each channel.
- Click on `Save Changes` to ensure that the new configuration is recorded such that it persists when you power cycle the board.
- When finished, click on `Stop Serial` to disconnect the serial configuration tool and then `Start EmonHub` to restart the EmonHub service.

![serial_config.png](img/serial_config.png)

## Directly via serial

Alternatively it's possible to configure the emonPi2 directly via serial using the arduino serial monitor or other similar tool. 
The following details the available commands and their function.

Available commands:

- **l** list the settings
- **r** restore sketch defaults
- **s** save settings to EEPROM
- **v** show firmware version
- **z** zero energy values
- **x** exit, lock and continue
- **?** show this text again
- **d\<xx.x\>** a floating point number for the datalogging period
- **c\<n\>** n = 0 for OFF, n = 1 for ON, enable current & power factor values to serial output for calibration.
- **f\<xx\>** the line frequency in Hz: normally either 50 or 60
- **k\<x\> \<yy.y\> \<zz.z\>**
  - Calibrate an analogue input channel:
  - x = a single numeral: 0 = voltage calibration, 1 = ct1 calibration, 2 = ct2 calibration, etc
  - yy.y = a floating point number for the voltage/current calibration constant
  - zz.z = a floating point number for the phase calibration for this c.t. (z is not needed, or ignored if supplied, when x = 0)
  - e.g. k0 256.8
  - k1 90.9 2.00
- **a\<xx.x\>** a floating point number for the assumed voltage if no a.c. is detected
- **m\<x\> \<yy\>** meter pulse counting: 
  - x = 0 for OFF, x = 1 for ON
  - yy = an integer for the pulse minimum period in ms. (y is not needed, or ignored when x = 0)
  
```{tip}
The emonPi2 measurement board can be used without a RaspberryPi as an energy monitoring transmitter unit (functionally the same as the emonTx4). The RFM69 radio is turned off in the firmware by default as the RaspberryPi takes over control of the radio module, but it can be enabled when using the board as an energy monitoring transmitter. **It is not possible to both operate the emonPi2 measurement board as a transmitter and have a RaspberryPi connected as the microcontroller and RaspberryPi will conflict in their instructions to the RFM69 radio unit.**
```

**Radio settings:**
  
- **w\<x\>** turn RFM Wireless data off: x = 0 or on: x = 1 (off by default on the emonPi2 as the RaspberryPi handles the radio)
- **b\<n\>** set r.f. band n = a single numeral: 4 = 433MHz, 8 = 868MHz, 9 = 915MHz (may require hardware change)
- **p\<nn\>** set the r.f. power. nn - an integer 0 - 31 representing -18 dBm to +13 dBm. Default: 25 (+7 dBm)
- **g\<nnn\>** set Network Group nnn - an integer (OEM default = 210)
- **n\<nn\>** set node ID n= an integer (standard node ids are 1..60)

```{note}
The standard emonPi2 firmware that uses the emonLibDB electricity monitoring library does not support temperature sensing. This functionality is handled by the RasperryPi instead. It is however possible when using the board in transmitter mode (without a RaspberryPi) to load the older EmonTx4 emonLibCM based firmware that does support temperature sensing. The following setting is then available:
```

- **t\<x\>** turn temperature measurement on or off: x = 0 for OFF, x = 1 for ON

