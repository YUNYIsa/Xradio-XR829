# XRadio XR829 on OpenWrt

This directory contains the OpenWrt packaging for the XRadio **XR829** SDIO
802.11b/g/n Wi-Fi driver kept in this repository, together with the changes
needed to build it against a mainline / OpenWrt kernel instead of the
Allwinner (sunxi) BSP it originally targeted.

## Layout

```
openwrt/
├── src/                                          # integrated driver source
│   ├── Makefile  Kconfig  license
│   ├── include/  umac/  wlan/
├── package/kernel/xradio-xr829/Makefile          # kmod-xradio-xr829
├── package/firmware/xradio-xr829-firmware/        # /lib/firmware blobs
│   ├── Makefile
│   └── files/                                     # drop vendor blobs here
├── dts/xr829-sdio.dtsi                            # example DT wiring
└── README.md                                      # this file
```

## What was changed in the driver

The only hard couplings to the Allwinner BSP were in the platform layer.
They have been removed so the module builds against a mainline kernel:

* `src/wlan/platform.c`
  * Dropped `sunxi_wlan_set_power()`, `sunxi_wlan_get_bus_index()`,
    `sunxi_wlan_get_oob_irq()` and `<linux/sunxi-gpio.h>`.
  * Power/enumeration are now delegated to the MMC subsystem
    (`mmc-pwrseq` + a static SDIO node in the device tree), so
    `xradio_wlan_power()` / `xradio_sdio_detect()` are stubs.
  * The out-of-band host-wake IRQ is resolved from the SDIO function's
    device-tree node with `of_irq_get()`, and `wakeup-source` enables
    wake-on-WLAN.
* `src/wlan/platform.h`
  * Dropped the `sunxi_mmc_*` / `sunxi_mci_*` / `sw_mci_*` externs and
    `MCI_RESCAN_CARD`. `MCI_CHECK_READY()` is now a harmless fallback.

The rest of the driver is unchanged. In particular the module still bundles
its own mac80211 fork ("xrmac"), so it links against **cfg80211** only and
builds a single `xr829.ko`.

## How to build

1. Copy the whole `openwrt/` directory into your OpenWrt tree (or add it as a
   feed). It is self-contained: the driver source lives in `openwrt/src/` and
   the kernel package builds directly from it, so there is **no download step**
   (no `PKG_SOURCE` / `dl/`). Its `Build/Prepare` copies the in-tree
   `Makefile`, `Kconfig`, `license`, `include/`, `umac/` and `wlan/` from
   `openwrt/src/` into the build directory. Keep the package directories
   alongside `openwrt/src/` so the relative path to the source tree stays
   valid.

2. Enable the packages:

   ```
   make menuconfig
   #   Kernel modules -> Wireless Drivers -> kmod-xradio-xr829
   #   Firmware       -> xradio-xr829-firmware
   ```

3. Provide the firmware blobs (not redistributed here) in
   `package/firmware/xradio-xr829-firmware/files/`:
   `fw_xr829.bin`, `sdd_xr829.bin`, and optionally `boot_xr829.bin` /
   `etf_xr829.bin`.

4. Build and iterate:

   ```
   make package/kernel/xradio-xr829/compile V=s
   make package/firmware/xradio-xr829-firmware/compile V=s
   ```

## Device-tree wiring

Copy `dts/xr829-sdio.dtsi` into your board DTS and adjust the MMC
controller, GPIOs, regulators and clock to match the hardware. The key
pieces are:

* an `mmc-pwrseq-simple` with the chip `reset-gpios` (WL_REG_ON) attached to
  the SDIO host via `mmc-pwrseq`;
* the SDIO host marked `non-removable`, `cap-sdio-irq`,
  `keep-power-in-suspend`, with the correct `vmmc-supply`/`vqmmc-supply`;
* a `wifi@1` child node (`compatible = "xradio,xr829"`) carrying the
  out-of-band host-wake `interrupts` (and optional `wakeup-source`).

## Bring-up checklist

* `dmesg` shows the SDIO function enumerated as `0x0A9E:0x2282`.
* `dmesg` shows `fw_xr829.bin` / `sdd_xr829.bin` loaded from `/lib/firmware`.
* `iw dev` lists the new interface; STA/AP association and throughput work.
* suspend/resume (and wake-on-WLAN if `wakeup-source` is set) behave.

## Known limitations / remaining work

* **Bundled mac80211 fork.** The module ships its own `xrmac`. It builds as
  a self-contained `xr829.ko`, but it does not use OpenWrt's system
  mac80211/backports. Fully switching to the system `kmod-mac80211`
  (dropping `umac/`) is a larger, separate effort and is *not* done here.
* **Kernel API drift.** The driver was written for ~Linux 4.9 (Allwinner
  BSP) and vendors a copy of the cfg80211/nl80211/mac80211 headers under
  `include/`. On newer OpenWrt kernels you will likely need to fix up
  cfg80211/nl80211 vendor-command and rfkill/`device_move` usage. Build with
  `V=s` and resolve compiler errors iteratively.
* **Firmware/SDD matching.** Use firmware/SDD blobs that match your specific
  XR829 module revision.
* **Non-sunxi boards.** The SDIO power/reset timing in the example overlay is
  a template; adapt it to your board.
