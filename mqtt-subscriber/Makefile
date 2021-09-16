include $(TOPDIR)/rules.mk

PKG_NAME:=mqtt-subscriber
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/mqtt-subscriber
	CATEGORY:=Base system
	TITLE:=mqtt-subscriber
	DEPENDS:=+luci-app-mqtt-subscriber +libuci +libmosquitto +libsqlite3 +libcurl +libjson-c
endef

define Package/mqtt-subscriber/description
	Simple MQTT subscriber
endef

define Package/mqtt-subscriber/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mqttsub $(1)/usr/bin
	$(INSTALL_CONF) ./files/mqttsub.config $(1)/etc/config/mqttsub
	$(INSTALL_BIN) ./files/mqttsub.init $(1)/etc/init.d/mqttsub
endef

$(eval $(call BuildPackage,mqtt-subscriber))
