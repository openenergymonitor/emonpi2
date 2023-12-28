---
github_url: "https://github.com/openenergymonitor/emonpi2/blob/main/docs/emonpi2_emonbase_install.md"
---

# emonPi2 Install Guide

The following guide covers installation of the [emonPi2 6x input energy monitor and integrated base-station](overview.md).

![emonPi2_complete_kit.jpg](img/emonPi2_complete_kit.jpg)

**Hardware covered in this guide:**

- [emonPi2: 6 input energy monitor](overview.md)
- [emonVs: Precision voltage sensor and power supply](voltage_sensors.md)
- Up to 6 CT sensors
- RJ45 voltage sensor and power supply cable

## Quick Start

1. Clip the CT current sensors around Live OR Neutral cable of the AC circuit to be measured (not both), note CT direction K -> L (L: Load), That’s on the Line conductor, the arrow points
away from the load on the Neutral.

2. Plug the CT current sensors into emonPi2 via 3.5mm jack plugs

3. Plug emonVs RJ45 cable into emonPi2

4. Plug emonVs into mains power via a domestic wall socket

6. (optionally) If you have the RaspberryPi 4 variant of the emonPi2 a hard wired Ethernet internet/LAN connection can be connected.

7. Switch on mains socket. The display on the emonPi2 will show `emonPi2, starting..`. On the SD card side of the case the RaspberryPi 4 indicator LED should be visible through the black face plate.

8. Continue with step 5. emonBase Setup below.

```{admonition} Instructions for safe use
- Clip-on CT sensors are non-invasive and should not have direct contact with the AC mains. As a precaution, we recommend ensuring all cables are fully isolated prior to installing. If in doubt seek professional assistance.
- Do not expose to water or moisture
- Do not expose to temperate above rated operating limits
- Indoor use only
- Do not connect unapproved accessories
- Please contact us if you have any questions
```

---

## Extended version

A good place to start is to assess the location where you wish to install the emonPi2, identify the circuits that you wish to monitor using the clip-on CT sensors and decide how you would like to install the emonVs voltage sensor. There are a couple of example installations given below which may provide inspiration.

## 1. emonVs installation

There are two different ways of installing the emonVs voltage sensor:

**Using the mains plug supplied:** If you have a convenient socket near-by this will be the easiest and quickest option.<br>

**Direct installation:** The emonVs can be hardwired by a suitably competent person into a 6A or lower circuit protection device in the fuse board (consumer unit) or a 3A fused spur. The supplied emonVs mains power cable has a cross sectional area of 1.0mm<sup>2</sup>. This can provide a tidy installation if no socket is available and helps ensure higher monitoring uptime if sockets are at risk of being unplugged for use by other appliances.

The emonVs unit can be wall mounted using the brackets on the enclosure.

## 2. CT sensor installation

- Current transformers (CTs) are sensors that measure alternating current (AC).

- The emonPi2 supports a wide variety of 333mV voltage output CT sensors. We stock 20A, 50A, 100A & 200A options in the OpenEnergyMonitor shop. The physical size of these CT sensors is also roughly proportional to their current rating. For highest accuracy it's a good idea to choose CT sensors that match the rating of the circuit being monitored. As an example a 20A CT should be used for 16A or 20A circuits, a 50A CT for 32A EV chargers or 100A CT for whole house monitoring (assuming a 100A rated supply).

- CT sensors need to be clipped around the Live OR Neutral cable of the AC circuit to be measured (not both). The CT sensors have an indicated direction printed on the case `K->L`, where L is the direction of the load. That’s on the Line conductor, the arrow points away from the load on the Neutral. This will ensure the correct sign (+/-) on the power readings.

- Take care not to compress the sensor with any sideways force as this can affect the accuracy of the measurement.

- The new range of CT sensors used with the emonPi2 are all voltage output CT sensors with integrated burden resistors and so are safe to clip on to the circuits that you wish to measure before plugging into the emonPi2 if that makes installation easier.

- Note the CT sensor used on each circuit, as well as the channel number on the emonPi2 that the CT is connected to, as this may be required to calibrate the emonPi2 if you are using non standard CT ratings. It's worth making a physical note of this on a label next to the emonPi2 for future reference.


```{note}
The emonPi2 requires voltage output CT sensors, it is not compatible with the old current output YHDC (blue) 100A CT sensors - unless appropriate burden resistors are installed.
```

```{tip} 
With 6 CT sensor cables and often more cable than you need it's easy for an installation to look like a hive of wires! A little electrical trunking can go a long way to tidying it all up, allowing for excess cable to be looped back on itself.
```

## 3. emonPi2 Installation

- The emonPi2 can be wall mounted using the wall mounting kit supplied. Installation on it's side, with the aluminium side plates on the top and bottom can help reduce risk of things falling onto the sockets and can make for an easier installation in terms of CT sensor routing.

- Plug in the CT sensors, note which CT sensor is plugged into each input on the emonPi2 as each input needs to have the correct calibration applied.

- Connect the RJ45 cable (ethernet sized connector with 8 pins) from the emonVs voltage sensor and power supply to the RJ45 socket on the opposite side of the case to the CT sensor sockets. (If you have the RaspberryPi 4 variant of the emonPi2 the RaspberryPi Ethernet socket is on the same side as the CT sensors, avoid plugging the emonVs RJ45 cable into the Ethernet socket and vice versa).

### Once the emonPi2 is powered up

- **If the emonPi2 is powered up without Ethernet connected it will create a Wi-Fi Access Point** called `emonpi`. Connect to this using password `emonpi2016`. Browse the IP address [http://192.168.42.1](http://192.168.42.1) and follow the setup wizard to connect the device to your local Wi-Fi network.
- **Once connected via Wi-Fi or Ethernet** the base-station can be accessed via [http://emonpi](http://emonpi) or [http://emonpi.local](http://emonpi.local). If you are unable to locate the emonPi2 via this hostname, try using a network scanner app such as ['Fing'](https://play.google.com/store/apps/details?id=com.overlook.android.fing&hl=en_GB&gl=US) or ['Network Analyser'](https://play.google.com/store/apps/details?id=net.techet.netanalyzerlite.an&hl=en_GB&gl=US) to list the IP addresses of devices on your network or check the device list on your router.
- **The emonPi2 should now present the emoncms login screen.** Login with default username `emonsd` and password `emonsd`, these can be chnaged once logged in.

### CT calibration selection

Differant rating CT sensors e.g 100A, 50A, 20A etc, require differant preset calibrations.  

CT calibration is usually pre-configured in the shop as part of the order process, but you may wish to double check that your calibration configuration matches the sockets that you have plugged the CT sensors into at this point.

- Navigate to `Setup > Admin > Serial Config`
- Click on `Stop EmonHub` to temporarily stop the EmonHub service while we perform calibration.
- Select firmware version `emonLibDB`.
- Select serial port `/dev/ttyAMA0` and click `Connect`.
- After a couple of seconds the emonPi2 will print out its current configuration which will populate the interface (if it does not do this type `l` and click `Send` to reload the calibration details from the emonPi2 measurement board).
- Adjust the  CT rating to match the CT sensor that you have installed on each channel.
- Click on `Save Changes` to ensure that the new configuration is recorded such that it persists when you power cycle the board.
- When finished, click on `Stop Serial` to disconnect the serial configuration tool and then `Start EmonHub` to restart the EmonHub service.

![serial_config.png](img/serial_config.png)

### Setup input and feeds

**Configure inputs by navigating to Setup > Inputs.** The emonPi2 will pop up here automatically under the `EmonPi2` node name.

![emonpi2_inputs.png](img/emonpi2_inputs.png)

**The next step is to log the input data to feeds.** Inputs are just placeholders showing the latest values sent from the emonPi2, we need to create feeds if we want to record a time-series of these values. It’s possible to either manually configure each input as required, or if you just want to record everything for now and delete what you don’t need later, then you can use the pre-configured Device Template.

```{tip} 
Input configuration using the emonPi2 device template:** On the Setup > Inputs page, Click on the cog icon (top right corner) of the emonPi2 node. The 'Configure Device' window will appear, click on 'emonPi2 Standard', you may need to scroll down a little in the Devices pane to find. Click 'Save' and 'Initialize'. This will create feeds that record real power and cumulative energy for each channel, Vrms, total message count, temperatures and total pulse count. Navigate to Setup > Feeds to see these feeds.
```

**Manual input configuration:** You may only want to record specific channels or apply more complex input processing. See input processing guide.

**With feeds created, explore the data using the graph view.** Navigate to Setup > Feeds and click on a feed of interest to open the graph view. Click on the drop down time selector near the title and select the last hour. Click and drag to zoom further to see the new data coming in.

**Try creating an Emoncms App.** Click on the Apps tab. From the Available Apps list select 'My Electric' and click 'Create', Select a power feed for 'use' and cumulative kWh energy feed for 'use_kwh' and then click 'Launch App'. After a few days this will start to show a daily bar graph of consumption alongside the real-time power graph and totals. There are a wide variety of different app's to choose from depending on the application.


## Installation Examples

**Installation example**<br>
In this installation example the emonVs has been hardwired into a suitable circuit breaker in the consumer unit. 5 CT sensors are clipped around individual circuits in the consumer unit and 1 CT sensor is clipped around the main supply to the house. A short strip of trunking here made all the difference for a neater finish!

*Add installation example picture here*
