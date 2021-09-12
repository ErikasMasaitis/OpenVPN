include $(TOPDIR)/rules.mk

PKG_NAME:=sockets
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/sockets
	DEPENDS:=+libubus +libubox +libblobmsg-json
	CATEGORY:=Base system
	TITLE:=sockets
endef

define Package/sockets/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sockets $(1)/usr/bin
endef

$(eval $(call BuildPackage,sockets))
