/*
 * Copyright (c) 2013 MundoReader S.L.
 * Author: Heiko Stuebner <heiko@sntech.de>
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

#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include "clk.h"

//FIXME: from dt-bindings doc
#define NR_CLKS 200

/* Helper macros to define clock arrays. */
#define MUX_CLOCKS(name)	\
		static struct rockchip_mux_clock name[]
#define DIV_CLOCKS(name)	\
		static struct rockchip_div_clock name[]
#define GATE_CLOCKS(name)	\
		static struct rockchip_gate_clock name[]


/* list of PLLs to be registered */
enum rk3188_plls {
	apll, cpll, dpll, gpll,
};

struct rockchip_pll_rate_table rk3188_apll_rates[] = {
	RK3066_PLL_RATE(2208000000, 1, 92, 1),
	RK3066_PLL_RATE(2184000000, 1, 91, 1),
	RK3066_PLL_RATE(2160000000, 1, 90, 1),
	RK3066_PLL_RATE(2136000000, 1, 89, 1),
	RK3066_PLL_RATE(2112000000, 1, 88, 1),
	RK3066_PLL_RATE(2088000000, 1, 87, 1),
	RK3066_PLL_RATE(2064000000, 1, 86, 1),
	RK3066_PLL_RATE(2040000000, 1, 85, 1),
	RK3066_PLL_RATE(2016000000, 1, 84, 1),
	RK3066_PLL_RATE(1992000000, 1, 83, 1),
	RK3066_PLL_RATE(1968000000, 1, 82, 1),
	RK3066_PLL_RATE(1944000000, 1, 81, 1),
	RK3066_PLL_RATE(1920000000, 1, 80, 1),
	RK3066_PLL_RATE(1896000000, 1, 79, 1),
	RK3066_PLL_RATE(1872000000, 1, 78, 1),
	RK3066_PLL_RATE(1848000000, 1, 77, 1),
	RK3066_PLL_RATE(1824000000, 1, 76, 1),
	RK3066_PLL_RATE(1800000000, 1, 75, 1),
	RK3066_PLL_RATE(1776000000, 1, 74, 1),
	RK3066_PLL_RATE(1752000000, 1, 73, 1),
	RK3066_PLL_RATE(1728000000, 1, 72, 1),
	RK3066_PLL_RATE(1704000000, 1, 71, 1),
	RK3066_PLL_RATE(1680000000, 1, 70, 1),
	RK3066_PLL_RATE(1656000000, 1, 69, 1),
	RK3066_PLL_RATE(1632000000, 1, 68, 1),
	RK3066_PLL_RATE(1608000000, 1, 67, 1),
	RK3066_PLL_RATE(1560000000, 1, 65, 1),
	RK3066_PLL_RATE(1512000000, 1, 63, 1),
	RK3066_PLL_RATE(1488000000, 1, 62, 1),
	RK3066_PLL_RATE(1464000000, 1, 61, 1),
	RK3066_PLL_RATE(1440000000, 1, 60, 1),
	RK3066_PLL_RATE(1416000000, 1, 59, 1),
	RK3066_PLL_RATE(1392000000, 1, 58, 1),
	RK3066_PLL_RATE(1368000000, 1, 57, 1),
	RK3066_PLL_RATE(1344000000, 1, 56, 1),
	RK3066_PLL_RATE(1320000000, 1, 55, 1),
	RK3066_PLL_RATE(1296000000, 1, 54, 1),
	RK3066_PLL_RATE(1272000000, 1, 53, 1),
	RK3066_PLL_RATE(1248000000, 1, 52, 1),
	RK3066_PLL_RATE(1224000000, 1, 51, 1),
	RK3066_PLL_RATE(1200000000, 1, 50, 1),
	RK3066_PLL_RATE(1176000000, 1, 49, 1),
	RK3066_PLL_RATE(1128000000, 1, 47, 1),
	RK3066_PLL_RATE(1104000000, 1, 46, 1),
	RK3066_PLL_RATE(1008000000, 1, 84, 2),
	RK3066_PLL_RATE( 912000000, 1, 76, 2),
	RK3066_PLL_RATE( 888000000, 1, 74, 2),
	RK3066_PLL_RATE( 816000000, 1, 68, 2),
	RK3066_PLL_RATE( 792000000, 1, 66, 2),
	RK3066_PLL_RATE( 696000000, 1, 58, 2),
	RK3066_PLL_RATE( 600000000, 1, 50, 2),
	RK3066_PLL_RATE( 504000000, 1, 84, 4),
	RK3066_PLL_RATE( 408000000, 1, 68, 4),
	RK3066_PLL_RATE( 312000000, 1, 52, 4),
	RK3066_PLL_RATE( 252000000, 1, 84, 8),
	RK3066_PLL_RATE( 216000000, 1, 72, 8),
	RK3066_PLL_RATE( 126000000, 1, 84, 16),
	RK3066_PLL_RATE(  48000000, 1, 64, 32),
	{ /* sentinel */ },
};

struct rockchip_pll_rate_table rk3188_cpll_rates[] = {
	RK3066_PLL_RATE(1188000000, 2,  99, 1),
	RK3066_PLL_RATE( 798000000, 2, 133, 2),
	RK3066_PLL_RATE( 768000000, 1,  64, 2),
	RK3066_PLL_RATE( 742500000, 8, 495, 2),
	RK3066_PLL_RATE( 600000000, 1,  50, 2),
	RK3066_PLL_RATE( 594000000, 2, 198, 4),
	RK3066_PLL_RATE( 552000000, 1,  46, 2),
	RK3066_PLL_RATE( 504000000, 1,  84, 4),
	RK3066_PLL_RATE( 456000000, 1,  76, 4),
	RK3066_PLL_RATE( 408000000, 1,  68, 4),
	RK3066_PLL_RATE( 360000000, 1,  60, 4),
	{ /* sentinel */ },
};

struct rockchip_pll_rate_table rk3188_gpll_rates[] = {
	RK3066_PLL_RATE(1200000000, 1,  50, 1),
	RK3066_PLL_RATE(1188000000, 2,  99, 1),
	RK3066_PLL_RATE( 891000000, 8, 594, 2),
	RK3066_PLL_RATE( 768000000, 1,  64, 2),
	RK3066_PLL_RATE( 594000000, 2, 198, 4),
	RK3066_PLL_RATE( 384000000, 2, 128, 4),
	RK3066_PLL_RATE( 300000000, 1,  50, 4),
	RK3066_PLL_RATE( 297000000, 2, 198, 8),
	RK3066_PLL_RATE( 148500000, 2,  99, 8),
	{ /* sentinel */ },
};

static struct rockchip_pll_clock rk3188_pll_clks[] __initdata = {
	[apll] = PLL(pll_rk3066, 0, "apll", "xin24m", 0, RK2928_PLL_CON(0), RK2928_MODE_CON, 0, 6, rk3188_apll_rates),
	[dpll] = PLL(pll_rk3066, 0, "dpll", "xin24m", 0, RK2928_PLL_CON(4), RK2928_MODE_CON, 4, 5, NULL),
	[cpll] = PLL(pll_rk3066, 0, "cpll", "xin24m", 0, RK2928_PLL_CON(8), RK2928_MODE_CON, 8, 7, rk3188_cpll_rates),
	[gpll] = PLL(pll_rk3066, 0, "gpll", "xin24m", 0, RK2928_PLL_CON(12), RK2928_MODE_CON, 12, 8, rk3188_gpll_rates),
};

PNAME(mux_armclk_p)		= { "apll", "gate_cpu_gpll" };
PNAME(mux_aclk_cpu_p)		= { "apll", "gpll" };
PNAME(mux_aclk_peri_p)		= { "cpll", "gpll" };

#define MFLAGS CLK_MUX_HIWORD_MASK
static struct rockchip_mux_clock rk3188_mux_clks[] __initdata = {
	MUX(0, "mux_aclk_cpu", mux_aclk_cpu_p, RK2928_CLKSEL_CON(0), 5, 1, CLK_SET_RATE_NO_REPARENT, MFLAGS),
	MUX(0, "mux_aclk_peri", mux_aclk_peri_p, RK2928_CLKSEL_CON(10), 15, 1, CLK_SET_RATE_NO_REPARENT, MFLAGS),

};

/* 2 ^ (val + 1) */
static struct clk_div_table div_core_peri_t[] = {
	{ .val = 0, .div = 2 },
	{ .val = 1, .div = 4 },
	{ .val = 2, .div = 8 },
	{ .val = 3, .div = 16 },
	{ /* sentinel */},
};

static struct clk_div_table div_aclk_core_t[] = {
	{ .val = 0, .div = 1 },
	{ .val = 1, .div = 2 },
	{ .val = 2, .div = 3 },
	{ .val = 3, .div = 4 },
	{ .val = 4, .div = 8 },
	{ /* sentinel */},
};

#define DFLAGS CLK_DIVIDER_HIWORD_MASK
static struct rockchip_div_clock rk3188_div_clks[] __initdata = {
	/* these two are set by the cpuclk and should not be changed */
	DIV(0, "div_core_peri", "armclk", RK2928_CLKSEL_CON(0), 6, 2, 0, DFLAGS | CLK_DIVIDER_READ_ONLY, div_core_peri_t),
	DIV(0, "div_aclk_core", "armclk", RK2928_CLKSEL_CON(1), 3, 3, 0, DFLAGS | CLK_DIVIDER_READ_ONLY, div_aclk_core_t), 


	DIV(0, "div_aclk_cpu", "mux_aclk_cpu", RK2928_CLKSEL_CON(0), 0, 5, 0, DFLAGS, NULL), 
	DIV(0, "div_hclk_cpu", "gate_aclk_cpu", RK2928_CLKSEL_CON(1), 8, 2, 0, DFLAGS, NULL), 
	DIV(0, "div_pclk_cpu", "gate_aclk_cpu", RK2928_CLKSEL_CON(1), 12, 2, 0, DFLAGS, NULL), 

	DIV(0, "div_aclk_peri", "mux_aclk_peri", RK2928_CLKSEL_CON(10), 0, 5, 0, DFLAGS, NULL), 
	DIV(0, "div_hclk_peri", "gate_aclk_peri", RK2928_CLKSEL_CON(10), 8, 2, 0, DFLAGS | CLK_DIVIDER_POWER_OF_TWO, NULL), 
	DIV(0, "div_pclk_peri", "gate_aclk_peri", RK2928_CLKSEL_CON(10), 12, 2, 0, DFLAGS | CLK_DIVIDER_POWER_OF_TWO, NULL), 


};



#define GFLAGS CLK_GATE_HIWORD_MASK | CLK_GATE_SET_TO_DISABLE
static struct rockchip_gate_clock rk3188_gate_clks[] __initdata = {
	/* CLKGATE_CON_0 */
	GATE(0, "gate_core_peri", "div_core_peri", RK2928_CLKGATE_CON(0), 0, 0, GFLAGS),
	GATE(0, "gate_cpu_gpll", "gpll", RK2928_CLKGATE_CON(0), 1, 0, GFLAGS),
	GATE(0, "gate_ddr", "div_ddr", RK2928_CLKGATE_CON(0), 2, 0, GFLAGS),
	GATE(0, "gate_aclk_cpu", "div_aclk_cpu", RK2928_CLKGATE_CON(0), 3, 0, GFLAGS),
	GATE(0, "gate_hclk_cpu", "div_hclk_cpu", RK2928_CLKGATE_CON(0), 4, 0, GFLAGS),
	GATE(0, "gate_pclk_cpu", "div_pclk_cpu", RK2928_CLKGATE_CON(0), 5, 0, GFLAGS),
	GATE(0, "gate_atclk_cpu", "gate_pclk_cpu", RK2928_CLKGATE_CON(0), 6, 0, GFLAGS),
	GATE(0, "gate_aclk_core", "div_aclk_core", RK2928_CLKGATE_CON(0), 7, 0, GFLAGS),
	GATE(0, "gate_i2s0", "div_i2s0", RK2928_CLKGATE_CON(0), 9, 0, GFLAGS),
	GATE(0, "gate_i2s0_frac", "div_i2s0_frac", RK2928_CLKGATE_CON(0), 10, 0, GFLAGS),
	GATE(0, "gate_spdif", "div_spdif", RK2928_CLKGATE_CON(13), 13, 0, GFLAGS),
	GATE(0, "gate_spdif_frac", "div_spdif_frac", RK2928_CLKGATE_CON(0), 14, 0, GFLAGS),
	GATE(0, "gate_testclk", "dummy", RK2928_CLKGATE_CON(0), 15, 0, GFLAGS),

	/* CLKGATE_CON_2 */
	GATE(0, "gate_peri_src", "gate_aclk_peri", RK2928_CLKGATE_CON(2), 0, 0, GFLAGS), //FIXME, what is this
	GATE(0, "gate_aclk_peri", "div_aclk_peri", RK2928_CLKGATE_CON(2), 1, 0, GFLAGS),
	GATE(0, "gate_hclk_peri", "div_hclk_peri", RK2928_CLKGATE_CON(2), 2, 0, GFLAGS),
	GATE(0, "gate_pclk_peri", "div_pclk_peri", RK2928_CLKGATE_CON(2), 3, 0, GFLAGS),
//	GATE(0, "gate_peri_src", "gate_aclk_peri", RK2928_CLKGATE_CON(2), 0, 0, GFLAGS),

};


struct rockchip_clk_init_table rk3188_clk_init_tbl[] = {
	{ "gpll", NULL, 768000000 /* 891000000*/, 0 },

	{ "mux_aclk_cpu", "gpll", 0, 0 },
/*	{ "div_aclk_cpu", NULL, 297000000, 0 },
	{ "div_hclk_cpu", NULL, 148500000, 0 },
	{ "div_pclk_cpu", NULL,  74250000, 0 },*/
	{ "div_aclk_cpu", NULL, 300000000, 0 },
	{ "div_hclk_cpu", NULL, 150000000, 0 },
	{ "div_pclk_cpu", NULL,  75000000, 0 },

	{ "mux_aclk_peri", "gpll", 0, 0 },
/*	{ "div_aclk_peri", NULL, 148500000, 0 },
	{ "div_hclk_peri", NULL, 148500000, 0 },
	{ "div_pclk_peri", NULL,  74250000, 0 },*/
	{ "div_aclk_peri", NULL, 150000000, 0 },
	{ "div_hclk_peri", NULL, 150000000, 0 },
	{ "div_pclk_peri", NULL,  75000000, 0 },

//	{ "cpll", NULL, 594000000, 0 },
	{ "cpll", NULL, 24000000, 0 },
	{ "armclk", NULL, 1200000000, 0 },
	{ "cpll", NULL, 594000000, 0 },
};

static void __init rk3188_clock_apply_init_table(void)
{
	rockchip_clk_init_from_table(rk3188_clk_init_tbl, ARRAY_SIZE(rk3188_clk_init_tbl));
}

static void __init rk3188_clk_init(struct device_node *np)
{
	void __iomem *reg_base, *reg_grf_soc_status;

	reg_base = of_iomap(np, 0);

	rockchip_clk_init(np, reg_base, NR_CLKS);

	reg_grf_soc_status = of_iomap(np, 1);

printk("%s: registering plls\n", __func__);
	rockchip_clk_register_pll(rk3188_pll_clks, ARRAY_SIZE(rk3188_pll_clks),
				  reg_base, reg_grf_soc_status);

	rockchip_clk_register_mux(rk3188_mux_clks, ARRAY_SIZE(rk3188_mux_clks), reg_base);
	rockchip_clk_register_div(rk3188_div_clks, ARRAY_SIZE(rk3188_div_clks), reg_base);
	rockchip_clk_register_gate(rk3188_gate_clks, ARRAY_SIZE(rk3188_gate_clks), reg_base);

printk("%s: registering cpuclk\n", __func__);
	rockchip_clk_register_cpuclk(1, "armclk", mux_armclk_p, ARRAY_SIZE(mux_armclk_p), reg_base, np);

	rockchip_clk_apply_init_table = rk3188_clock_apply_init_table;
}
CLK_OF_DECLARE(rk3188_cru, "rockchip,rk3188-cru", rk3188_clk_init);
