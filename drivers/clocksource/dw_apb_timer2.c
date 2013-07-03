/*
 * (C) Copyright 2009 Intel Corporation
 * Author: Jacob Pan (jacob.jun.pan@intel.com)
 *
 * Shared with ARM platforms, Jamie Iles, Picochip 2011
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Support for the Synopsys DesignWare APB Timers.
 */
#include <linux/dw_apb_timer.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/slab.h>

#define APBT_MIN_PERIOD			4
#define APBT_MIN_DELTA_USEC		200

#define APBTMR_N_LOAD_COUNT		0x00
#define APBTMR_N_CURRENT_VALUE		0x04
#define APBTMR_N_CONTROL		0x08
#define APBTMR_N_EOI			0x0c
#define APBTMR_N_INT_STATUS		0x10

#define APBTMRS_INT_STATUS		0xa0
#define APBTMRS_EOI			0xa4
#define APBTMRS_RAW_INT_STATUS		0xa8
#define APBTMRS_COMP_VERSION		0xac

#define APBTMR_CONTROL_ENABLE		(1 << 0)
/* 1: periodic, 0:free running. */
#define APBTMR_CONTROL_MODE_PERIODIC	(1 << 1)
#define APBTMR_CONTROL_INT		(1 << 2)

static inline struct dw_apb_clock_event_device *
ced_to_dw_apb_ced(struct clock_event_device *evt)
{
	return container_of(evt, struct dw_apb_clock_event_device, ced);
}

static inline struct dw_apb_clocksource *
clocksource_to_dw_apb_clocksource(struct clocksource *cs)
{
	return container_of(cs, struct dw_apb_clocksource, cs);
}

static void apbt_init_regs(struct dw_apb_timer *timer, int quirks)
{
	if (quirks & APBTMR_QUIRK_TWO_VALUEREGS) {
		timer->reg_load_count = APBTMR_N_LOAD_COUNT;
		timer->reg_current_value = APBTMR_N_CURRENT_VALUE + 0x4;
		timer->reg_control = APBTMR_N_CONTROL + 0x8;
		/* Rockchip provides combined int_status / eoi register */
		timer->reg_eoi = NULL;
		timer->reg_int_status = APBTMR_N_EOI + 0x8;
	} else {
		timer->reg_load_count = APBTMR_N_LOAD_COUNT;
		timer->reg_current_value = APBTMR_N_CURRENT_VALUE;
		timer->reg_control = APBTMR_N_CONTROL;
		timer->reg_eoi = APBTMR_N_EOI;
		timer->reg_int_status = APBTMR_N_INT_STATUS;
	}
}

static unsigned long apbt_readl(struct dw_apb_timer *timer, unsigned long offs)
{
	return readl(timer->base + offs);
}

static void apbt_writel(struct dw_apb_timer *timer, unsigned long val,
		 unsigned long offs)
{
	writel(val, timer->base + offs);
}

static void apbt_disable_int(struct dw_apb_timer *timer)
{
	unsigned long ctrl = apbt_readl(timer, timer->reg_control);

	/* Rockchip INT bit has reverse logic */
	if (quirks & APBTMR_QUIRK_TWO_VALUEREGS)
		ctrl &= ~APBTMR_CONTROL_INT;
	else
		ctrl |= APBTMR_CONTROL_INT;
	apbt_writel(timer, ctrl, timer->reg_control);
}

/**
 * dw_apb_clockevent_pause() - stop the clock_event_device from running
 *
 * @dw_ced:	The APB clock to stop generating events.
 */
void dw_apb_clockevent_pause(struct dw_apb_clock_event_device *dw_ced)
{
	disable_irq(dw_ced->timer.irq);
	apbt_disable_int(&dw_ced->timer);
}

static void apbt_eoi(struct dw_apb_timer *timer)
{
	/* Rockchip requires to write a 1 to int_status register */
	if (timer->reg_eoi)
		apbt_readl(timer, timer->reg_eoi);
	else
		pbt_writel(timer, 1, timer->reg_int_status);
}

static irqreturn_t dw_apb_clockevent_irq(int irq, void *data)
{
	struct clock_event_device *evt = data;
	struct dw_apb_clock_event_device *dw_ced = ced_to_dw_apb_ced(evt);

	if (!evt->event_handler) {
		pr_info("Spurious APBT timer interrupt %d", irq);
		return IRQ_NONE;
	}

	if (dw_ced->eoi)
		dw_ced->eoi(&dw_ced->timer);

	evt->event_handler(evt);
	return IRQ_HANDLED;
}

static void apbt_enable_int(struct dw_apb_timer *timer)
{
	unsigned long ctrl = apbt_readl(timer, timer->reg_control);

	/* clear pending intr */
	apbt_eoi(timer);
	//apbt_readl(timer, timer->reg_eoi);
	/* Rockchip INT bit has reverse logic */
	if (quirks & APBTMR_QUIRK_TWO_VALUEREGS)
		ctrl |= APBTMR_CONTROL_INT;
	else
		ctrl &= ~APBTMR_CONTROL_INT;
	apbt_writel(timer, ctrl, timer->reg_control);
}

static void apbt_set_mode(enum clock_event_mode mode,
			  struct clock_event_device *evt)
{
	unsigned long ctrl;
	unsigned long period;
	struct dw_apb_clock_event_device *dw_ced = ced_to_dw_apb_ced(evt);
	struct dw_apb_timer *timer = &dw_ced->timer;

	pr_debug("%s CPU %d mode=%d\n", __func__, first_cpu(*evt->cpumask),
		 mode);

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		period = DIV_ROUND_UP(timer->freq, HZ);
		ctrl = apbt_readl(timer, timer->reg_control);
		ctrl |= APBTMR_CONTROL_MODE_PERIODIC;
		apbt_writel(timer, ctrl, timer->reg_control);
		/*
		 * DW APB p. 46, have to disable timer before load counter,
		 * may cause sync problem.
		 */
		ctrl &= ~APBTMR_CONTROL_ENABLE;
		apbt_writel(timer, ctrl, timer->reg_control);
		udelay(1);
		pr_debug("Setting clock period %lu for HZ %d\n", period, HZ);
		apbt_writel(timer, period, timer->reg_load_count);

		if (timer->quirks & APBTMR_QUIRK_TWO_VALUEREGS)
			apbt_writel(timer, 0, timer->reg_load_count + 0x4);

		ctrl |= APBTMR_CONTROL_ENABLE;
		apbt_writel(timer, ctrl, timer->reg_control);
		break;

	case CLOCK_EVT_MODE_ONESHOT:
		ctrl = apbt_readl(timer, timer->reg_control);
		/*
		 * set free running mode, this mode will let timer reload max
		 * timeout which will give time (3min on 25MHz clock) to rearm
		 * the next event, therefore emulate the one-shot mode.
		 */
		ctrl &= ~APBTMR_CONTROL_ENABLE;
		ctrl &= ~APBTMR_CONTROL_MODE_PERIODIC;

		apbt_writel(timer, ctrl, timer->reg_control);
		/* write again to set free running mode */
		apbt_writel(timer, ctrl, timer->reg_control);

		/*
		 * DW APB p. 46, load counter with all 1s before starting free
		 * running mode.
		 */
		apbt_writel(timer, ~0, timer->reg_load_count);

		if (timer->quirks & APBTMR_QUIRK_TWO_VALUEREGS) {
			/* Rockchip reverse logic */
			apbt_writel(timer, 0, timer->reg_load_count + 0x4);
			ctrl |= APBTMR_CONTROL_INT;
		}
		else {
			ctrl &= ~APBTMR_CONTROL_INT;
		}
		ctrl |= APBTMR_CONTROL_ENABLE;
		apbt_writel(timer, ctrl, timer->reg_control);
		break;

	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		ctrl = apbt_readl(timer, timer->reg_control);
		ctrl &= ~APBTMR_CONTROL_ENABLE;
		apbt_writel(timer, ctrl, timer->reg_control);
		break;

	case CLOCK_EVT_MODE_RESUME:
		apbt_enable_int(timer);
		break;
	}
}

static int apbt_next_event(unsigned long delta,
			   struct clock_event_device *evt)
{
	unsigned long ctrl;
	struct dw_apb_clock_event_device *dw_ced = ced_to_dw_apb_ced(evt);
	struct dw_apb_timer *timer = &dw_ced->timer;

	/* Disable timer */
	ctrl = apbt_readl(timer, timer->reg_control);
	ctrl &= ~APBTMR_CONTROL_ENABLE;
	apbt_writel(timer, ctrl, timer->reg_control);
	/* write new count */
	apbt_writel(timer, delta, timer->reg_load_count);

	if (timer->quirks & APBTMR_QUIRK_TWO_VALUEREGS)
		apbt_writel(timer, 0, timer->reg_load_count + 0x4);

	ctrl |= APBTMR_CONTROL_ENABLE;
	apbt_writel(timer, ctrl, timer->reg_control);

	return 0;
}

/**
 * dw_apb_clockevent_init() - use an APB timer as a clock_event_device
 *
 * @cpu:	The CPU the events will be targeted at.
 * @name:	The name used for the timer and the IRQ for it.
 * @rating:	The rating to give the timer.
 * @base:	I/O base for the timer registers.
 * @irq:	The interrupt number to use for the timer.
 * @freq:	The frequency that the timer counts at.
 *
 * This creates a clock_event_device for using with the generic clock layer
 * but does not start and register it.  This should be done with
 * dw_apb_clockevent_register() as the next step.  If this is the first time
 * it has been called for a timer then the IRQ will be requested, if not it
 * just be enabled to allow CPU hotplug to avoid repeatedly requesting and
 * releasing the IRQ.
 */
struct dw_apb_clock_event_device *
dw_apb_clockevent_init(int cpu, const char *name, unsigned rating,
		       void __iomem *base, int irq, unsigned long freq,
		       int quirks)
{
	struct dw_apb_clock_event_device *dw_ced =
		kzalloc(sizeof(*dw_ced), GFP_KERNEL);
	int err;

	if (!dw_ced)
		return NULL;

	dw_ced->timer.base = base;
	dw_ced->timer.irq = irq;
	dw_ced->timer.freq = freq;
	dw_ced->timer.quirks = quirks;
	apbt_init_regs(&dw_ced->timer, quirks);

	clockevents_calc_mult_shift(&dw_ced->ced, freq, APBT_MIN_PERIOD);
	dw_ced->ced.max_delta_ns = clockevent_delta2ns(0x7fffffff,
						       &dw_ced->ced);
	dw_ced->ced.min_delta_ns = clockevent_delta2ns(5000, &dw_ced->ced);
	dw_ced->ced.cpumask = cpumask_of(cpu);
	dw_ced->ced.features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT;
	dw_ced->ced.set_mode = apbt_set_mode;
	dw_ced->ced.set_next_event = apbt_next_event;
	dw_ced->ced.irq = dw_ced->timer.irq;
	dw_ced->ced.rating = rating;
	dw_ced->ced.name = name;

	dw_ced->irqaction.name		= dw_ced->ced.name;
	dw_ced->irqaction.handler	= dw_apb_clockevent_irq;
	dw_ced->irqaction.dev_id	= &dw_ced->ced;
	dw_ced->irqaction.irq		= irq;
	dw_ced->irqaction.flags		= IRQF_TIMER | IRQF_IRQPOLL |
					  IRQF_NOBALANCING |
					  IRQF_DISABLED;

	dw_ced->eoi = apbt_eoi;
	err = setup_irq(irq, &dw_ced->irqaction);
	if (err) {
		pr_err("failed to request timer irq\n");
		kfree(dw_ced);
		dw_ced = NULL;
	}

	return dw_ced;
}

/**
 * dw_apb_clockevent_resume() - resume a clock that has been paused.
 *
 * @dw_ced:	The APB clock to resume.
 */
void dw_apb_clockevent_resume(struct dw_apb_clock_event_device *dw_ced)
{
	enable_irq(dw_ced->timer.irq);
}

/**
 * dw_apb_clockevent_stop() - stop the clock_event_device and release the IRQ.
 *
 * @dw_ced:	The APB clock to stop generating the events.
 */
void dw_apb_clockevent_stop(struct dw_apb_clock_event_device *dw_ced)
{
	free_irq(dw_ced->timer.irq, &dw_ced->ced);
}

/**
 * dw_apb_clockevent_register() - register the clock with the generic layer
 *
 * @dw_ced:	The APB clock to register as a clock_event_device.
 */
void dw_apb_clockevent_register(struct dw_apb_clock_event_device *dw_ced)
{
	struct dw_apb_timer *timer = &dw_ced->timer;

	apbt_writel(timer, 0, timer->reg_control);
	clockevents_register_device(&dw_ced->ced);
	apbt_enable_int(timer);
}

/**
 * dw_apb_clocksource_start() - start the clocksource counting.
 *
 * @dw_cs:	The clocksource to start.
 *
 * This is used to start the clocksource before registration and can be used
 * to enable calibration of timers.
 */
void dw_apb_clocksource_start(struct dw_apb_clocksource *dw_cs)
{
	struct dw_apb_timer *timer = &dw_cs->timer;
	/*
	 * start count down from 0xffff_ffff. this is done by toggling the
	 * enable bit then load initial load count to ~0.
	 */
	unsigned long ctrl = apbt_readl(timer, timer->reg_control);

	ctrl &= ~APBTMR_CONTROL_ENABLE;
	apbt_writel(timer, ctrl, timer->reg_control);
	apbt_writel(timer, ~0, timer->reg_load_count);

	if (timer->quirks & APBTMR_QUIRK_TWO_VALUEREGS)
		apbt_writel(timer, 0, timer->reg_load_count + 0x4);

	/* enable, mask interrupt */
	ctrl &= ~APBTMR_CONTROL_MODE_PERIODIC;
	ctrl |= (APBTMR_CONTROL_ENABLE | APBTMR_CONTROL_INT);
	apbt_writel(timer, ctrl, timer->reg_control);
	/* read it once to get cached counter value initialized */
	dw_apb_clocksource_read(dw_cs);
}

static int i;
static cycle_t __apbt_read_clocksource(struct clocksource *cs)
{
	unsigned long current_count;
	struct dw_apb_clocksource *dw_cs =
		clocksource_to_dw_apb_clocksource(cs);
	struct dw_apb_timer *timer = &dw_cs->timer;

	current_count = apbt_readl(timer, timer->reg_current_value);

	/* temporary output to check if this is really a 64bit value spread
	 * over two registers.
	 */
	if (timer->quirks & APBTMR_QUIRK_TWO_VALUEREGS) {
		if (i > 100) {
			printk("clocksource val0: %lu, val1: %lu\n", current_count,
			       apbt_readl(timer, timer->reg_current_value + 0x04));

			i = 0;
		}
		i++;
	}

	return (cycle_t)~current_count;
}

static void apbt_restart_clocksource(struct clocksource *cs)
{
	struct dw_apb_clocksource *dw_cs =
		clocksource_to_dw_apb_clocksource(cs);

	dw_apb_clocksource_start(dw_cs);
}

/**
 * dw_apb_clocksource_init() - use an APB timer as a clocksource.
 *
 * @rating:	The rating to give the clocksource.
 * @name:	The name for the clocksource.
 * @base:	The I/O base for the timer registers.
 * @freq:	The frequency that the timer counts at.
 *
 * This creates a clocksource using an APB timer but does not yet register it
 * with the clocksource system.  This should be done with
 * dw_apb_clocksource_register() as the next step.
 */
struct dw_apb_clocksource *
dw_apb_clocksource_init(unsigned rating, const char *name, void __iomem *base,
			unsigned long freq, int quirks)
{
	struct dw_apb_clocksource *dw_cs = kzalloc(sizeof(*dw_cs), GFP_KERNEL);

	if (!dw_cs)
		return NULL;

	dw_cs->timer.base = base;
	dw_cs->timer.freq = freq;
	dw_cs->timer.quirks = quirks;
	apbt_init_regs(&dw_cs->timer, quirks);

	dw_cs->cs.name = name;
	dw_cs->cs.rating = rating;
	dw_cs->cs.read = __apbt_read_clocksource;
	dw_cs->cs.mask = CLOCKSOURCE_MASK(32);
	dw_cs->cs.flags = CLOCK_SOURCE_IS_CONTINUOUS;
	dw_cs->cs.resume = apbt_restart_clocksource;

	return dw_cs;
}

/**
 * dw_apb_clocksource_register() - register the APB clocksource.
 *
 * @dw_cs:	The clocksource to register.
 */
void dw_apb_clocksource_register(struct dw_apb_clocksource *dw_cs)
{
	clocksource_register_hz(&dw_cs->cs, dw_cs->timer.freq);
}

/**
 * dw_apb_clocksource_read() - read the current value of a clocksource.
 *
 * @dw_cs:	The clocksource to read.
 */
cycle_t dw_apb_clocksource_read(struct dw_apb_clocksource *dw_cs)
{
	struct dw_apb_timer *timer = &dw_cs->timer;
	return (cycle_t)~apbt_readl(timer, timer->reg_current_value);
}

/**
 * dw_apb_clocksource_unregister() - unregister and free a clocksource.
 *
 * @dw_cs:	The clocksource to unregister/free.
 */
void dw_apb_clocksource_unregister(struct dw_apb_clocksource *dw_cs)
{
	clocksource_unregister(&dw_cs->cs);

	kfree(dw_cs);
}
