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
	DEPENDS:=+libmosquitto +kmod-usb-acm
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
	@echo ""
	@echo ""
	@echo "############################## Prepare  powqutyd  #######################"
	@echo ""
	mkdir -p $(PKG_BUILD_DIR)
	cp -r ./files/. $(PKG_BUILD_DIR)/.
	@echo ""
	@echo "Done"
	@echo ""
endef

define Build/Compile
	@echo ""
	@echo ""
	@echo "############################## Compile  powqutyd  #######################"
	@echo ""
	$(call Build/Compile/Default,all)
	#$(MAKE) -C files
	@echo ""
	@echo "Done"
	@echo ""
endef

define Package/powqutyd/install
	@echo ""
	@echo ""
	@echo "############################## Install  powqutyd  #######################"
	@echo $(PKG_BUILD_DIR)
	@echo ""
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/powqutyd $(1)/usr/sbin/
	@echo ""
	@echo "Done"
	@echo ""
endef

$(eval $(call BuildPackage,powqutyd))
