#
# OpenWrt package that installs the XRadio XR829 firmware blobs into
# /lib/firmware.  The binaries are NOT redistributed in this repository
# (they are covered by Allwinner's firmware license); drop the following
# files into the ./files/ directory before building:
#
#   files/fw_xr829.bin     (main firmware)
#   files/sdd_xr829.bin    (SDD / RF configuration)
#   files/boot_xr829.bin   (bootloader, if your firmware set uses one)
#   files/etf_xr829.bin    (ETF test firmware, optional)
#
# Copyright (C) 2024 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xradio-xr829-firmware
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_MAINTAINER:=YUNYIsa

include $(INCLUDE_DIR)/package.mk

define Package/xradio-xr829-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=XRadio XR829 Wi-Fi firmware
  DEPENDS:=+kmod-xradio-xr829
endef

define Package/xradio-xr829-firmware/description
  Firmware and SDD/RF configuration blobs loaded by the XR829 driver via
  request_firmware() from /lib/firmware. Supply the vendor blobs in this
  package's files/ directory before building.
endef

# Nothing to compile; the blobs are shipped verbatim.
define Build/Compile
endef

define Package/xradio-xr829-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	for f in fw_xr829.bin sdd_xr829.bin boot_xr829.bin etf_xr829.bin; do \
		if [ -f ./files/$$$$f ]; then \
			$(INSTALL_DATA) ./files/$$$$f $(1)/lib/firmware/$$$$f; \
		fi; \
	done
endef

$(eval $(call BuildPackage,xradio-xr829-firmware))
