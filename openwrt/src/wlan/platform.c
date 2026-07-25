/*
 * platform interfaces for XRadio drivers
 *
 * Copyright (c) 2013, XRadio
 * Author: XRadio
 *
 * OpenWrt / mainline port:
 *   The original implementation depended on Allwinner BSP symbols
 *   (sunxi_wlan_set_power/sunxi_wlan_get_bus_index/sunxi_wlan_get_oob_irq
 *   and sunxi_mmc_rescan_card).  Those symbols do not exist in a mainline
 *   kernel, so this file has been rewritten to use only standard,
 *   device-tree based mechanisms:
 *
 *     - Power / clock sequencing is expected to be handled by an
 *       "mmc-pwrseq" (e.g. mmc-pwrseq-simple) attached to the SDIO host in
 *       the device tree, so xradio_wlan_power()/xradio_sdio_detect() become
 *       no-ops here.
 *     - The out-of-band host wake IRQ is described in the SDIO function
 *       node of the device tree ("interrupts" + optional "wakeup-source")
 *       and is resolved with of_irq_get() at subscribe time.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/pm_wakeirq.h>
#include "xradio.h"
#include "platform.h"
#include "sbus.h"

MODULE_AUTHOR("XRadioTech");
MODULE_DESCRIPTION("XRadioTech WLAN driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("xradio_wlan");

/* Resolved out-of-band host-wake IRQ and its properties. */
static int gpio_irq_handle;
static unsigned long irq_flags;
static bool wakeup_enable;

/*
 * On mainline the WLAN power rail and SDIO card enumeration are driven by
 * the MMC subsystem (regulators + mmc-pwrseq described in the device tree),
 * so there is no per-driver system-config to read here.
 */
int xradio_get_syscfg(void)
{
	return 0;
}

/*********************Interfaces called by xradio core. *********************/
int  xradio_plat_init(void)
{
	return 0;
}

void xradio_plat_deinit(void)
{
	;
}

/*
 * Power up/down is handled by the SDIO host's mmc-pwrseq / regulators in the
 * device tree.  Kept as a stub so the core power flow is unchanged.
 */
int xradio_wlan_power(int on)
{
	return 0;
}

/*
 * Card insertion/removal is handled by the MMC core (mmc-pwrseq +
 * non-removable SDIO node in the device tree), so nothing to do here.
 */
void xradio_sdio_detect(int enable)
{
	xradio_dbg(XRADIO_DBG_ALWY, "%s SDIO card handled by mmc-pwrseq\n",
				enable ? "Detect" : "Remove");
}

static irqreturn_t xradio_gpio_irq_handler(int irq, void *sbus_priv)
{
	struct sbus_priv *self = (struct sbus_priv *)sbus_priv;
	unsigned long flags;

	SYS_BUG(!self);
	spin_lock_irqsave(&self->lock, flags);
	if (self->irq_handler)
		self->irq_handler(self->irq_priv);
	spin_unlock_irqrestore(&self->lock, flags);
	return IRQ_HANDLED;
}

int xradio_request_gpio_irq(struct device *dev, void *sbus_priv)
{
	struct device_node *np = dev ? dev->of_node : NULL;
	int ret = -EINVAL;

	if (!np) {
		xradio_dbg(XRADIO_DBG_ERROR,
			   "%s: no device-tree node for SDIO function, "
			   "cannot resolve host-wake IRQ.\n", __func__);
		return -ENXIO;
	}

	gpio_irq_handle = of_irq_get(np, 0);
	if (gpio_irq_handle <= 0) {
		xradio_dbg(XRADIO_DBG_ERROR,
			   "%s: of_irq_get FAIL! ret=%d\n",
			   __func__, gpio_irq_handle);
		gpio_irq_handle = 0;
		return -ENXIO;
	}

	/* Trigger type is configured from the DT interrupt specifier. */
	irq_flags = irq_get_trigger_type(gpio_irq_handle);
	wakeup_enable = of_property_read_bool(np, "wakeup-source");

	ret = devm_request_irq(dev, gpio_irq_handle,
					(irq_handler_t)xradio_gpio_irq_handler,
					irq_flags, "xradio_irq", sbus_priv);
	if (ret < 0) {
		gpio_irq_handle = 0;
		xradio_dbg(XRADIO_DBG_ERROR, "%s: request_irq FAIL!ret=%d\n",
				__func__, ret);
		return ret;
	}

	if (wakeup_enable) {
		ret = device_init_wakeup(dev, true);
		if (ret < 0) {
			xradio_dbg(XRADIO_DBG_ERROR, "device init wakeup failed!\n");
			return ret;
		}

		ret = dev_pm_set_wake_irq(dev, gpio_irq_handle);
		if (ret < 0) {
			xradio_dbg(XRADIO_DBG_ERROR, "can't enable wakeup src!\n");
			return ret;
		}
	}

	return ret;
}

void xradio_free_gpio_irq(struct device *dev, void *sbus_priv)
{
	struct sbus_priv *self = (struct sbus_priv *)sbus_priv;
	if (wakeup_enable) {
		device_init_wakeup(dev, false);
		dev_pm_clear_wake_irq(dev);
	}
	if (gpio_irq_handle)
		devm_free_irq(dev, gpio_irq_handle, self);
	gpio_irq_handle = 0;
}
