/*
 * act8846-regulator.c - Voltage regulation for the active-semi ACT8846
 *
 * Copyright (C) 2013 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/act8846.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regmap.h>

/*
 * ACT8846 Global Register Map.
 */
#define	ACT8846_SYS_MODE	0x00
#define	ACT8846_SYS_CTRL	0x01
#define	ACT8846_DCDC1_VSET1	0x10
#define	ACT8846_DCDC1_CTRL	0x12
#define	ACT8846_DCDC2_VSET1	0x20
#define	ACT8846_DCDC2_VSET2	0x21
#define	ACT8846_DCDC2_CTRL	0x22
#define	ACT8846_DCDC3_VSET1	0x30
#define	ACT8846_DCDC3_VSET2	0x31
#define	ACT8846_DCDC3_CTRL	0x32
#define	ACT8846_DCDC4_VSET1	0x40
#define	ACT8846_DCDC4_VSET2	0x41
#define	ACT8846_DCDC4_CTRL	0x42
#define	ACT8846_LDO5_VSET	0x50
#define	ACT8846_LDO5_CTRL	0x51
#define	ACT8846_LDO6_VSET	0x58
#define	ACT8846_LDO6_CTRL	0x59
#define	ACT8846_LDO7_VSET	0x60
#define	ACT8846_LDO7_CTRL	0x61
#define	ACT8846_LDO8_VSET	0x68
#define	ACT8846_LDO8_CTRL	0x69
#define	ACT8846_LDO9_VSET	0x70
#define	ACT8846_LDO9_CTRL	0x71
#define	ACT8846_LDO10_VSET	0x80
#define	ACT8846_LDO10_CTRL	0x81
#define	ACT8846_LDO11_VSET	0x90
#define	ACT8846_LDO11_CTRL	0x91
#define	ACT8846_LDO12_VSET	0xA0
#define	ACT8846_LDO12_CTRL	0xA1
#define ACT8846_LDO13_CTRL	0xB1

/*
 * Field Definitions.
 */
#define	ACT8846_ENA		BIT(7)	/* ON - [7] */
#define	ACT8846_VSEL_MASK	0x3F	/* VSET - [5:0] */

/*
 * ACT8846 voltage number
 */
#define	ACT8846_VOLTAGE_NUM	64

struct act8846 {
	struct regulator_dev *rdev[ACT8846_REG_NUM];
	struct regmap *regmap;
};

static const struct regmap_config act8846_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};

static const struct regulator_linear_range act8846_voltage_ranges[] = {
	REGULATOR_LINEAR_RANGE(600000, 0, 23, 25000),
	REGULATOR_LINEAR_RANGE(1200000, 24, 47, 50000),
	REGULATOR_LINEAR_RANGE(2400000, 48, 63, 100000),
};

static struct regulator_ops act8846_ops = {
	.list_voltage		= regulator_list_voltage_linear_range,
	.map_voltage		= regulator_map_voltage_linear_range,
	.get_voltage_sel	= regulator_get_voltage_sel_regmap,
	.set_voltage_sel	= regulator_set_voltage_sel_regmap,
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.is_enabled		= regulator_is_enabled_regmap,
};

static struct regulator_ops act8846_fixed_ops = {
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.is_enabled		= regulator_is_enabled_regmap,
};

static const struct regulator_desc act8846_reg[] = {
	{
		.name = "DCDC_REG1",
		.id = ACT8846_ID_DCDC1,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_DCDC1_VSET1,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_DCDC1_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "DCDC_REG2",
		.id = ACT8846_ID_DCDC2,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_DCDC2_VSET1,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_DCDC2_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "DCDC_REG3",
		.id = ACT8846_ID_DCDC3,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_DCDC3_VSET1,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_DCDC3_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "DCDC_REG4",
		.id = ACT8846_ID_DCDC4,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_DCDC4_VSET1,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_DCDC4_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "LDO_REG5",
		.id = ACT8846_ID_LDO5,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_LDO5_VSET,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_LDO5_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "LDO_REG6",
		.id = ACT8846_ID_LDO6,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_LDO6_VSET,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_LDO6_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "LDO_REG7",
		.id = ACT8846_ID_LDO7,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_LDO7_VSET,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_LDO7_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "LDO_REG8",
		.id = ACT8846_ID_LDO8,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_LDO8_VSET,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_LDO8_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "LDO_REG9",
		.id = ACT8846_ID_LDO9,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_LDO9_VSET,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_LDO9_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "LDO_REG10",
		.id = ACT8846_ID_LDO10,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_LDO10_VSET,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_LDO10_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "LDO_REG11",
		.id = ACT8846_ID_LDO11,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_LDO11_VSET,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_LDO11_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
	}, {
		.name = "LDO_REG12",
		.id = ACT8846_ID_LDO12,
		.ops = &act8846_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ACT8846_VOLTAGE_NUM,
		.linear_ranges = act8846_voltage_ranges,
		.n_linear_ranges = ARRAY_SIZE(act8846_voltage_ranges),
		.vsel_reg = ACT8846_LDO12_VSET,
		.vsel_mask = ACT8846_VSEL_MASK,
		.enable_reg = ACT8846_LDO12_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,
/*	}, {
		.name = "LDO_REG13",
		.id = ACT8846_ID_LDO13,
		.ops = &act8846_fixed_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = 1,
		.fixed_uV = 1800000,
		.enable_reg = ACT8846_LDO13_CTRL,
		.enable_mask = ACT8846_ENA,
		.owner = THIS_MODULE,*/
	},
};

#ifdef CONFIG_OF
static const struct of_device_id act8846_dt_ids[] = {
	{ .compatible = "active-semi,act8846" },
	{ }
};
MODULE_DEVICE_TABLE(of, act8846_dt_ids);

static struct of_regulator_match act8846_matches[] = {
	[ACT8846_ID_DCDC1]	= { .name = "DCDC_REG1"},
	[ACT8846_ID_DCDC2]	= { .name = "DCDC_REG2"},
	[ACT8846_ID_DCDC3]	= { .name = "DCDC_REG3"},
	[ACT8846_ID_DCDC4]	= { .name = "DCDC_REG4"},
	[ACT8846_ID_LDO5]	= { .name = "LDO_REG5"},
	[ACT8846_ID_LDO6]	= { .name = "LDO_REG6"},
	[ACT8846_ID_LDO7]	= { .name = "LDO_REG7"},
	[ACT8846_ID_LDO8]	= { .name = "LDO_REG8"},
	[ACT8846_ID_LDO9]	= { .name = "LDO_REG9"},
	[ACT8846_ID_LDO10]	= { .name = "LDO_REG10"},
	[ACT8846_ID_LDO11]	= { .name = "LDO_REG11"},
	[ACT8846_ID_LDO12]	= { .name = "LDO_REG12"},
//	[ACT8846_ID_LDO13]	= { .name = "LDO_REG13"},
};

static int act8846_pdata_from_dt(struct device *dev,
				 struct device_node **of_node,
				 struct act8846_platform_data *pdata)
{
	int matched, i;
	struct device_node *np;
	struct act8846_regulator_data *regulator;

	np = of_find_node_by_name(dev->of_node, "regulators");
	if (!np) {
		dev_err(dev, "missing 'regulators' subnode in DT\n");
		return -EINVAL;
	}

	matched = of_regulator_match(dev, np,
				act8846_matches, ARRAY_SIZE(act8846_matches));
	if (matched <= 0)
		return matched;

	pdata->regulators = devm_kzalloc(dev,
				sizeof(struct act8846_regulator_data) *
				ARRAY_SIZE(act8846_matches), GFP_KERNEL);
	if (!pdata->regulators) {
		dev_err(dev, "%s: failed to allocate act8846 registor\n",
						__func__);
		return -ENOMEM;
	}

	pdata->num_regulators = matched;
	regulator = pdata->regulators;

	for (i = 0; i < ARRAY_SIZE(act8846_matches); i++) {
		regulator->id = i;
		regulator->name = act8846_matches[i].name;
		regulator->platform_data = act8846_matches[i].init_data;
		of_node[i] = act8846_matches[i].of_node;
		regulator++;
	}

	return 0;
}
#else
static inline int act8846_pdata_from_dt(struct device *dev,
					struct device_node **of_node,
					struct act8846_platform_data *pdata)
{
	return 0;
}
#endif

static int act8846_pmic_probe(struct i2c_client *client,
			   const struct i2c_device_id *i2c_id)
{
	struct regulator_dev **rdev;
	struct device *dev = &client->dev;
	struct act8846_platform_data *pdata = dev_get_platdata(dev);
	struct regulator_config config = { };
	struct act8846 *act8846;
	struct device_node *of_node[ACT8846_REG_NUM];
	int i, id;
	int ret = -EINVAL;
	int error;

	if (dev->of_node && !pdata) {
		const struct of_device_id *id;
		struct act8846_platform_data pdata_of;

		id = of_match_device(of_match_ptr(act8846_dt_ids), dev);
		if (!id)
			return -ENODEV;

		ret = act8846_pdata_from_dt(dev, of_node, &pdata_of);
		if (ret < 0)
			return ret;

		pdata = &pdata_of;
	}

	if (pdata->num_regulators > ACT8846_REG_NUM) {
		dev_err(dev, "Too many regulators found!\n");
		return -EINVAL;
	}

	act8846 = devm_kzalloc(dev, sizeof(struct act8846), GFP_KERNEL);
	if (!act8846)
		return -ENOMEM;

	rdev = act8846->rdev;

	act8846->regmap = devm_regmap_init_i2c(client, &act8846_regmap_config);
	if (IS_ERR(act8846->regmap)) {
		error = PTR_ERR(act8846->regmap);
		dev_err(&client->dev, "Failed to allocate register map: %d\n",
			error);
		return error;
	}

	/* Finally register devices */
	for (i = 0; i < ACT8846_REG_NUM; i++) {

		id = pdata->regulators[i].id;

		config.dev = dev;
		config.init_data = pdata->regulators[i].platform_data;
		config.of_node = of_node[i];
		config.driver_data = act8846;
		config.regmap = act8846->regmap;

		rdev[i] = devm_regulator_register(&client->dev,
						&act8846_reg[i], &config);
		if (IS_ERR(rdev[i])) {
			dev_err(dev, "failed to register %s\n",
				act8846_reg[id].name);
			return PTR_ERR(rdev[i]);
		}
	}

	i2c_set_clientdata(client, act8846);

	return 0;
}

static const struct i2c_device_id act8846_ids[] = {
	{ "act8846", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, act8846_ids);

static struct i2c_driver act8846_pmic_driver = {
	.driver	= {
		.name	= "act8846",
		.owner	= THIS_MODULE,
	},
	.probe		= act8846_pmic_probe,
	.id_table	= act8846_ids,
};

module_i2c_driver(act8846_pmic_driver);

MODULE_DESCRIPTION("active-semi act8846 voltage regulator driver");
MODULE_AUTHOR("Wenyou Yang <wenyou.yang@atmel.com>");
MODULE_LICENSE("GPL v2");
