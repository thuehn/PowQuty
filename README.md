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

# What is PowQuty ?
PowQuty consists of an Ansi C deamon and a luci webinterface extension to perform power quality measurements with a dedicated USB oszilloscope connected to a wireless routers based on LEDE Linux.

Sustainable energy production and consumption are crucial for a prospering life on earth. Decentralized energy production is one of the next big challenges, which will influence the energy production in the future years. The emerging smart grids include an inherent need for communication for monitoring and control purposes in a more and more dynamic environment. One of the major challenges is monitoring the power quality parameters in a decentralized manner. The Freifunk mesh network is an outstanding example for a decentralized infrastructure that could be augmented with grid related functionalities to cope with future energy challenges. The main goal of this project is to enable power quality measurements on OpenWrt. Voltage samples from the electric socket are retrieved at the router. Next power quality parameters are calculated, and finally made available for retrieval over IP networks.


# How to install Powquty ?
* Add the following line to your feeds.conf in your source directory:
```
$vi feeds.conf
src-git powquty https://github.com/thuehn/powquty.git
```
* Update the your feeds:
```
$./scripts/feeds update -a
```
* install powqutyd from feed
```
$./scripts/feeds install powqutyd
```
* Include powqutyd in your config
```
$make menuconfig
```
  * choose powqutyd from: Utilities --> powqutyd
  * save and exit menuconfig

* Compile and install powqutyd
  * compile:
```
$make package/powqutyd/compile
```
  * install
```
$make package/powqutyd/install
```
At this point you should have the ipk file generated under:
```
./bin/packages/<target>/powquty/powqutyd_0.1-1_<target>.ipk
```
* scp the ipk file to your router and install it:
```
#opkg install powqutyd_0.1-1_<target>.ipk
```
Note this package depends on the following libraries/packages, that have to be installed before installing powqutyd:
* libmosquitto 
* libconfig
* kmod-usb-acm (kernel module)

When successfull the  powqutyd package will create:
* the binary powqutyd in /usr/sbin
* the configuration file in /etc/powqutyd/powqutyd.cfg


# How to use PowQuty ?

## powquty configuration
Before running powqutyd you need to configure it. 

### Configure the USB connection settings
#### TTY-Device configuration
Powqutyd needs to read the measurement samples from the USB oscilloscope. The USB oscilloscope has to be plugged to the router before running powqutyd.
The USB oscilloscope implements the USB Communication Device Class (CDC) device specification. This means that the kernel module kmod-usb-acm will recognize the USB oscilliscope once plugged and will create a tty device probably under /dev/ttyACM0. Depending on your setup this could be different. Check your system logs after plugging the USB oscilloscope to find out the actual path of the tty-device on your setup and adjust the path in the config file of powqutyd (/etc/powqutyd/powqutyd.cfg) accordingly.
Note: if the tty-device is not set right the powqutyd will not start!

```
// the device_tty confis is the path to the tty device created by cdc-acm driver
// device_tty = "/dev/ttyACM0";
```

### Configure the MQTT-Broker settings

#### MQTT-Broker
Powqutyd will send the calculated power quality parameters using MQTT-protocol to an MQTT-broker. This means that powqutyd requires IP connectivity between your router and the MQTT-broker.
Of course this is given if you set up an MQTT broker on your router itself, but this is not a requirement, as long as the router has an IP connectivity to an MQTT-broker. 
For testing purposes we used mosquitto on the router as MQTT-broker. Depending on your setup you need to adjust the mqtt_host config option in your /etc/powqutyd/powqutyd.cfg accordingly. 
The mqtt_host config option is a string that could contain either the IP-address of the Fully Qualified Domain Name (FQDN) of the MQTT-broker.
Note: at the current state, the MQTT-client implemeted by powqutyd uses the port 1883 with no SSL support. 

```
// the mqtt_host is the IP-address or URL to the MQTT broker who receives the publish messages of powqutd
// mqtt_host = "localhost";
```

#### MQTT-topic
Furthermore the powqutyd's MQTT-client is an publish only client, thus it will not subscribe and has no will. Nonetheless the topic under which the powqutyd's MQTT-client publishes needs to be set. This can be done by adjusting the mqtt_topic config option in your /etc/powqutyd/powqutyd.cfg accordingly.
```
// the mqtt_topic is the topic under which powquty will publish the mesurement results
// the Format is the following device_uuid,timestamp,3,RMS_Voltag_RMS_Frequency,H3,H5,H7,H9,H11,H13,H15
// mqtt_topic = "devices/update";
```

#### Device unique ID
Powqutyd sends three types of messages to the MQTT-broker: 
 * msg_device_online 
 * msg_device_data
 * msg_device_offline

These messages are explained below, yet all of them use a common setting which is a (universally) unique id for the devices that communicate with the same MQTT-Broker. This way the MQTT-broker can differenciate between the messages it receives. This device-unique-id is set by the config option dev_uuid
```
// the dev_uuid sets the device name used in the MQTT-publish messages
// dev_uuid = "BERTUB001"
```

### (Optional) configure whether or not to print result to stdout
It is possible to print the results messages that powqutyd sends to the MQTT-Broker to stdout. This can be set by the option powqutyd_print. If set to 0 (zero) powqutyd will not print the result to stdout.
```
// This option, if activated, will print the results published to the MQTT broker to stdout
// Setting this option to 0 (zero) will desactivate stdout printing. Setting it to any other int value will activate stdout printing.
// powqutyd_print = 0;
```

## Running powqutyd
Once all the configuration above are done powqutyd can be started by typing:
```
powqutyd &
```
to your terminal.

# powqutyd messages
Powqutyd sends three types of messages to the MQTT-broker: 
 * msg_device_online 
 * msg_device_data
 * msg_device_offline
### msg_device_online 
Once powqutyd is started it sends msg_device_online message that signalize to the MQTT-Broker that the device is online. Here we see an example
```
BERTUB001,1458689325895,1,0.1,0.1,029
```
It is a string with the Format: DEV_UUID,Timestamp,1,Version
### msg_device_offline 
when powqutyd ends it sends msg_device_offline message that signalize to the MQTT-Broker that the device is offline. Here we see an example
```
BERTUB001,1458689329483,0
```
It is a string with the Format: DEV_UUID,Timestamp,0
### msg_device_data
Once powqutyd has calculated the RMS Voltage in Volts, the actual Frequency in Hz and Harmonics coefficients, (H3 - to - H15) it publishes them to the MQTT-broker. Here we see an example:
```
BERTUB001,1458689327099,3,223.732391,49.973961,0.926936,1.542370,2.207536,1.318457,1.243623,0.722359,2.283980
```
It is a string with the Format: DEV_UUID,Timestamp,3,RMS_Voltag_RMS_Frequency,H3,H5,H7,H9,H11,H13,H15
