#ifndef _RK_PHY_CONTROL_H_
#define _RK_PHY_CONTROL_H_

/* USB OTG 2.0 Interface */
#define GRF_UOC0_CON0	(0x00) /* 0x010c W 0x0000c889 otg control register */
#define GRF_UOC0_CON1	(0x04) /* 0x0110 W 0x00007333 otg control register */
#define GRF_UOC0_CON2	(0x08) /* 0x0114 W 0x00000da8 otg control register */
#define GRF_UOC0_CON3	(0x0c) /* 0x0118 W 0x00000001 otg control register */

/* USB HOST 2.0 Interface */
#define GRF_UOC1_CON0	(0x00) /* 0x011c W 0x0000c889 host control register */
#define GRF_UOC1_CON1	(0x04) /* 0x0120 W 0x00007333 host control register */
#define GRF_UOC1_CON2	(0x08) /* 0x0124 W 0x00000da8 host control register */
#define GRF_UOC1_CON3	(0x0c) /* 0x0128 W 0x00000001 host control register */

/* USB HSIC 2.0 Interface */
#define GRF_UOC2_CON0	(0x00) /* 0x012c W 0x000030eb hsic phy control reg */
#define GRF_UOC2_CON1	(0x04) /* 0x0130 W 0x00000003 hsic phy control reg */
#define GRF_UOC3_CON0	(0x0c) /* 0x0138 W 0x00000080 hsic control register */
#define GRF_UOC3_CON1	(0x10) /* 0x013c W 0x00000820 hsic control register */
#define GRF_HSIC_STAT	(0x14) /* 0x0140 W 0x00000000 hsic status register */

/* General SOC Status Register 0 */
#define GRF_SOC_STATUS0 (0x00ac)
/* General SOC Status Register 0 - USB HOST Flags */
#define STAT_UHOST_ADPPRB 		(1 << 22) /* adpprb signal status */
#define STAT_UHOST_ADPSNS 		(1 << 21) /* adpsns status */
#define STAT_UHOST_UTMI_IDDIG	(1 << 20) /* iddig signal status */
#define	STAT_UHOST_UTMI_LSTATE	(3 << 18) /* linstate signal status */
#define STAT_UHOST_UTMI_BVALID	(1 << 17) /* bvalid status signal */
#define STAT_UHOST_UTMI_VBUSV	(1 << 16) /* vbusvalid signal status */

/* General SOC Status Register 0 - USB OTG Flags */
#define STAT_OTG_ADPPRB			(1 << 15) /* adpprb signal status */
#define STAT_OTG_ADPSNS 		(1 << 14) /* adpsns status */
#define STAT_OTG_UTMI_IDDIG		(1 << 13) /* iddig signal status */
#define STAT_OTG_UTMI_LSTATE	(3 << 11) /* linestate signal status */
#define STAT_OTG_UTMI_BVALID	(1 << 10) /* bvalid status signal */
#define STAT_OTG_UTMI_VBUSV		(1 <<  9) /* vbusvalid signal status */

// TODO: register setup and flags are identical for HOST and OTG
// But OTG has some more bits
// So HOST can be driven like OTG but use some other MASKs to avoid
// incorrect settings.
/*
 * Bits of GRF_UOCx_CON0 registers
 */
#define UPHY_TXBITSTUFF_ENH		(1 << 15) /* Enable High-Byte TX Bit-Stuffing */
#define UPHY_TXBITSTUFF_ENL		(1 << 14) /* Enable Low-Byte TX Bit-Stuffing */
#define UPHY_TEST_IDDQ			(1 << 13) /* IDDQ Test Enable */
#define UPHY_PORT_RESET			(1 << 12) /* Per-Port Reset */
#define UPHY_REFCLK_SEL_CORE	(2 << 10) /* Use CLKCORE as reference */
#define UPHY_REFCLK_SEL_EXT		(1 << 10) /* 1.8V clock via XO pin */
#define UPHY_REFCLK_SEL_XTAL	(0 << 10) /* crystal supplied clock */
#define UPHY_REFCLK_SEL_MASK	(0 << 10) /* crystal supplied clock */
#define UPHY_BYPASS_SEL			(1 <<  9) /* tx digital bypass mode enable */
#define UPHY_BYPASS_DMEN		(1 <<  8) /* DM0 FS/LS driver enabled */
#define UPHY_OTG_TUNE_P9		(7 <<  5) /*  +9% VBUS valid adjust */
#define UPHY_OTG_TUNE_P6		(6 <<  5) /*  +6% VBUS valid adjust */
#define UPHY_OTG_TUNE_P3		(5 <<  5) /*  +3% VBUS valid adjust */
#define UPHY_OTG_TUNE_0			(4 <<  5) /* normal VBUS valid adjust */
#define UPHY_OTG_TUNE_N3		(3 <<  5) /*  -3% VBUS valid adjust */
#define UPHY_OTG_TUNE_N6		(2 <<  5) /*  -6% VBUS valid adjust */
#define UPHY_OTG_TUNE_N9		(1 <<  5) /*  -9% VBUS valid adjust */
#define UPHY_OTG_TUNE_N12		(0 <<  5) /* -12% VBUS valid adjust */
#define UPHY_OTG_TUNE_MASK		(7 <<  5)
#define UPHY_OTG_DISBALE		(1 <<  4) /* 1: OTG block power down */
#define UPHY_COMPDISTUNE_P45	(7 <<  1) /* +4.5% Disconnect Threshold Adj */
#define UPHY_COMPDISTUNE_P30 	(6 <<  1) /* +3.0% Disconnect Threshold Adj */
#define UPHY_COMPDISTUNE_P15 	(5 <<  1) /* +1.5% Disconnect Threshold Adj */
#define UPHY_COMPDISTUNE_0		(4 <<  1) /* Normal Disconnect Threshold Adj */
#define UPHY_COMPDISTUNE_N15 	(3 <<  1) /* -1.5% Disconnect Threshold Adj */
#define UPHY_COMPDISTUNE_N30 	(2 <<  1) /* -3.0% Disconnect Threshold Adj */
#define UPHY_COMPDISTUNE_N45 	(1 <<  1) /* -4.5% Disconnect Threshold Adj */
#define UPHY_COMPDISTUNE_N60 	(0 <<  1) /* -6.0% Disconnect Threshold Adj */
#define UPHY_COMPDISTUNE_MASK 	(7 <<  1)
#define UPHY_POWER_DOWN			(1 <<  0) /* 1: Power down in Suspend & Sleep */

/*
 * Bits of GRF_UOCx_CON1 registers
 */
#define UPHY_TXRISE_TUNE_N20	(3 << 14) /* -20% HSTX adjust rise/fall times */
#define UPHY_TXRISE_TUNE_N15	(2 << 14) /* -15% HSTX adjust rise/fall times */
#define UPHY_TXRISE_TUNE_0		(1 << 14) /* HSTX default rise/fall times */
#define UPHY_TXRISE_TUNE_P10	(0 << 14) /* +10% HSTX adjust rise/fall times */
#define UPHY_TXRISE_TUNE_MASK	(0 << 14)
#define UPHY_TXRISE_TUNE_0		(3 << 12) /* defaul HSTX Crossover Point */
#define UPHY_TXRISE_TUNE_p15	(2 << 12) /* +15mV HSTX Crossover Adjustment */
#define UPHY_TXRISE_TUNE_m15	(1 << 12) /* -15mV HSTX Crossover Adjustment */
#define UPHY_TXRISE_TUNE_mask	(3 << 12)
#define UPHY_TXVREF_TUNE_P875	(15 << 8) /* +8.75% HS DC level adj */
#define UPHY_TXVREF_TUNE_P750	(14 << 8) /* +7.50% HS DC level adj */
#define UPHY_TXVREF_TUNE_P625	(13 << 8) /* +6.25% HS DC level adj */
#define UPHY_TXVREF_TUNE_P500	(12 << 8) /* +5.00% HS DC level adj */
#define UPHY_TXVREF_TUNE_P375	(11 << 8) /* +3.75% HS DC level adj */
#define UPHY_TXVREF_TUNE_P250	(10 << 8) /* +2.50% HS DC level adj */
#define UPHY_TXVREF_TUNE_P125	( 9 << 8) /* +1.25% HS DC level adj */
#define UPHY_TXVREF_TUNE_0		( 8 << 8) /*  0.00% HS DC level adj */
#define UPHY_TXVREF_TUNE_N125	( 7 << 8) /* -1.25% HS DC level adj */
#define UPHY_TXVREF_TUNE_N250	( 6 << 8) /* -2.50% HS DC level adj */
#define UPHY_TXVREF_TUNE_N375	( 5 << 8) /* -3.75% HS DC level adj */
#define UPHY_TXVREF_TUNE_N500	( 4 << 8) /* -5.00% HS DC level adj */
#define UPHY_TXVREF_TUNE_N625	( 3 << 8) /* -6.25% HS DC level adj */
#define UPHY_TXVREF_TUNE_N750	( 2 << 8) /* -7.50% HS DC level adj */
#define UPHY_TXVREF_TUNE_N875	( 1 << 8) /* -8.75% HS DC level adj */
#define UPHY_TXVREF_TUNE_N100	( 0 << 8) /* -10.0% HS DC level adj */
#define UPHY_TXVREF_TUNE_MASK	(15 << 8)
#define UPHY_TXFSLS_TUNE_N50	(15 << 4) /* -5.0% FS/LS Source Impedance Adj */
#define UPHY_TXFSLS_TUNE_N25	( 7 << 4) /* -2.5% FS/LS Source Impedance Adj */
#define UPHY_TXFSLS_TUNE_0		( 3 << 4) /*    0% FS/LS Source Impedance Adj */
#define UPHY_TXFSLS_TUNE_P25	( 1 << 4) /* +2.5% FS/LS Source Impedance Adj */
#define UPHY_TXFSLS_TUNE_P50	( 0 << 4) /* +5.0% FS/LS Source Impedance Adj */
#define UPHY_TXFSLS_TUNE_MASK	(15 << 4)
#define UPHY_TXPREEMP_TUNE_1X	( 1 << 3) /* 1=1x, 0=2x HS pre-emphasis */
#define UPHY_TXRESTUNE_N20		( 7 << 0) /* -20% Squelch Threshold Adj */
#define UPHY_SQRXTUNE_N15		( 6 << 0) /* -15% Squelch Threshold Adj */
#define UPHY_SQRXTUNE_N10		( 5 << 0) /* -10% Squelch Threshold Adj */
#define UPHY_SQRXTUNE_N5		( 4 << 0) /*  -5% Squelch Threshold Adj */
#define UPHY_SQRXTUNE_0			( 3 << 0) /*   0% Squelch Threshold Adj */
#define UPHY_SQRXTUNE_P5		( 2 << 0) /*  +5% Squelch Threshold Adj */
#define UPHY_SQRXTUNE_P10		( 1 << 0) /* +10% Squelch Threshold Adj */
#define UPHY_SQRXTUNE_P15		( 0 << 0) /* +15% Squelch Threshold Adj */

/*
 * Bits of GRF_UOCx_CON2 registers
 */
#define UPHY_ADP_PRBEN			(1 << 15) /* Ena. APD probe comparator */
#define UPHY_ADP_DISCHRG		(1 << 14) /* Ena. discharging Vbus during ADP */
#define UPHY_ADP_CHRG			(1 << 13) /* Ena. charging Vbus during ADP */
#define UPHY_TXRESTUNE_N4R		(3 << 11) /* Decrease source impedance by 4R */
#define UPHY_TXRESTUNE_N2R		(2 << 11) /* ... by 2R */
#define UPHY_TXRESTUNE_0		(1 << 11) /* ... by 0R */
#define UPHY_TXRESTUNE_N15R		(0 << 11) /* ... by 1.5R */
#define UPHY_TXRESTUNE_MASK		(3 << 11)
#define UPHY_RETMODE_DISABLE	(1 <<  8) /* 1: retention mode disable */
#define UPHY_FSEL_50M			(7 <<  5) /* 50.0MHz reference clock */
#define UPHY_FSEL_24M			(5 <<  5) /* 24.0MHz reference clock */
#define UPHY_FSEL_20M			(4 <<  5) /* 20.0MHz reference clock */
#define UPHY_FSEL_19M			(3 <<  5) /* 19.2MHz reference clock */
#define UPHY_FSEL_12M			(2 <<  5) /* 12.0MHz reference clock */
#define UPHY_FSEL_10M			(1 <<  5) /* 10.0MHz reference clock */
#define UPHY_FSEL_9M			(0 <<  5) /*  9.6MHz reference clock */
#define UPHY_FSEL_MASK			(7 <<  5)
#define UPHY_TXPREEMP_3X		(3 <<  3) /* 3X pre-emphasis current */
#define UPHY_TXPREEMP_2X		(2 <<  3) /* 2X pre-emphasis current */
#define UPHY_TXPREEMP_1X		(1 <<  3) /* 1X pre-emphasis current */
#define UPHY_TXPREEMP_DIS		(0 <<  3) /* HSTX pre-emphasis disable */
#define UPHY_SOFT_CON_SEL		(1 <<  2) /* 1 : software control usb phy enable */
#define UPHY_VBUS_DET_EXT		(1 <<  1) /* VBUS_DET pin used */
#define UPHY_VBUS_DET_INT		(0 <<  1) /* Internally detected */

/*
 * Bits of GRF_UOCx_CON3 registers
 */
/* Valid only for OTG capable interfaces */
#define OTG_BVALID_IRQ_PEND		(1 << 15) /* OTG0 bvalid interrupt pending */
#define OTG_BVALID_IRQ_EN		(1 << 14) /* OTG0 bvalid interrupt enable */
/* Valid only for all interfaces */
#define UTMI_TERMSELECT			(1 <<  5) /* 1:FS 0:HS termination enabled */
#define UTMI_XCVRSEL_LSFS		(3 <<  3) /* LS over FS transceiver */
#define UTMI_XCVRSEL_LS			(2 <<  3) /* select LS transceiver */
#define UTMI_XCVRSEL_FS			(1 <<  3) /* select FS transceiver */
#define UTMI_XCVRSEL_HS			(0 <<  3) /* select HS transceiver */
#define UTMI_XCVRSEL_MASK		(3 <<  3)
#define UTMI_OPMODE_NOSYNC		(3 <<  1) /* normal operation without SYNC or EOP */
#define UTMI_OPMODE_NOSTUFF		(2 <<  1) /* disable bit stuffing & NRZI encoding */
#define UTMI_OPMODE_NODRIVE		(1 <<  1) /* no-driving */
#define UTMI_OPMODE_NORMAL		(0 <<  1) /* normal operation */
#define UTMI_OPMODE_MASK		(3 <<  1)
#define UTMI_SUSPEND_EN			(1 <<  0) /* 1: suspend */

/* Abstr. functions to set masked values and bits */
#define HIWORD_UPDATE(val, mask)	(((mask) << 16) | (val))
#define HIWORD_BITSET(mask)		(((mask) << 16) | (mask))
#define HIWORD_BITCLR(mask) 		((mask) << 16)

struct phy_control {
	void (*phy_power)(struct phy_control *phy_ctrl, u32 id, bool on);
	void (*phy_wkup)(struct phy_control *phy_ctrl, u32 id, bool on);
};

static inline void phy_ctrl_power(struct phy_control *phy_ctrl, u32 id, bool on)
{
	phy_ctrl->phy_power(phy_ctrl, id, on);
}

static inline void phy_ctrl_wkup(struct phy_control *phy_ctrl, u32 id, bool on)
{
	phy_ctrl->phy_wkup(phy_ctrl, id, on);
}

struct phy_control *rk_get_phy_control(struct device *dev);

#endif
