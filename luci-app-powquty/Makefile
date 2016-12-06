include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-powquty
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_MAINTAINER:=me
PKG_LICENSE:=GPL-3.0+
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-powquty
	SECTION:=luci
	CATEGORY:=LuCI
	DEPENDS:=+lua +luci-base +powqutyd
	MAINTAINER:=me
	TITLE:=LuCI Support for powqutyd
	PKGARCH:=all
endef

define Package/luci-app-powquty/description
	LuCI Support for powqutyd
endef

define Build/Compile
endef

define Package/luci-app-powquty/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/40_luci-powquty $(1)/etc/uci-defaults/40_luci-powquty
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/powquty
	$(INSTALL_DATA) ./luasrc/model/cbi/powquty.lua $(1)/usr/lib/lua/luci/model/cbi/powquty/powquty.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/powquty.lua $(1)/usr/lib/lua/luci/controller/powquty.lua
endef

$(eval $(call BuildPackage,luci-app-powquty))
