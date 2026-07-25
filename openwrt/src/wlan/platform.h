/*
 * platform interfaces for XRadio drivers
 *
 * Copyright (c) 2013, XRadio
 * Author: XRadio
 *
 * OpenWrt / mainline port: the Allwinner BSP MMC rescan / ready helpers
 * (sunxi_mmc_*, sunxi_mci_*, sw_mci_*) do not exist on a mainline kernel.
 * SDIO enumeration and power sequencing are handled by the MMC core via
 * "mmc-pwrseq" and a statically declared SDIO node in the device tree, so
 * MCI_CHECK_READY is provided as a harmless fallback and MCI_RESCAN_CARD is
 * no longer needed.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef XRADIO_PLAT_H_INCLUDED
#define XRADIO_PLAT_H_INCLUDED

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/mmc/host.h>

/*
 * The WLAN function is expected to be ready as soon as the MMC core has
 * enumerated it (mmc-pwrseq handles reset/power timing), so there is no
 * vendor-specific "card ready" poll to perform.
 */
#define MCI_CHECK_READY(h, t)     (0)

int xradio_get_syscfg(void);

/* platform interfaces */
int  xradio_plat_init(void);
void xradio_plat_deinit(void);
void  xradio_sdio_detect(int enable);
int  xradio_request_gpio_irq(struct device *dev, void *sbus_priv);
void xradio_free_gpio_irq(struct device *dev, void *sbus_priv);
int  xradio_wlan_power(int on);

#endif /* XRADIO_PLAT_H_INCLUDED */
