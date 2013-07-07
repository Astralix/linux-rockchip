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

/* register definitions of 32bits variant */
#define APBTMR_N_LOAD_COUNT		0x00
#define APBTMR_N_CURRENT_VALUE		0x04
#define APBTMR_N_CONTROL		0x08
#define APBTMR_N_EOI			0x0c
#define APBTMR_N_INT_STATUS		0x10

#define APBTMRS_INT_STATUS		0xa0
#define APBTMRS_EOI			0xa4
#define APBTMRS_RAW_INT_STATUS		0xa8
#define APBTMRS_COMP_VERSION		0xac

/* register definitions of 64bits variant */
#define APBTMR_N64_LOAD_COUNTL		0x00
#define APBTMR_N64_LOAD_COUNTH		0x04
#define APBTMR_N64_CURRENT_VALUEL	0x08
#define APBTMR_N64_CURRENT_VALUEH	0x0c
#define APBTMR_N64_CONTROL		0x10
#define APBTMR_N64_INT_STATUS		0x18



#define APBTMR_CONTROL_ENABLE		(1 << 0)
/* 1: periodic, 0:free running. */
#define APBTMR_CONTROL_MODE_PERIODIC	(1 << 1)
#define APBTMR_CONTROL_INT		(1 << 2)

/**
 * Common hardware access functions.
 */
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


static unsigned long apbt_readl(struct dw_apb_timer *timer, 
		unsigned long offs)
{
	return readl(timer->base + offs);
}

static void apbt_writel(struct dw_apb_timer *timer, unsigned long val,
		 unsigned long offs)
{
	writel(val, timer->base + offs);
}

static void tmr_bit_set(struct dw_apb_timer *timer, unsigned long offs, 
		unsigned long bits)
{
	apbt_writel( timer, (apbt_readl(timer, offs) | bits), offs);
}

static void tmr_bit_clr(struct dw_apb_timer *timer, unsigned long offs, 
		unsigned long bits)
{
	apbt_writel( timer, (apbt_readl(timer, offs) & ~bits), offs);
}


static void apbt_timer_init( struct dw_apb_timer *timer)
{
 	apbt_writel(timer, 0, APBTMR_N_CONTROL);
}

/**
 * DW_APB implementattion of the timers.
 */

static void apbt_load32( struct dw_apb_timer *timer,unsigned long period)
{
	apbt_writel(timer, period, APBTMR_N_LOAD_COUNT);
}

static unsigned long apbt_read32( struct dw_apb_timer *timer)
{
	return apbt_readl(timer, APBTMR_N_CURRENT_VALUE);
}

/**
 * apb_tmr_start() - Start the timer in the previously set mode
 *
 * @timer:	The timer to start.
 */
static void apb_tmr_start( struct dw_apb_timer *timer)
{
	tmr_bit_set(timer, APBTMR_N_CONTROL, APBTMR_CONTROL_ENABLE);
}

/**
 * apb_tmr_stop() - Stop the timer
 *
 * @timer:	The timer to stop.
 */
static void apb_tmr_stop( struct dw_apb_timer *timer)
{
	tmr_bit_set(timer, APBTMR_N_CONTROL, APBTMR_CONTROL_ENABLE);
}

static void apbt_eoi(struct dw_apb_timer *timer)
{
	apbt_readl(timer, APBTMR_N_EOI);
}

 static void apbt_disable_int(struct dw_apb_timer *timer)
{
	tmr_bit_set(timer, APBTMR_N_CONTROL, APBTMR_CONTROL_INT);
}

static void apbt_enable_int(struct dw_apb_timer *timer)
{
	/* clear pending intr */
	timer->eoi(timer);
	/* enable interrupt */
	tmr_bit_clr(timer, APBTMR_N_CONTROL, APBTMR_CONTROL_INT);
}

static void apb_set_periodic( struct dw_apb_timer *timer)
{
	tmr_bit_set(timer, APBTMR_N_CONTROL, APBTMR_CONTROL_MODE_PERIODIC);
}

static void apb_set_oneshot( struct dw_apb_timer *timer)
{
	tmr_bit_clr(timer, APBTMR_N_CONTROL, APBTMR_CONTROL_MODE_PERIODIC);
}


/**
 * Rockchips RK3188 implementation of the timers.
 */

static void rkc_timer_init( struct dw_apb_timer *timer)
{
	printk("%s: %p\n", __func__, timer->base);
 	apbt_writel(timer, 0, APBTMR_N64_CONTROL);
}

static void rkc_load32( struct dw_apb_timer *timer, unsigned long period)
{
	printk("%s: %p\n", __func__, timer->base);
	apbt_writel(timer, period, APBTMR_N64_LOAD_COUNTL);
	apbt_writel(timer, 0, APBTMR_N64_LOAD_COUNTH);
}

static unsigned long rkc_read32( struct dw_apb_timer *timer)
{
	return apbt_readl(timer, APBTMR_N64_CURRENT_VALUEL);
}

/**
 * rkc_tmr_start() - Start the timer in the previously set mode
 *
 * @timer:	The timer to start.
 */
static void rkc_tmr_start( struct dw_apb_timer *timer)
{
	tmr_bit_set(timer, APBTMR_N64_CONTROL, APBTMR_CONTROL_ENABLE);
}

/**
 * rkc_tmr_stop() - Stop the timer, Rockchips variant
 *
 * @timer:	The timer to stop.
 */
static void rkc_tmr_stop( struct dw_apb_timer *timer)
{
	tmr_bit_clr(timer, APBTMR_N64_CONTROL, APBTMR_CONTROL_ENABLE);
}

static void rkc_eoi(struct dw_apb_timer *timer)
{
	apbt_writel(timer, 1, APBTMR_N64_INT_STATUS);
}

static void rkc_disable_int(struct dw_apb_timer *timer)
{
	tmr_bit_clr(timer, APBTMR_N64_CONTROL, APBTMR_CONTROL_INT);
}

static void rkc_enable_int(struct dw_apb_timer *timer)
{
	/* clear pending intr */
	timer->eoi(timer);
	/* enable interrupt */
	tmr_bit_set(timer, APBTMR_N64_CONTROL, APBTMR_CONTROL_INT);
}

static void rkc_set_periodic( struct dw_apb_timer *timer)
{
	tmr_bit_clr(timer, APBTMR_N64_CONTROL, APBTMR_CONTROL_MODE_PERIODIC);
}

static void rkc_set_oneshot( struct dw_apb_timer *timer)
{
	tmr_bit_set(timer, APBTMR_N64_CONTROL, APBTMR_CONTROL_MODE_PERIODIC);
}

static irqreturn_t dw_apb_clockevent_irq(int irq, void *data)
{
	struct clock_event_device *evt = data;
	struct dw_apb_clock_event_device *dw_ced = ced_to_dw_apb_ced(evt);

	printk("%s: %p\n", __func__, dw_ced->timer->base);
	if (!evt->event_handler) {
		pr_info("Spurious APBT timer interrupt %d", irq);
		return IRQ_NONE;
	}

	if (dw_ced->eoi)
		dw_ced->eoi(dw_ced->timer);

	evt->event_handler(evt);
	return IRQ_HANDLED;
}

/**
 * dw_apb_clockevent_pause() - stop the clock_event_device from running
 *
 * @dw_ced:	The APB clock to stop generating events.
 */
void dw_apb_clockevent_pause(struct dw_apb_clock_event_device *dw_ced)
{
	disable_irq(dw_ced->timer->irq);
	apbt_disable_int(dw_ced->timer);
}

static void apbt_set_mode(enum clock_event_mode mode,
			  struct clock_event_device *evt)
{
	unsigned long period;
	struct dw_apb_clock_event_device *dw_ced = ced_to_dw_apb_ced(evt);
	struct dw_apb_timer *timer = dw_ced->timer;

	pr_debug("%s CPU %d mode=%d\n", __func__, first_cpu(*evt->cpumask),
		 mode);

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		period = DIV_ROUND_UP(timer->freq, HZ);

		timer->tmr_mode_periodic(timer);
		/*
		 * DW APB p. 46, have to disable timer before load counter,
		 * may cause sync problem.
		 */
		timer->tmr_stop(timer);
		udelay(1);
		pr_debug("Setting clock period %lu for HZ %d\n", period, HZ);
		timer->tmr_load(timer, period);
		// timer->tmr_enable_int(timer);
		timer->tmr_start(timer);
		break;

	case CLOCK_EVT_MODE_ONESHOT:
		timer->tmr_mode_oneshot(timer);
		/*
		 * set free running mode, this mode will let timer reload max
		 * timeout which will give time (3min on 25MHz clock) to rearm
		 * the next event, therefore emulate the one-shot mode.
		 */
		timer->tmr_stop(timer);
		/*
		 * DW APB p. 46, load counter with all 1s before starting free
		 * running mode.
		 */
		timer->tmr_load(timer, ~0);
		timer->tmr_enable_int(timer);
		timer->tmr_start(timer);
		break;

	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		timer->tmr_stop(timer);
		break;

	case CLOCK_EVT_MODE_RESUME:
		apbt_enable_int(timer);
		break;
	}
}

static int apbt_next_event(unsigned long delta,
			   struct clock_event_device *evt)
{
	struct dw_apb_clock_event_device *dw_ced = ced_to_dw_apb_ced(evt);
	struct dw_apb_timer *timer = dw_ced->timer;

	/* Disable timer */
	timer->tmr_stop(timer);
	/* write new count */
	timer->tmr_load(timer, delta);
	timer->tmr_start(timer);

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
		       void __iomem *base, struct dw_apb_timer *ptimer,
		       int irq, unsigned long freq)
{
	struct dw_apb_clock_event_device *dw_ced =
		kzalloc(sizeof(*dw_ced), GFP_KERNEL);
	int err;

	if (!dw_ced)
		return NULL;

	if (ptimer == NULL) {
		pr_err("no valid ptimer to register clockevent\n");
		kfree(dw_ced);
		return NULL;
	}

	dw_ced->timer = ptimer;
	dw_ced->timer->base = base;
	dw_ced->timer->irq = irq;
	dw_ced->timer->freq = freq;
	dw_ced->eoi = ptimer->eoi;

	printk("%s:[%p] irq %u, frq %lu\n", __func__, dw_ced->timer, irq, freq);

	clockevents_calc_mult_shift(&dw_ced->ced, freq, APBT_MIN_PERIOD);
	dw_ced->ced.max_delta_ns = clockevent_delta2ns(0x7fffffff,
						       &dw_ced->ced);
	dw_ced->ced.min_delta_ns = clockevent_delta2ns(5000, &dw_ced->ced);
	dw_ced->ced.cpumask = cpumask_of(cpu);
	dw_ced->ced.features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT;
	dw_ced->ced.set_mode = apbt_set_mode;
	dw_ced->ced.set_next_event = apbt_next_event;
	dw_ced->ced.irq = dw_ced->timer->irq;
	dw_ced->ced.rating = rating;
	dw_ced->ced.name = name;

	dw_ced->irqaction.name		= dw_ced->ced.name;
	dw_ced->irqaction.handler	= dw_apb_clockevent_irq;
	dw_ced->irqaction.dev_id	= &dw_ced->ced;
	dw_ced->irqaction.irq		= irq;
	dw_ced->irqaction.flags		= IRQF_TIMER | IRQF_IRQPOLL |
					  IRQF_NOBALANCING |
					  IRQF_DISABLED;

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
	enable_irq(dw_ced->timer->irq);
}

/**
 * dw_apb_clockevent_stop() - stop the clock_event_device and release the IRQ.
 *
 * @dw_ced:	The APB clock to stop generating the events.
 */
void dw_apb_clockevent_stop(struct dw_apb_clock_event_device *dw_ced)
{
	free_irq(dw_ced->timer->irq, &dw_ced->ced);
}

/**
 * dw_apb_clockevent_register() - register the clock with the generic layer
 *
 * @dw_ced:	The APB clock to register as a clock_event_device.
 */
void dw_apb_clockevent_register(struct dw_apb_clock_event_device *dw_ced)
{
	struct dw_apb_timer *timer = dw_ced->timer;

	timer->tmr_init(timer);
	clockevents_register_device(&dw_ced->ced);
	timer->tmr_enable_int(timer);
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
	struct dw_apb_timer *timer = dw_cs->timer;
	/*
	 * start count down from 0xffff_ffff. This is done by toggling the
	 * enable bit then load initial load count to ~0.
	 */
	timer->tmr_stop(timer);
	timer->tmr_load(timer, ~0);
	/* set periodic, mask interrupt, enable timer */
	timer->tmr_mode_periodic(timer);
	timer->tmr_disable_int(timer);
	timer->tmr_start(timer);
	/* read it once to get cached counter value initialized */
	dw_apb_clocksource_read(dw_cs);
}

static cycle_t __apbt_read_clocksource(struct clocksource *cs)
{
	struct dw_apb_clocksource *dw_cs =
		clocksource_to_dw_apb_clocksource(cs);
	struct dw_apb_timer *timer = dw_cs->timer;

	return (cycle_t)~(timer->tmr_read)(timer);
}

static void apbt_restart_clocksource(struct clocksource *cs)
{
	struct dw_apb_clocksource *dw_cs =
		clocksource_to_dw_apb_clocksource(cs);

	dw_apb_clocksource_start(dw_cs);
}

struct dw_apb_timer rkc_apbt_32 = {
	.eoi = rkc_eoi,
	.tmr_init = rkc_timer_init,
	.tmr_start = rkc_tmr_start,
	.tmr_stop = rkc_tmr_stop,
	.tmr_load = rkc_load32,
	.tmr_read = rkc_read32,
	.tmr_enable_int = rkc_enable_int,
	.tmr_disable_int = rkc_disable_int,
	.tmr_mode_oneshot = rkc_set_oneshot,
	.tmr_mode_periodic = rkc_set_periodic,

};

struct dw_apb_timer dw_apbt_32 = {
	.eoi = apbt_eoi,
	.tmr_init = apbt_timer_init,
	.tmr_start = apb_tmr_start,
	.tmr_stop = apb_tmr_stop,
	.tmr_load = apbt_load32,
	.tmr_read = apbt_read32,
	.tmr_enable_int = apbt_enable_int,
	.tmr_disable_int = apbt_disable_int,
	.tmr_mode_oneshot = apb_set_oneshot,
	.tmr_mode_periodic = apb_set_periodic,
};

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
			struct dw_apb_timer *ptimer, unsigned long freq)
{
	struct dw_apb_clocksource *dw_cs = kzalloc(sizeof(*dw_cs), GFP_KERNEL);

	if (!dw_cs)
		return NULL;

	if (ptimer == NULL) {
		pr_err("no valid ptimer to register clocksource\n");
		kfree(dw_cs);
		return NULL;
	}

	dw_cs->timer = ptimer;
	dw_cs->timer->base = base;
	dw_cs->timer->freq = freq;

	printk("%s:[%p] frq %lu\n", __func__, dw_cs->timer, freq);

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
	clocksource_register_hz(&dw_cs->cs, dw_cs->timer->freq);
}

/**
 * dw_apb_clocksource_read() - read the current value of a clocksource.
 *
 * @dw_cs:	The clocksource to read.
 */
cycle_t dw_apb_clocksource_read(struct dw_apb_clocksource *dw_cs)
{
	struct dw_apb_timer *timer = dw_cs->timer;
	return (cycle_t)~(timer->tmr_read)(timer);
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
