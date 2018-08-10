```
@@@@@@@    @@@@@@   @@@  @@@  @@@   @@@@@@   @@@  @@@  @@@@@@@  @@@ @@@  
@@@@@@@@  @@@@@@@@  @@@  @@@  @@@  @@@@@@@@  @@@  @@@  @@@@@@@  @@@ @@@  
@@!  @@@  @@!  @@@  @@!  @@!  @@!  @@!  @@@  @@!  @@@    @@!    @@! !@@  
!@!  @!@  !@!  @!@  !@!  !@!  !@!  !@!  @!@  !@!  @!@    !@!    !@! @!!  
@!@@!@!   @!@  !@!  @!!  !!@  @!@  @!@  !@!  @!@  !@!    @!!     !@!@!   
!!@!!!    !@!  !!!  !@!  !!!  !@!  !@!  !!!  !@!  !!!    !!!      @!!!   
!!:       !!:  !!!  !!:  !!:  !!:  !!:!!:!:  !!:  !!!    !!:      !!:    
:!:       :!:  !:!  :!:  :!:  :!:  :!: :!:   :!:  !:!    :!:      :!:    
 ::       ::::: ::   :::: :: :::   ::::: :!  ::::: ::     ::       ::    
 :         : :  :     :: :  : :     : :  :::  : :  :      :        :     
```

### What is PowQuty ?

The PowQuty package is a software collection of tools to collect, analyse and display voltage measurements with the WeSense USB osciloscope attached to any router that runs on OpenWRT Linux. PowQuty consists of an Ansi C daemon and a Luci web-interface extension. The goal of this development is the proivide an efficient and low-cost solution to monitor the Power-Quality state of the local low-voltage grid.

#### The big picture:
Sustainable energy production and consumption are crucial for a prospering life on earth. Decentralized energy production based on renewable sources increases and happens in the low-voltage grid. That contradicts the "old" way of energy production: big power plants based on gas, cole or nuclear sources produce our energy. The design of todays power grid topologies was based on the fact to distibute and hence transmit power from those big sources all the the way down to each single consumer. From a few important and therfore good monitored high voltage lines to more midle voltage lines down to a large low voltage grid. This one directional power flow assupmtion changes with every new houshold producing energy and pumping it to the low-voltage power grid that was never design nor monitored for that purpose.
So the largest part in terms of power lines in the ground - the low voltage grid - is used to connect power producers from renewabbles ... a power flow in the opposite direction with unknown effects to the overall power grid we have used before.
So what is the impact of this new bi-directional power flows in the unmonitored low-voltage grid? 
PowQuty provides a first step to start to monitor the power quality and hence the state of low voltage grids with renewable sources in a decentralized manner. The energy system transformation with its effects to the deployed low-voltage grid structurtes can be monitored and analysed with PowQuty.

### PowQuty Measurement Features

* collect and store raw voltage samples with a WeSense USB osciloscope at __10kHz resolution__
* any embedded system based on __OpenWRT Linux__ is sufficient to monitor voltage on a regular power plug
* statistical post-processing of this raw voltage measurements to aggragate it to __power-quality values per second: RMS voltage, frequency and harmonics up to H15__
* provide a __Mosquitto MQTT__ based publish and subscribe data service to distribute the power-quality data
* further post-processing to trigger events once the measured power quality does not comply with the __EU-norm EN50160__

### A typical setup for low-voltage grid monitorinig with PowQuty 

![powquty setup](https://user-images.githubusercontent.com/1880886/43984817-4deb4042-9d03-11e8-9a17-2aec5ba885f8.png)

### Example of PowQuty LUCI web interface view

![alt tag](https://cloud.githubusercontent.com/assets/1880886/23344150/98904e36-fc77-11e6-8fc0-ebbea06efe2b.png)


### How to install PowQuty (under Linux OpenWrt [https://openwrt.org])
1. Add the following line to your feeds.conf in your OpenWrt source directory:
```
src-git powquty https://github.com/thuehn/powquty.git
```
2. install powqutyd with:
```
$./scripts/feeds install powqutyd
```
3. add powqutyd to your OpenWrt config within make menuconfig -> Utilities --> powqutyd
4. rebuild your OpenWrt image
5. flash a new image or install the powquty with the help of opkg

Note this package depends on the following libraries/packages, that have to be installed before installing powqutyd:
* libmosquitto
* libconfig
* kmod-usb-acm (kernel module)

When successful the  powqutyd package will create:
* the binary powqutyd in /usr/sbin
* the configuration file in /etc/powqutyd/powqutyd.cfg


### How to show PowQuty's power quality plots in your local routers web-interface

1. point your Linux OpenWrt feeds.conf to our PowQuty repository by adding the following line to your feeds.conf:
```
src-git powquty https://github.com/thuehn/powquty.git
```
2. trigger a `feeds update`
3. select our luci_app_powquty from `make menuconfig` under menu LUCI
4. rebuild your OpenWrt image
5. flash a new image or install the luci_app_powquty with the help of opkg


## How to use PowQuty ?

### powquty configuration
Before running powqutyd you need to configure it.

### Configure the USB connection settings
#### TTY-Device configuration
Powqutyd needs to read the measurement samples from the USB oscilloscope. The USB oscilloscope has to be plugged to the router before running powqutyd.
The USB oscilloscope implements the USB Communication Device Class (CDC) device specification. This means that the kernel module kmod-usb-acm will recognize the USB oscilloscope once plugged and will create a tty device probably under /dev/ttyACM0. Depending on your setup this could be different. Check your system logs after plugging the USB oscilloscope to find out the actual path of the tty-device on your setup and adjust the path in the config file of powqutyd (/etc/config/powquty) accordingly.
Note: if the tty-device is not set right the powqutyd will not start!

```
// the device_tty confis is the path to the tty device created by cdc-acm driver
// device_tty = "/dev/ttyACM0";
```
#### Possible general configuration options
```
device_tty          'usb device to use'         //max 31 character
dev_lat             'set latitude'              //max 31 character
dev_lon             'set longitude'             //max 31 character
dev_acc             'set gps accuracy'          //max 31 character
dev_alt             'set altitude'              //max 31 character
poquty_path         'path/to/logfile'           //max 511 character
powquty_event_path  'path/to/event/logfile'     //max 511 character
max_log_size_kb     'set max logfile size(kb)'  //default 4096
```

### Configure the MQTT-Broker settings

#### MQTT-Broker
Powqutyd will send the calculated power quality parameters using MQTT-protocol to an MQTT-broker. This means that powqutyd requires IP connectivity between your router and the MQTT-broker.
Of course this is given if you set up an MQTT broker on your router itself, but this is not a requirement, as long as the router has an IP connectivity to an MQTT-broker.
For testing purposes we used mosquitto on the router as MQTT-broker. Depending on your setup you need to adjust the mqtt_host config option in your /etc/config/powquty accordingly.
The mqtt_host config option is a string that could contain either the IP-address of the Fully Qualified Domain Name (FQDN) of the MQTT-broker.
Note: at the current state, the MQTT-client implemented by powqutyd uses the port 1883 with no SSL support.

```
// the mqtt_host is the IP-address or URL to the MQTT broker who receives the publish messages of powqutd
// mqtt_host = "localhost";
```

#### MQTT-topic
Furthermore the powqutyd's MQTT-client is an publish only client, thus it will not subscribe and has no will. Nonetheless the topic under which the powqutyd's MQTT-client publishes needs to be set. This can be done by adjusting the mqtt_topic config option in your /etc/powqutyd/powqutyd.cfg accordingly.
```
// the mqtt_topic is the topic under which powquty will publish the mesurement results
// mqtt_topic = "devicaes/update";
```

#### Message Format
The current message format is a Json string with the following elements:
```
{"acc":0,
 "alt":0,
 "id":"BERTUB001",
 "lat":52.520008,
 "lng":13.404954,
 "metadata": { //optional object
   "comment": "",
   "id": "",
   "operator": "",
   "phase": "",
   "reason": "",
   "type": ""
 },
 "pkg":"0",
 "t5060": { //optional object
   "f": 50.017506,
   "u": 230.599289,
   "h3":  1.157205,
   "h5":  0.515359,
   "h7":  1.126879,
   "h9":  0.951026,
   "h11":  0.527944,
   "h13":  0.481196,
   "h15":  0.302587
 },
 "t1012": { //optional object
   "f":  50.017506,
   "u":  230.599289
 },
 "utc":"2018-03-14 14:45:37.282"
}
```

#### Possible configuration file options
```
mqtt_host       'set_your_hostname' //max 31 character
mqtt_topic      'set_mqtt_topic'    //max 31 character
mqtt_uname      'username'          //max 31 character
mqtt_pw         'password'          //max 31 character
send_t5060_data '1'                 //1 or 0 -> send voltage, frequency and harmonics
send_t1012_data '0'                 //1 or 0 -> send voltage and frequency

use_metadata    '0'                 //1 or 0 -> send meta-data block
meta_phase      'set phase'         //max 31 character
meta_id         'set an id'         //max 31 character
meta_comment    'set a comment'     //max 63 character
meta_operator   'set node operator' //max 63 character
meta_reason     'set a reason'      //max 63 character
meta_type       'set a node type'   //max 63 character
```
#### Device unique ID
Powqutyd sends three types of messages to the MQTT-broker:
 * msg_device_online
 * msg_device_data
 * msg_device_offline

These messages are explained below, yet all of them use a common setting which is a (universally) unique id for the devices that communicate with the same MQTT-Broker. This way the MQTT-broker can differentiate between the messages it receives. This device-unique-id is set by the config option dev_uuid
```
// the dev_uuid sets the device name used in the MQTT-publish messages
dev_uuid 'BERTUB001'        //max 31 character
```

### (Optional) configure whether or not to print result to stdout
It is possible to print the results messages that powqutyd sends to the MQTT-Broker to stdout. This can be set by the option powqutyd_print. If set to 0 (zero) powqutyd will not print the result to stdout.
```
// This option, if activated, will print the results published to the MQTT broker to stdout
// Setting this option to 0 (zero) will desactivate stdout printing. Setting it to any other int value will activate stdout printing.
powqutyd_print '0'          //1 or 0
```

## Running powqutyd
Once all the configuration above are done powqutyd can be started by typing:
```
powqutyd &
```
to your terminal.

Upon installation PowQuty will create an init file ```/etc/init.d/powqutd```
and powqutyd will be enabled and started.
Stop it:
```
/etc/init.d/powqutyd stop
```
configure it for your setting:
```
vim /etc/config/powquty
```
and restart it:
```
/etc/init.d/powqutyd start
```

## Command line options
Print raw samples
```
powqutyd -r
```
Print raw samples to file
```
powqutyd --rawfile <Path/to/file>
```
Read samples from file
```
powqutyd -f <Path/to/file>
```

## Do you want to contribute ?
Everybody can participate, and any help is highly appreciated.
Feel free to send pull requests or open a new issue via GitHub.
- testing PowQuty and its power quality measurements in your power grid environment
- reviewing patches

### Supporters and Developers
- Nadmin el Sayed from TU-Berlin
- Stefan Venz from HTW Berlin
- Thomas Huehn from TU-Berlin

### How to reference to PowQuty ?
Just use the following bibtex :

To be added by Nadim
