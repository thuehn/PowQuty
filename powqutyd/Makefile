#
# Copyright (C) 2016 LEDE project (Neez El Sayed)
#
# This is free software, licensed under the GNU General Public License v3.

include $(TOPDIR)/rules.mk

PKG_NAME:=powqutyd
PKG_VERSION:=0.1
PKG_RELEASE:=1
PKG_MAINTAINER:=Neez <neez.el.sayed@gmail.com>
PKG_LICENSE:=GPL-3.0+
PKG_LICENSE_FILES:=LICENSE

PKG_CONFIG_DEPENDS:= CONFIG_POWQUTYD_NO_MQTT

include $(INCLUDE_DIR)/package.mk

define Package/powqutyd/config
menu "Configuration"
	depends on PACKAGE_powqutyd

config POWQUTYD_NO_MQTT
	bool
	default n
	prompt "Compile without MQTT/Mosquitto"

config POWQUTYD_NO_SLACK
	bool
	default n
	prompt "Compile without Slack notification support"

endmenu
endef


define Package/powqutyd
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+libconfig +kmod-usb-acm \
		 +libuci \
		 +!POWQUTYD_NO_MQTT:libmosquitto \
		 +!POWQUTYD_NO_SLACK:libcurl
	MAINTAINER:=El Sayed
	TITLE:=Power quality measurement and analyzer toolset
endef

define Package/powqutyd/description
	Powqty allows to connect to a USB oszilloscope, collect cyclic voltage
	samples, analysze typical power quality key performance indicator (e.g.:
	RMS voltage, undervoltages, overvoltages, voltage flicker, ect.) and
	provide the PQ statistics via MQTT.
endef


ifeq ($(CONFIG_POWQUTYD_NO_MQTT),y)

  NO_MQTT := y
  export NO_MQTT
  
  JKL := $(filter-out +libmosquitto,$(DEPENDS))
  DEPENDS := $(JKL)
else
  NO_MQTT := n
  export NO_MQTT
endif

ifeq ($(CONFIG_POWQUTYD_NO_SLACK),y)

  NO_SLACK := y
  export NO_SLACK

  JKL := $(filter-out + libcurl,$(DEPENDS))
  DEPENDS := $(JKL)
else
  NO_SLACK := n
  export NO_SLACK
 endif

CC_TYPE= $(CONFIG_CPU_TYPE)
export CC_TYPE

define Build/Prepare
#	@echo ""
#	@echo ""
#	@echo "############################## Prepare  powqutyd  #######################"
#	@echo ""
	mkdir -p $(PKG_BUILD_DIR)
	cp -r ./files/. $(PKG_BUILD_DIR)/.
#	@echo ""
#	@echo "Done"
#	@echo ""
endef

define Build/Compile
	@echo "ARCH:"
	@echo $(ARCH)
	export ARCH
#	@echo ""
#	@echo ""
#	@echo "############################## Compile  powqutyd  #######################"
#	@echo ""
	$(call Build/Compile/Default,all)
	#$(MAKE) -C files
#	@echo ""
#	@echo "Done"
#	@echo ""
endef

define Package/powqutyd/install
#	@echo ""
#	@echo ""
#	@echo "############################## Install  powqutyd  #######################"
#	@echo $(PKG_BUILD_DIR)
#	@echo ""
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/powqutyd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/etc/init.d/powqutyd.init $(1)/etc/init.d/powqutyd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/etc/powqutyd.config $(1)/etc/config/powquty
#	@echo ""
#	@echo "Done"
#	@echo ""
endef

$(eval $(call BuildPackage,powqutyd))
