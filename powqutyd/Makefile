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

include $(INCLUDE_DIR)/package.mk

define Package/powqutyd
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+libm +libc +libpthread +libmosquitto
	MAINTAINER:=El Sayed
	TITLE:=Power quality measurement and analyzer toolset
endef

define Package/powqutyd/description
	Powqty allows to connect to a USB oszilloscope, collect cyclic voltage
	samples, analysze typical power quality key performance indicator (e.g.:
	RMS voltage, undervoltages, overvoltages, voltage flicker, ect.) and
	provide the PQ statistics via MQTT.
endef

define Build/Prepare
	echo "############################## Prepare  powqutyd  #######################"
	@echo "############################## Prepare  powqutyd  with @ #######################"
	mkdir -p $(PKG_BUILD_DIR)
	cp -r ./files/. $(PKG_BUILD_DIR)/.
endef

define Build/Compile
	echo "############################## Compile  powqutyd  #######################"
	$(call Build/Compile/Default,all)
	#$(MAKE) -C files
endef

define Package/powqutyd/install
	echo "############################## Install  powqutyd  #######################"
	pwd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/powqutyd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,powqutyd))
