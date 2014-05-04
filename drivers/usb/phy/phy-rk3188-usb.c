/* linux/drivers/usb/phy/phy-rk3188-usb.c
 *
 * Copyright (c) 2014 Ulrich Prinz (ulrich.prinz at googlemail. com)
 *
 * Rockchips USB PHY transceiver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/usb/otg.h>
// #include <linux/platform_data/samsung-usbphy.h>

#include "phy-rk3188-usb.h"

static int rk3188_usbphy_set_host(struct usb_otg *otg, struct usb_bus *host)
{
	if (!otg)
		return -ENODEV;

	// TODO: Check if I/F is HOST or OTG
	if (!otg->host)
		otg->host = host;

	return 0;
}

static bool rk3188_phyhost_is_on(void __iomem *regs)
{
	u32 reg;

	reg = readl(regs + EXYNOS5_PHY_HOST_CTRL0);

	return !(reg & HOST_CTRL0_SIDDQ);
}

static void rk3188_usbphy_enable(struct rk3188_usbphy *sphy)
{
	void __iomem *regs = sphy->regs;
	u32 phyclk = sphy->ref_clk_freq;
	u32 phyhost;
	u32 phyotg;
	u32 phyhsic;
	u32 ehcictrl;
	u32 ohcictrl;

	/*
	 * phy_usage helps in keeping usage count for phy
	 * so that the first consumer enabling the phy is also
	 * the last consumer to disable it.
	 */

	atomic_inc(&sphy->phy_usage);

	if (exynos5_phyhost_is_on(regs)) {
		dev_info(sphy->dev, "Already power on PHY\n");
		return;
	}

	/* Host configuration */
	phyhost = readl(regs + EXYNOS5_PHY_HOST_CTRL0);

	/* phy reference clock configuration */
	phyhost &= ~HOST_CTRL0_FSEL_MASK;
	phyhost |= HOST_CTRL0_FSEL(phyclk);

	/* host phy reset */
	phyhost &= ~(HOST_CTRL0_PHYSWRST |
			HOST_CTRL0_PHYSWRSTALL |
			HOST_CTRL0_SIDDQ |
			/* Enable normal mode of operation */
			HOST_CTRL0_FORCESUSPEND |
			HOST_CTRL0_FORCESLEEP);

	/* Link reset */
	phyhost |= (HOST_CTRL0_LINKSWRST |
			HOST_CTRL0_UTMISWRST |
			/* COMMON Block configuration during suspend */
			HOST_CTRL0_COMMONON_N);
	writel(phyhost, regs + EXYNOS5_PHY_HOST_CTRL0);
	udelay(10);
	phyhost &= ~(HOST_CTRL0_LINKSWRST |
			HOST_CTRL0_UTMISWRST);
	writel(phyhost, regs + EXYNOS5_PHY_HOST_CTRL0);

	/* OTG configuration */
	phyotg = readl(regs + EXYNOS5_PHY_OTG_SYS);

	/* phy reference clock configuration */
	phyotg &= ~OTG_SYS_FSEL_MASK;
	phyotg |= OTG_SYS_FSEL(phyclk);

	/* Enable normal mode of operation */
	phyotg &= ~(OTG_SYS_FORCESUSPEND |
			OTG_SYS_SIDDQ_UOTG |
			OTG_SYS_FORCESLEEP |
			OTG_SYS_REFCLKSEL_MASK |
			/* COMMON Block configuration during suspend */
			OTG_SYS_COMMON_ON);

	/* OTG phy & link reset */
	phyotg |= (OTG_SYS_PHY0_SWRST |
			OTG_SYS_LINKSWRST_UOTG |
			OTG_SYS_PHYLINK_SWRESET |
			OTG_SYS_OTGDISABLE |
			/* Set phy refclk */
			OTG_SYS_REFCLKSEL_CLKCORE);

	writel(phyotg, regs + EXYNOS5_PHY_OTG_SYS);
	udelay(10);
	phyotg &= ~(OTG_SYS_PHY0_SWRST |
			OTG_SYS_LINKSWRST_UOTG |
			OTG_SYS_PHYLINK_SWRESET);
	writel(phyotg, regs + EXYNOS5_PHY_OTG_SYS);

	/* HSIC phy configuration */
	phyhsic = (HSIC_CTRL_REFCLKDIV_12 |
			HSIC_CTRL_REFCLKSEL |
			HSIC_CTRL_PHYSWRST);
	writel(phyhsic, regs + EXYNOS5_PHY_HSIC_CTRL1);
	writel(phyhsic, regs + EXYNOS5_PHY_HSIC_CTRL2);
	udelay(10);
	phyhsic &= ~HSIC_CTRL_PHYSWRST;
	writel(phyhsic, regs + EXYNOS5_PHY_HSIC_CTRL1);
	writel(phyhsic, regs + EXYNOS5_PHY_HSIC_CTRL2);

	udelay(80);

	/* enable EHCI DMA burst */
	ehcictrl = readl(regs + EXYNOS5_PHY_HOST_EHCICTRL);
	ehcictrl |= (HOST_EHCICTRL_ENAINCRXALIGN |
				HOST_EHCICTRL_ENAINCR4 |
				HOST_EHCICTRL_ENAINCR8 |
				HOST_EHCICTRL_ENAINCR16);
	writel(ehcictrl, regs + EXYNOS5_PHY_HOST_EHCICTRL);

	/* set ohci_suspend_on_n */
	ohcictrl = readl(regs + EXYNOS5_PHY_HOST_OHCICTRL);
	ohcictrl |= HOST_OHCICTRL_SUSPLGCY;
	writel(ohcictrl, regs + EXYNOS5_PHY_HOST_OHCICTRL);
}

static void samsung_usb2phy_enable(struct rk3188_usbphy *sphy)
{
	void __iomem *regs = sphy->regs;
	u32 phypwr;
	u32 phyclk;
	u32 rstcon;

	/* set clock frequency for PLL */
	phyclk = sphy->ref_clk_freq;
	phypwr = readl(regs + SAMSUNG_PHYPWR);
	rstcon = readl(regs + SAMSUNG_RSTCON);

	switch (sphy->drv_data->cpu_type) {
	case TYPE_S3C64XX:
		phyclk &= ~PHYCLK_COMMON_ON_N;
		phypwr &= ~PHYPWR_NORMAL_MASK;
		rstcon |= RSTCON_SWRST;
		break;
	case TYPE_EXYNOS4X12:
		phypwr &= ~(PHYPWR_NORMAL_MASK_HSIC0 |
				PHYPWR_NORMAL_MASK_HSIC1 |
				PHYPWR_NORMAL_MASK_PHY1);
		rstcon |= RSTCON_HOSTPHY_SWRST;
	case TYPE_EXYNOS4210:
		phypwr &= ~PHYPWR_NORMAL_MASK_PHY0;
		rstcon |= RSTCON_SWRST;
	default:
		break;
	}

	writel(phyclk, regs + SAMSUNG_PHYCLK);
	/* Configure PHY0 for normal operation*/
	writel(phypwr, regs + SAMSUNG_PHYPWR);
	/* reset all ports of PHY and Link */
	writel(rstcon, regs + SAMSUNG_RSTCON);
	udelay(10);
	if (sphy->drv_data->cpu_type == TYPE_EXYNOS4X12)
		rstcon &= ~RSTCON_HOSTPHY_SWRST;
	rstcon &= ~RSTCON_SWRST;
	writel(rstcon, regs + SAMSUNG_RSTCON);
}

static void samsung_exynos5_usb2phy_disable(struct rk3188_usbphy *sphy)
{
	void __iomem *regs = sphy->regs;
	u32 phyhost;
	u32 phyotg;
	u32 phyhsic;

	if (atomic_dec_return(&sphy->phy_usage) > 0) {
		dev_info(sphy->dev, "still being used\n");
		return;
	}

	phyhsic = (HSIC_CTRL_REFCLKDIV_12 |
			HSIC_CTRL_REFCLKSEL |
			HSIC_CTRL_SIDDQ |
			HSIC_CTRL_FORCESLEEP |
			HSIC_CTRL_FORCESUSPEND);
	writel(phyhsic, regs + EXYNOS5_PHY_HSIC_CTRL1);
	writel(phyhsic, regs + EXYNOS5_PHY_HSIC_CTRL2);

	phyhost = readl(regs + EXYNOS5_PHY_HOST_CTRL0);
	phyhost |= (HOST_CTRL0_SIDDQ |
			HOST_CTRL0_FORCESUSPEND |
			HOST_CTRL0_FORCESLEEP |
			HOST_CTRL0_PHYSWRST |
			HOST_CTRL0_PHYSWRSTALL);
	writel(phyhost, regs + EXYNOS5_PHY_HOST_CTRL0);

	phyotg = readl(regs + EXYNOS5_PHY_OTG_SYS);
	phyotg |= (OTG_SYS_FORCESUSPEND |
			OTG_SYS_SIDDQ_UOTG |
			OTG_SYS_FORCESLEEP);
	writel(phyotg, regs + EXYNOS5_PHY_OTG_SYS);
}

static void rk3188_usbphy_disable(struct rk3188_usbphy *rkphy)
{
	void __iomem *regs = rkphy->regs;
	u32 phypwr;

	phypwr = readl(regs + SAMSUNG_PHYPWR);

	switch (rkphy->drv_data->if_type) {
	case TYPE_HOST:
		phypwr |= PHYPWR_NORMAL_MASK;
		break;
	case TYPE_OTG:
		phypwr |= (PHYPWR_NORMAL_MASK_HSIC0 |
				PHYPWR_NORMAL_MASK_HSIC1 |
				PHYPWR_NORMAL_MASK_PHY1);
	case TYPE_HSIC:
		phypwr |= PHYPWR_NORMAL_MASK_PHY0;
	default:
		break;
	}

	/* Disable analog and otg block power */
	writel(phypwr, regs + SAMSUNG_PHYPWR);
}

/*
 * The function passed to the USB driver for PHY initialization
 */
static int rk3188_usbphy_init(struct usb_phy *phy)
{
	struct rk3188_usbphy *rkphy;
	struct usb_bus *host = NULL;
	unsigned long flags;
	int ret = 0;

	rkphy = phy_to_rkphy(phy);

	host = phy->otg->host;

	/* Enable the phy clock */
	ret = clk_prepare_enable(rkphy->clk);
	if (ret) {
		dev_err(rkphy->dev, "%s: clk_prepare_enable failed\n", __func__);
		return ret;
	}

	spin_lock_irqsave(&rkphy->lock, flags);

	if (host) {
		/* setting default phy-type for USB 2.0 */
		if (!strstr(dev_name(host->controller), "ehci") ||
				!strstr(dev_name(host->controller), "ohci"))
			rk3188_usbphy_set_type(&rkphy->phy, USB_PHY_TYPE_HOST);
	} else {
		rk3188_usbphy_set_type(&rkphy->phy, USB_PHY_TYPE_DEVICE);
	}

	/* Disable phy isolation */
	if (rkphy->plat && rkphy->plat->pmu_isolation)
		rkphy->plat->pmu_isolation(false);
	else if (rkphy->drv_data->set_isolation)
		rkphy->drv_data->set_isolation(rkphy, false);

	/* Selecting Host/OTG mode; After reset USB2.0PHY_CFG: HOST */
	rk3188_usbphy_cfg_sel(rkphy);

	/* Initialize usb phy registers */
	sphy->drv_data->phy_enable(rkphy);

	spin_unlock_irqrestore(&rkphy->lock, flags);

	/* Disable the phy clock */
	clk_disable_unprepare(rkphy->clk);

	return ret;
}

/*
 * The function passed to the usb driver for phy shutdown
 */
static void rk3188_usbphy_shutdown(struct usb_phy *phy)
{
	struct rk3188_usbphy *rkphy;
	struct usb_bus *host = NULL;
	unsigned long flags;

	rkphy = phy_to_rkphy(phy);

	host = phy->otg->host;

	if (clk_prepare_enable(rkphy->clk)) {
		dev_err(rkphy->dev, "%s: clk_prepare_enable failed\n", __func__);
		return;
	}

	spin_lock_irqsave(&rkphy->lock, flags);

	if (host) {
		/* setting default phy-type for USB 2.0 */
		if (!strstr(dev_name(host->controller), "ehci") ||
				!strstr(dev_name(host->controller), "ohci"))
			rk3188_usbphy_set_type(&rkphy->phy, USB_PHY_TYPE_HOST);
	} else {
		rk3188_usbphy_set_type(&rkphy->phy, USB_PHY_TYPE_DEVICE);
	}

	/* De-initialize usb phy registers */
	rkphy->drv_data->phy_disable(rkphy);

	/* Enable phy isolation */
	if (rkphy->plat && rkphy->plat->pmu_isolation)
		rkphy->plat->pmu_isolation(true);
	else if (rkphy->drv_data->set_isolation)
		rkphy->drv_data->set_isolation(rkphy, true);

	spin_unlock_irqrestore(&rkphy->lock, flags);

	clk_disable_unprepare(rkphy->clk);
}

static int rk3188_usbphy_probe(struct platform_device *pdev)
{
	struct rk3188_usbphy *rkphy;
	struct usb_otg *otg;
	struct rk3188_usbphy_data *pdata = dev_get_platdata(&pdev->dev);
	const struct rk3188_usbphy_drvdata *drv_data;
	struct device *dev = &pdev->dev;
	struct resource *phy_mem;
	void __iomem	*phy_base;
	struct clk *clk;
	int ret;

	phy_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	phy_base = devm_ioremap_resource(dev, phy_mem);
	if (IS_ERR(phy_base))
		return PTR_ERR(phy_base);

	rkphy = devm_kzalloc(dev, sizeof(*rkphy), GFP_KERNEL);
	if (!rkphy)
		return -ENOMEM;

	otg = devm_kzalloc(dev, sizeof(*otg), GFP_KERNEL);
	if (!otg)
		return -ENOMEM;

	drv_data = rk3188_usbphy_get_driver_data(pdev);

	if (drv_data->cpu_type == TYPE_EXYNOS5250)
		clk = devm_clk_get(dev, "usbhost");
	else
		clk = devm_clk_get(dev, "otg");

	if (IS_ERR(clk)) {
		dev_err(dev, "Failed to get usbhost/otg clock\n");
		return PTR_ERR(clk);
	}

	rkphy->dev = dev;

	if (dev->of_node) {
		ret = rk3188_usbphy_parse_dt(rkphy);
		if (ret < 0)
			return ret;
	} else {
		if (!pdata) {
			dev_err(dev, "no platform data specified\n");
			return -EINVAL;
		}
	}

	rkphy->plat		= pdata;
	rkphy->regs		= phy_base;
	rkphy->clk		= clk;
	rkphy->drv_data		= drv_data;
	rkphy->phy.dev		= rkphy->dev;
	rkphy->phy.label	= "rk3188-usbphy";
	rkphy->phy.type		= USB_PHY_TYPE_USB2;
	rkphy->phy.init		= rk3188_usbphy_init;
	rkphy->phy.shutdown	= rk3188_usbphy_shutdown;

	rkphy->ref_clk_freq = rk3188_usbphy_get_refclk_freq(rkphy);
	if (rkphy->ref_clk_freq < 0)
		return -EINVAL;

	rkphy->phy.otg		= otg;
	rkphy->phy.otg->phy	= &rkphy->phy;
	rkphy->phy.otg->set_host = rk3188_usbphy_set_host;

	spin_lock_init(&rkphy->lock);

	platform_set_drvdata(pdev, rkphy);

	return usb_add_phy_dev(&rkphy->phy);
}

static int rk3188_usbphy_remove(struct platform_device *pdev)
{
	struct rk3188_usbphy *rkphy = platform_get_drvdata(pdev);

	usb_remove_phy(&rkphy->phy);

	if (rkphy->pmuregs)
		iounmap(rkphy->pmuregs);
	if (rkphy->sysreg)
		iounmap(rkphy->sysreg);

	return 0;
}

static const struct rk3188_usbphy_drvdata usbphy_host = {
	.if_type		= TYPE_HOST,
	.devphy_en_mask		= RK3188_USBPHY_ENABLE,
	.rate_to_clksel		= rk3188_usbphy_rate_to_clksel_64xx,
	.phy_enable		= rk3188_usbphy_enable,
	.phy_disable		= rk3188_usbphy_disable,
};

static const struct rk3188_usbphy_drvdata usb2phy_exynos4 = {
	.if_type		= TYPE_OTG,
	.devphy_en_mask		= RK3188_USBPHY_ENABLE,
	.hostphy_en_mask	= RK3188_USBPHY_ENABLE,
	.rate_to_clksel		= rk3188_usbphy_rate_to_clksel_64xx,
	.set_isolation		= rk3188_usbphy_set_isolation_4210,
	.phy_enable		= rk3188_usbphy_enable,
	.phy_disable		= rk3188_usbphy_disable,
};

static const struct rk3188_usbphy_drvdata usb2phy_exynos4x12 = {
	.if_type		= TYPE_HSIC,
	.devphy_en_mask		= RK3188_USBPHY_ENABLE,
	.hostphy_en_mask	= RK3188_USBPHY_ENABLE,
	.rate_to_clksel		= rk3188_usbphy_rate_to_clksel_4x12,
	.set_isolation		= rk3188_usbphy_set_isolation_4210,
	.phy_enable		= rk3188_usbphy_enable,
	.phy_disable		= rk3188_usbphy_disable,
};

#ifdef CONFIG_OF
static const struct of_device_id rk3188_usbphy_dt_match[] = {
	{
		.compatible = "rockchip,rk3188-usbphy-host",
		.data = &usb2phy_s3c64xx,
	}, {
		.compatible = "rockchip,rk3188-usbphy-otg",
		.data = &usb2phy_exynos4,
	}, {
		.compatible = "rockchip,rk3188-usb2phy-hsic",
		.data = &usb2phy_exynos4x12,
	},
	{},
};
MODULE_DEVICE_TABLE(of, rk3188_usbphy_dt_match);
#endif

static struct platform_device_id rk3188_usbphy_driver_ids[] = {
	{
		.name		= "rk3188-usbphy-host",
		.driver_data	= (unsigned long)&rk3188_usbphy_host,
	}, {
		.name		= "rk3188-usbphy-otg",
		.driver_data	= (unsigned long)&rk3188_usbphy_otg,
	}, {
		.name		= "rk3188-hsic",
		.driver_data	= (unsigned long)&rk3188_usbphy_hsic,
	},
	{},
};

MODULE_DEVICE_TABLE(platform, rk3188_usbphy_driver_ids);

static struct platform_driver samsung_usb2phy_driver = {
	.probe		= rk3188_usbphy_probe,
	.remove		= rk3188_usbphy_remove,
	.id_table	= rk3188_usbphy_driver_ids,
	.driver		= {
		.name	= "rk3188-usbphy",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(rk3188_usbphy_dt_match),
	},
};

module_platform_driver(samsung_usb2phy_driver);

MODULE_DESCRIPTION("Rockchips USB phy controller");
MODULE_AUTHOR("Ulrich Prinz <ulrich.prinz@googlemail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:rockchip-usbphy");
