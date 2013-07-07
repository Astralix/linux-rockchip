/*
 * clk-rockchip-debug.c
 *
 *  Created on: 30.06.2013
 *      Author: uprinz
 */

#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/hardirq.h>

#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include "clk-rockchip-pll.h"

enum rk_plls_id {
	APLL_ID = 0,
	DPLL_ID,
	CPLL_ID,
	GPLL_ID,
	END_PLL_ID,
};

/*****pmu reg offset*****/
#define PMU_PWRDN_CON          0x08
#define PMU_PWRDN_ST           0x0c

/*****cru reg offset*****/

#define CRU_MODE_CON		0x40
#define CRU_CLKSEL_CON		0x44
#define CRU_CLKGATE_CON		0xd0
#define CRU_GLB_SRST_FST	0x100
#define CRU_GLB_SRST_SND	0x104
#define CRU_SOFTRST_CON		0x110

#define PLL_CONS(id, i)		((id) * 0x10 + ((i) * 4))

#define CRU_CLKSELS_CON_CNT	(35)
#define CRU_CLKSELS_CON(i)	(CRU_CLKSEL_CON + ((i) * 4))

#define CRU_CLKGATES_CON_CNT	(10)
#define CRU_CLKGATES_CON(i)	(CRU_CLKGATE_CON + ((i) * 4))

#define CRU_SOFTRSTS_CON_CNT	(9)
#define CRU_SOFTRSTS_CON(i)	(CRU_SOFTRST_CON + ((i) * 4))

#define CRU_MISC_CON		(0x134)
#define CRU_GLB_CNT_TH		(0x140)

//#define regfile_readl(offset)	readl_relaxed(RK30_GRF_BASE + offset)
//#define regfile_writel(v, offset) do { writel_relaxed(v, RK30_GRF_BASE + offset); dsb(); } while (0)
//#define cru_readl(offset)	readl_relaxed(RK30_CRU_BASE + offset)
//#define cru_writel(v, offset)	do { writel_relaxed(v, RK30_CRU_BASE + offset); dsb(); } while (0)

static void __iomem *pmu_base_addr;
static void __iomem *cru_base_addr;
static void __iomem *pll_base_addr;
static void __iomem *timer_base_addr;

typedef struct {
	void __iomem *pll;
	char *name;
}t_xpll;

void rk30_timer_dump_regs(void)
{
	int i, r;
	u32 v;
	void __iomem *a;

	struct device_node *node;
	node = of_find_node_by_name( NULL, "timer0");
	timer_base_addr = of_iomap(node, 0);

	printk("++++++++ TIMER ++++++++++++++++\n");

	for (i = 0; i < 6; i++) {
		for( r = 0; r < 0x20; r+=4) {
			a = timer_base_addr + ( i * 0x20) + r;
			v = readl_relaxed(a);
			printk("TIMER%u:[%p] REG[0x%02x] = 0x%08x (%u)\n", i, a, r, v, v);
		}
		printk("\n");
	}

	printk("++++++++ TIMER END ++++++++++++\n");
}


static t_xpll xpll_base[] = {
	{ NULL, "rockchip,rk3066a-apll" },
	{ NULL, "rockchip,rk3066a-dpll" },
	{ NULL, "rockchip,rk3066a-cpll" },
	{ NULL, "rockchip,rk3066a-gpll" },
};


void rk30_clk_dump_regs(void)
{
	int i = 0;
	struct device_node *node;


	node = of_find_compatible_node(NULL, NULL, "rockchip,rk3066-pmu");
	pmu_base_addr = of_iomap(node, 0);
	printk("pmu_pwrdn_con: 0x%x\n", readl_relaxed(pmu_base_addr + PMU_PWRDN_CON));
	printk("pmu_pwrdn_st: 0x%x\n", readl_relaxed(pmu_base_addr + PMU_PWRDN_ST));

	printk("\nPLLRegisters:\n");
	for(i = 0; i < END_PLL_ID; i++) {

		node = of_find_compatible_node(NULL, NULL, xpll_base[i].name);
		pll_base_addr = of_iomap(node, 0);

		printk("%s :cons: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", xpll_base[i].name,
				readl_relaxed(pll_base_addr + PLL_CONS(i, 0)),
				readl_relaxed(pll_base_addr + PLL_CONS(i, 1)),
				readl_relaxed(pll_base_addr + PLL_CONS(i, 2)),
				readl_relaxed(pll_base_addr + PLL_CONS(i, 3))
		      );
	}

	/* CRU base address is same as APLL */
	node = of_find_compatible_node(NULL, NULL, "rockchip,rk3066a-apll");
	cru_base_addr = of_iomap(node, 0);

	printk("MODE        :%x\n", readl_relaxed(cru_base_addr + CRU_MODE_CON));

	for(i = 0; i < CRU_CLKSELS_CON_CNT; i++) {
		void *a = cru_base_addr + CRU_CLKSELS_CON(i);
		printk("%p:CLKSEL%d 	   :%x\n", a, i, readl_relaxed(a));
	}
	for(i = 0; i < CRU_CLKGATES_CON_CNT; i++) {
		void *a = cru_base_addr + CRU_CLKGATES_CON(i);
		printk("%p:CLKGATE%d 	  :%x\n", a, i, readl_relaxed(a));
	}
	printk("GLB_SRST_FST:%x\n", readl_relaxed(cru_base_addr + CRU_GLB_SRST_FST));
	printk("GLB_SRST_SND:%x\n", readl_relaxed(cru_base_addr + CRU_GLB_SRST_SND));

	for(i = 0; i < CRU_SOFTRSTS_CON_CNT; i++) {
		void *a = cru_base_addr + CRU_SOFTRSTS_CON(i);
		printk("%p:SOFTRST%d 	  :%x\n", a, i, readl_relaxed(a));
	}
	printk("CRU MISC    :%x\n", readl_relaxed(cru_base_addr + CRU_MISC_CON));
	printk("GLB_CNT_TH  :%x\n", readl_relaxed(cru_base_addr + CRU_GLB_CNT_TH));

	rk30_timer_dump_regs();

}

