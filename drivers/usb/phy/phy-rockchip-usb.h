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

/* General SOC Status Register 0 - USB HOST Flags */
#define uhost_adpprb 		(1 << 22) /* adpprb signal status */
#define uhost_adpsns 		(1 << 21) /* adpsns status */
#define uhost_utmiotg_iddig	(1 << 20) /* iddig signal status */
#define	uhost_utmi_linestate	(3 << 18) /* linstate signal status */
#define uhost_utmisrp_bvalid	(1 << 17) /* bvalid status signal */
#define uhost_utmiotg_vbusvalid (1 << 16) /* vbusvalid signal status */

/* General SOC Status Register 0 - USB OTG Flags */
#define GRF_SOC_STATUS0 (0x00ac)
#define otg_adpprb		(1 << 15) /* adpprb signal status */
#define otg_adpsns 		(1 << 14) /* adpsns status */
#define otg_utmiotg_iddig	(1 << 13) /* iddig signal status */
#define otg_utmi_linestate	(3 << 11) /* linestate signal status */
#define otg_utmisrp_bvalid	(1 << 10) /* bvalid status signal */
#define otg_utmiotg_vbusvalid	(1 <<  9) /* vbusvalid signal status */

// TODO: register setup and flags are identical for HOST and OTG
// But OTG has some more bits
// So HOST can be driven like OTG but use some other MASKs to avoid
// incorrect settings.
/*
 * Bits of GRF_UOCx_CON0 registers
 */
#define usbphy0_txbitstuff_enh	(1 << 15) /* Enable High-Byte TX Bit-Stuffing */
#define usbphy0_txbitstuff_en	(1 << 14) /* Enable Low-Byte TX Bit-Stuffing */
#define usbphy0_siddq		(1 << 13) /* IDDQ Test Enable */
#define usbphy0_port_reset	(1 << 12) /* Per-Port Reset */
#define usbphy0_refclk_sel_clk	(2 << 10) /* Use CLKCORE as reference */
#define usbphy0_refclk_sel_xop	(1 << 10) /* 1.8V clock via XO pin */
#define usbphy0_refclk_sel_xtal	(0 << 10) /* crystal supplied clock */
#define usbphy0_bypasssel	(1 <<  9) /* tx digital bypass mode enable */
#define usbphy0_bypassdmen	(1 <<  8) /* DM0 FS/LS driver enabled */
#define usbphy0_otg_tune_p9	(7 <<  5) /*  +9% VBUS valid adjust */
#define usbphy0_otg_tune_p6	(6 <<  5) /*  +6% VBUS valid adjust */
#define usbphy0_otg_tune_p3	(5 <<  5) /*  +3% VBUS valid adjust */
#define usbphy0_otg_tune_norm	(4 <<  5) /* normal VBUS valid adjust */
#define usbphy0_otg_tune_m3	(3 <<  5) /*  -3% VBUS valid adjust */
#define usbphy0_otg_tune_m6	(2 <<  5) /*  -6% VBUS valid adjust */
#define usbphy0_otg_tune_m9	(1 <<  5) /*  -9% VBUS valid adjust */
#define usbphy0_otg_tune_m12	(0 <<  5) /* -12% VBUS valid adjust */
#define usbphy0_otg_tune_mask	(7 <<  5)
#define usbphy0_otg_disable	(1 <<  4) /* 1: OTG block power down */
#define usbphy0_compdistune_p45 (7 <<  1) /* +4.5% Disconnect Threshold Adj */
#define usbphy0_compdistune_p30 (6 <<  1) /* +3.0% Disconnect Threshold Adj */
#define usbphy0_compdistune_p15 (5 <<  1) /* +1.5% Disconnect Threshold Adj */
#define usbphy0_compdistune_norm (4 <<  1) /* Normal Disconnect Threshold Adj */
#define usbphy0_compdistune_m15 (3 <<  1) /* -1.5% Disconnect Threshold Adj */
#define usbphy0_compdistune_m30 (2 <<  1) /* -3.0% Disconnect Threshold Adj */
#define usbphy0_compdistune_m45 (1 <<  1) /* -4.5% Disconnect Threshold Adj */
#define usbphy0_compdistune_m60 (0 <<  1) /* -6.0% Disconnect Threshold Adj */
#define usbphy0_compdistune_mask (7 <<  1)
#define usbphy0_common_on_n	(1 << 0) /* Power down in Suspend & Sleep */

/*
 * Bits of GRF_UOCx_CON1 registers
 */
#define usbphy_txrise_tune_m20	(3 << 14) /* -20% HSTX adjust rise/fall times */
#define usbphy_txrise_tune_m15	(2 << 14) /* -15% HSTX adjust rise/fall times */
#define usbphy_txrise_tune_0	(1 << 14) /* HSTX default rise/fall times */
#define usbphy_txrise_tune_p10	(0 << 14) /* +10% HSTX adjust rise/fall times */
#define usbphy_txrise_tune_mask	(0 << 14)
#define usbphy_txhsxv_tune_0	(3 << 12) /* defaul HSTX Crossover Point */
#define usbphy_txhsxv_tune_p15	(2 << 12) /* +15mV HSTX Crossover Adjustment */
#define usbphy_txhsxv_tune_m15	(1 << 12) /* -15mV HSTX Crossover Adjustment */
#define usbphy_txhsxv_tune_mask	(3 << 12)
#define usbphy_txvref_tune_p875 (15 << 8) /* +8.75% HS DC level adj */
#define usbphy_txvref_tune_p750 (14 << 8) /* +7.50% HS DC level adj */
#define usbphy_txvref_tune_p625 (13 << 8) /* +6.25% HS DC level adj */
#define usbphy_txvref_tune_p500 (12 << 8) /* +5.00% HS DC level adj */
#define usbphy_txvref_tune_p375 (11 << 8) /* +3.75% HS DC level adj */
#define usbphy_txvref_tune_p250 (10 << 8) /* +2.50% HS DC level adj */
#define usbphy_txvref_tune_p125 ( 9 << 8) /* +1.25% HS DC level adj */
#define usbphy_txvref_tune_0    ( 8 << 8) /*  0.00% HS DC level adj */
#define usbphy_txvref_tune_m125 ( 7 << 8) /* -1.25% HS DC level adj */
#define usbphy_txvref_tune_m250 ( 6 << 8) /* -2.50% HS DC level adj */
#define usbphy_txvref_tune_m375 ( 5 << 8) /* -3.75% HS DC level adj */
#define usbphy_txvref_tune_m500 ( 4 << 8) /* -5.00% HS DC level adj */
#define usbphy_txvref_tune_m625 ( 3 << 8) /* -6.25% HS DC level adj */
#define usbphy_txvref_tune_m750 ( 2 << 8) /* -7.50% HS DC level adj */
#define usbphy_txvref_tune_m875 ( 1 << 8) /* -8.75% HS DC level adj */
#define usbphy_txvref_tune_m100 ( 0 << 8) /* -10.0% HS DC level adj */
#define usbphy_txvref_tune_mask (15 << 8)
#define usbphy_txfsls_tune_m50	(15 << 4) /* -5.0% FS/LS Source Impedance Adj */
#define usbphy_txfsls_tune_m25	( 7 << 4) /* -2.5% FS/LS Source Impedance Adj */
#define usbphy_txfsls_tune_m0	( 3 << 4) /*    0% FS/LS Source Impedance Adj */
#define usbphy_txfsls_tune_p25	( 1 << 4) /* +2.5% FS/LS Source Impedance Adj */
#define usbphy_txfsls_tune_p50	( 0 << 4) /* +5.0% FS/LS Source Impedance Adj */
#define usbphy_txfsls_tune_mask	(15 << 4)
#define usbphy_txpreemppulse_tune_1x (1 << 3) /* 1=1x, 0=2x HS pre-emphasis */
#define usbphy_sqrxtune_m20 	( 7 << 0) /* -20% Squelch Threshold Adj */
#define usbphy_sqrxtune_m15 	( 6 << 0) /* -15% Squelch Threshold Adj */
#define usbphy_sqrxtune_m10 	( 5 << 0) /* -10% Squelch Threshold Adj */
#define usbphy_sqrxtune_m5 	( 4 << 0) /* -5% Squelch Threshold Adj */
#define usbphy_sqrxtune_0 	( 3 << 0) /*  0% Squelch Threshold Adj */
#define usbphy_sqrxtune_p5 	( 2 << 0) /* +5% Squelch Threshold Adj */
#define usbphy_sqrxtune_p10 	( 1 << 0) /* +10% Squelch Threshold Adj */
#define usbphy_sqrxtune_p15 	( 0 << 0) /* +15% Squelch Threshold Adj */

/*
 * Bits of GRF_UOCx_CON2 registers
 */
#define adpprbenb		(1 << 15) /* 1: APD proble comparator enabled */
#define adpdischrg		(1 << 14) /* 1: Enable discharging Vbus during ADP */
#define adpchrg			(1 << 13) /* 1: Enable charging Vbus during ADP */
#define txrestune_m4r		(3 << 11) /* source impedance dec by 4R */
#define txrestune_m2r		(2 << 11) /* source impedance dec by 2R */
#define txrestune_norm		(1 << 11) /* default */
#define txrestune_m15r		(0 << 11) /* source impedance dec by 1.5R */
#define txrestune_mask		(3 << 11)
#define retenable		(1 <<  8) /* 1: retention mode disable */
#define fsel_50M		(7 <<  5) /* 50.0MHz reference clock */
#define fsel_24M		(5 <<  5) /* 24.0MHz reference clock */
#define fsel_20M		(4 <<  5) /* 20.0MHz reference clock */
#define fsel_19M		(3 <<  5) /* 19.2MHz reference clock */
#define fsel_12M		(2 <<  5) /* 12.0MHz reference clock */
#define fsel_10M		(1 <<  5) /* 10.0MHz reference clock */
#define fsel_9M			(0 <<  5) /*  9.6MHz reference clock */
#define fsel_mask		(7 <<  5)
#define usbphy_txpreemp_3x	(3 << 3) /* 3X pre-emphasis current */
#define usbphy_txpreemp_2x	(2 << 3) /* 2X pre-emphasis current */
#define usbphy_txpreemp_1x	(1 << 3) /* 1X pre-emphasis current */
#define usbphy_txpreemp_dis	(0 << 3) /* HSTX pre-emphasis disable */
#define usbphy_soft_con_sel	(1 <<  2) /* 1 : software control usb phy enable */
#define usbphy_vbus_vld_extsel	(1 <<  1) /* VBUS_DET pin used */
#define usbphy_vbus_vld_intsel	(0 <<  1) /* Internally detected */

/*
 * Bits of GRF_UOCx_CON3 registers
 */
/* Valid only for OTG capable interfaces */
#define otg_bvalid_irq_pd	(0x01 << 15) /* otg0 bvalid interrupt pending */
#define otg_bvalid_irq_en	(0x01 << 14) /* otg0 bvalid interrupt enable */
/* Valid only for all interfaces */
#define utmi_termselect		(0x01 <<  5) /* 1:FS 0:HS termination enabled */
#define utmi_xcvrselect_lsfs	(0x03 <<  3) /* LS over FS transceiver */
#define utmi_xcvrselect_ls	(0x02 <<  3) /* select LS transceiver */
#define utmi_xcvrselect_fs	(0x01 <<  3) /* select FS transceiver */
#define utmi_xcvrselect_hs	(0x00 <<  3) /* select HS transceiver */
#define utmi_xcvrselect_mask	(0x03 <<  3)
#define utmi_opmode_nosync	(0x03 <<  1) /* normal operation without SYNC or EOP */
#define utmi_opmode_nostuff	(0x02 <<  1) /* disable bit stuffing & NRZI encoding */
#define utmi_opmode_nodrive	(0x01 <<  1) /* no-driving */
#define utmi_opmode_normal	(0x00 <<  1) /* normal operation */
#define utmi_opmode_mask	(0x03 <<  1)
#define utmi_suspend_n		(0x01 <<  0) /* 1: suspend */

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
