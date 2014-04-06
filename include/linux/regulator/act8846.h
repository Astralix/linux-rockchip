/*
 * act8846.h  --  Voltage regulation for the active-semi act8846
 *
 * Copyright (C) 2013 Atmel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_REGULATOR_ACT8846_H
#define __LINUX_REGULATOR_ACT8846_H

#include <linux/regulator/machine.h>

enum {
	ACT8846_ID_DCDC1,
	ACT8846_ID_DCDC2,
	ACT8846_ID_DCDC3,
	ACT8846_ID_DCDC4,
	ACT8846_ID_LDO5,
	ACT8846_ID_LDO6,
	ACT8846_ID_LDO7,
	ACT8846_ID_LDO8,
	ACT8846_ID_LDO9,
	ACT8846_ID_LDO10,
	ACT8846_ID_LDO11,
	ACT8846_ID_LDO12,
//	ACT8846_ID_LDO13,
	ACT8846_REG_NUM,
};

/**
 * act8846_regulator_data - regulator data
 * @id: regulator id
 * @name: regulator name
 * @platform_data: regulator init data
 */
struct act8846_regulator_data {
	int id;
	const char *name;
	struct regulator_init_data *platform_data;
};

/**
 * act8846_platform_data - platform data for act8846
 * @num_regulators: number of regulators used
 * @regulators: pointer to regulators used
 */
struct act8846_platform_data {
	int num_regulators;
	struct act8846_regulator_data *regulators;
};
#endif
