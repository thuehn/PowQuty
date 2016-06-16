#
# Copyright (C) 2016 LEDE project (Neez El Sayed)
#
# This is free software, licensed under the GNU General Public License v3.

include $(TOPDIR)/rules.mk

PKG_NAME:=powquty
PKG_VERSION:=0.1
PKG_RELEASE:=1
PKG_MAINTAINER:=Neez <neez.el.sayed@gmail.com>
PKG_LICENSE:=GPL-3.0+
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/powquty
    SECTION:=utils
    CATEGORY:=Utilities
    TITLE:=Power Quality utility ...
endef

define Package/powquty/description
        Here goes the description of the powquty library \\\
        the description will be here \\\\
endef


define Build/Compile
	true
endef

define Package/powquty/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/powqutyd $(1)/bin/
endef$(eval $(call BuildPackage,powquty))
