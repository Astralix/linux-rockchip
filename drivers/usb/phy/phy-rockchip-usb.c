#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/usb/otg.h>
#include <linux/usb/usb_phy_gen_xceiv.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include "phy-rockchip-usb.h"
#include "phy-generic.h"

struct rk30_usbphy {
	struct usb_phy_gen_xceiv usb_phy_gen;
	struct phy_control *phy_ctrl;
	int regs;
	int id;
};

static void rk_phy_wkup(struct rk30_usbphy *rk_phy, u32 id, bool on)
{
	 unsigned int phy_reg = rk_phy->regs;
	/*
	switch (id) {
	case 0:
		reg = AM335X_PHY0_WK_EN;
		break;
	case 1:
		reg = AM335X_PHY1_WK_EN;
		break;
	default:
		WARN_ON(1);
		return;
	}
	*/

	spin_lock(&usb_ctrl->lock);

	if (on) {	/* WAKEUP */
		regmap_write( regmap, phy_reg + GRF_UOC1_CON2,
				HIWORD_UPDATE( PHY_SFTCON_DIS, PHY_SFTCON_MASK);
	}
	else {		/* SUSPEND */
		regmap_write( regmap, phy_reg + GRF_UOC1_CON2,
				HIWORD_UPDATE( PHY_SFTCON_DIS, PHY_SFTCON_MASK);
		regmap_write( regmap, phy_reg + GRF_UOC1_CON3,
				HIWORD_UPDATE( UTMI_SUSP_ON | UTMI_OPM_NODRV,
						UTMI_SUSP_MASK | UTMI_OPM_MASK);
	}

	spin_unlock(&usb_ctrl->lock);
}

static int rk_host_init(struct usb_phy *phy)
{
	struct rk30_usbphy *rk_phy = dev_get_drvdata(phy->dev);

	phy_ctrl_power(rk_phy->phy_ctrl, rk_phy->id, true);
	return 0;
}

static void rk_host_shutdown(struct usb_phy *phy)
{
	struct rk30_usbphy *rk_phy = dev_get_drvdata(phy->dev);

	phy_ctrl_power(rk_phy->phy_ctrl, rk_phy->id, false);
}

static int rk_phy_probe(struct platform_device *pdev)
{
	struct rk30_usbphy *rk_phy;
	struct device *dev = &pdev->dev;
	int ret;

	rk_phy = devm_kzalloc(dev, sizeof(*rk_phy), GFP_KERNEL);
	if (!rk_phy)
		return -ENOMEM;

	rk_phy->phy_ctrl = am335x_get_phy_control(dev);
	if (!rk_phy->phy_ctrl)
		return -EPROBE_DEFER;
	rk_phy->id = of_alias_get_id(pdev->dev.of_node, "phy");
	if (rk_phy->id < 0) {
		dev_err(&pdev->dev, "Missing PHY id: %d\n", rk_phy->id);
		return rk_phy->id;
	}

	ret = usb_phy_gen_create_phy(dev, &rk_phy->usb_phy_gen, NULL);
	if (ret)
		return ret;

	ret = usb_add_phy_dev(&rk_phy->usb_phy_gen.phy);
	if (ret)
		return ret;
	rk_phy->usb_phy_gen.phy.init = am335x_init;
	rk_phy->usb_phy_gen.phy.shutdown = am335x_shutdown;

	platform_set_drvdata(pdev, rk_phy);
	device_init_wakeup(dev, true);

	/*
	 * If we leave PHY wakeup enabled then AM33XX wakes up
	 * immediately from DS0. To avoid this we mark dev->power.can_wakeup
	 * to false. The same is checked in suspend routine to decide
	 * on whether to enable PHY wakeup or not.
	 * PHY wakeup works fine in standby mode, there by allowing us to
	 * handle remote wakeup, wakeup on disconnect and connect.
	 */

	device_set_wakeup_enable(dev, false);
	phy_ctrl_power(rk_phy->phy_ctrl, rk_phy->id, false);

	return 0;
}

static int rk_phy_remove(struct platform_device *pdev)
{
	struct rk30_usbphy *rk_phy = platform_get_drvdata(pdev);

	usb_remove_phy(&rk_phy->usb_phy_gen.phy);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int rk_phy_suspend(struct device *dev)
{
	struct platform_device	*pdev = to_platform_device(dev);
	struct rk30_usbphy *rk_phy = platform_get_drvdata(pdev);

	/*
	 * Enable phy wakeup only if dev->power.can_wakeup is true.
	 * Make sure to enable wakeup to support remote wakeup	in
	 * standby mode ( same is not supported in OFF(DS0) mode).
	 * Enable it by doing
	 * echo enabled > /sys/bus/platform/devices/<usb-phy-id>/power/wakeup
	 */

	if (device_may_wakeup(dev))
		phy_ctrl_wkup(rk_phy->phy_ctrl, rk_phy->id, true);

	phy_ctrl_power(rk_phy->phy_ctrl, rk_phy->id, false);

	return 0;
}

static int rk_phy_resume(struct device *dev)
{
	struct platform_device	*pdev = to_platform_device(dev);
	struct rk30_usbphy	*rk_phy = platform_get_drvdata(pdev);

	phy_ctrl_power(rk_phy->phy_ctrl, rk_phy->id, true);

	if (device_may_wakeup(dev))
		phy_ctrl_wkup(rk_phy->phy_ctrl, rk_phy->id, false);

	return 0;
}

static const struct dev_pm_ops rk_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(rk_phy_suspend, rk_phy_resume)
};

#define DEV_PM_OPS     (&rk_pm_ops)
#else
#define DEV_PM_OPS     NULL
#endif

static const struct of_device_id rockchip_phy_ids[] = {
	{ .compatible = "rockchip,rockchip-phy-driver" },
	{ }
};
MODULE_DEVICE_TABLE(of, am335x_phy_ids);

static struct platform_driver rockchip_phy_driver = {
	.probe          = rk_phy_probe,
	.remove         = rk_phy_remove,
	.driver         = {
		.name   = "rockchip-phy-driver",
		.owner  = THIS_MODULE,
		.pm = DEV_PM_OPS,
		.of_match_table = rockchip_phy_ids,
	},
};

module_platform_driver(rockchip_phy_driver);
MODULE_LICENSE("GPL v2");
