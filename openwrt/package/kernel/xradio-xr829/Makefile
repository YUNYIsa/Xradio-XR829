#
# OpenWrt package for the XRadio XR829 SDIO Wi-Fi driver.
#
# This builds the out-of-tree driver kept in this repository as a kernel
# module (xr829.ko).  The module bundles its own mac80211 fork ("xrmac"),
# so it only needs cfg80211 from the kernel and does NOT use the system
# kmod-mac80211.  See ../../../README.md for the porting status and the
# device-tree wiring that this package expects.
#
# Copyright (C) 2024 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xradio-xr829
PKG_VERSION:=1
PKG_RELEASE:=1

# Build straight from the driver sources that are integrated in this
# repository under openwrt/src.  There is NO download step (no PKG_SOURCE /
# dl/): Build/Prepare copies the in-tree source into $(PKG_BUILD_DIR) below.
# The path is resolved relative to this Makefile so the whole openwrt/ tree
# can be copied into any OpenWrt feed as a self-contained package.
XRADIO_SRC_DIR:=$(realpath $(dir $(lastword $(MAKEFILE_LIST)))../../../src)

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=license
PKG_MAINTAINER:=YUNYIsa

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/xradio-xr829
  SUBMENU:=Wireless Drivers
  TITLE:=XRadio XR829 SDIO 802.11n WLAN driver
  DEPENDS:=+kmod-mmc +kmod-cfg80211 +@DRIVER_11N_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/xr829.ko
  AUTOLOAD:=$(call AutoProbe,xr829)
endef

define KernelPackage/xradio-xr829/description
  Kernel module for the Allwinner XRadio XR829 SDIO 802.11b/g/n Wi-Fi chip.

  Power sequencing and SDIO enumeration must be described in the device
  tree using an mmc-pwrseq and a statically declared SDIO node with the
  host-wake "interrupts" property (see the example overlay shipped in
  openwrt/dts/).  Firmware blobs are provided by the separate
  xradio-xr829-firmware package.
endef

# Configuration knobs consumed by the in-tree kbuild Makefile.  We keep the
# SDIO transport and GPIO OOB IRQ enabled and drop the noisy development
# debug/verbose options.
XRADIO_MAKE_OPTS:= \
	CONFIG_XR829_WLAN=m \
	CONFIG_XRADIO_SDIO=y \
	CONFIG_XRADIO_USE_GPIO_IRQ=y \
	CONFIG_XRADIO_DEBUG=n \
	CONFIG_XRADIO_VERBOSE_DEBUG=n \
	CONFIG_XRADIO_DEBUGFS=n \
	CONFIG_XRMAC_DEBUGFS=n

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(XRADIO_SRC_DIR)/Makefile \
	      $(XRADIO_SRC_DIR)/Kconfig \
	      $(XRADIO_SRC_DIR)/license \
	      $(XRADIO_SRC_DIR)/include \
	      $(XRADIO_SRC_DIR)/umac \
	      $(XRADIO_SRC_DIR)/wlan \
	      $(PKG_BUILD_DIR)/
endef

define Build/Compile
	+$(KERNEL_MAKE) M=$(PKG_BUILD_DIR) \
		$(XRADIO_MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,xradio-xr829))
