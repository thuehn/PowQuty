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
PowQuty consists of an Ansi C deamon and a luci webinterface extension to perform power quality measurements with a dedicated USB oszilloscope connected to a wireless routers based on LEDE Linux.

Sustainable energy production and consumption are crucial for a prospering life on earth. Decentralized energy production is one of the next big challenges, which will influence the energy production in the future years. The emerging smart grids include an inherent need for communication for monitoring and control purposes in a more and more dynamic environment. One of the major challenges is monitoring the power quality parameters in a decentralized manner. The Freifunk mesh network is an outstanding example for a decentralized infrastructure that could be augmented with grid related functionalities to cope with future energy challenges. The main goal of this project is to enable power quality measurements on OpenWrt. Voltage samples from the electric socket are retrieved at the router. Next power quality parameters are calculated, and finally made available for retrieval over IP networks.


### How to install PowQuty ?
1. Add the following line to your feeds.conf in your source directory:
```
$vi feeds.conf
src-git powquty https://github.com/thuehn/powquty.git
```
2. Update the your feeds:
```
$./scripts/feeds update -a
```
3. install powqutyd from feed
```
$./scripts/feeds install powqutyd
```
4. Include powqutyd in your config
```
$make menuconfig
```
  1. choose powqutyd from: Utilities --> powqutyd
  2. save and exit menuconfig

5. Compile and install powqutyd
  1. compile:
```
$make package/powqutyd/compile
```
 2. install
```
$make package/powqutyd/install
```
At this point you should have the ipk file generated under:
```
./bin/packages/<target>/powquty/powqutyd_0.1-1_<target>.ipk
```
6. scp the ipk file to your router and install it:
```
#opkg install powqutyd_0.1-1_<target>.ipk
```
Note this package depends on the following libraries/packages, that have to be installed before installing powqutyd:
	* libmosquitto 
	* libconfig
	* kmod-usb-acm (kernel module)
When successfull the  powqutyd package will create:
	- the binary powqutyd in /usr/sbin
	- the configuration file in /etc/powqutyd/powqutyd.cfg


### How to use PowQuty ?
TBD..
