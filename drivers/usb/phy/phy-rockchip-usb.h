#ifndef _RK_PHY_CONTROL_H_
#define _RK_PHY_CONTROL_H_

/* General SOC Status Register 0 - USB HOST Flags */
#define GRF_SOC_STATUS0 (0x00ac)
#define uhost_adpprb 		(1 << 22) /* adpprb signal status */
#define uhost_adpsns 		(1 << 21) /* adpsns status */
#define uhost_utmiotg_iddig	(1 << 20) /* iddig signal status */
#define	uhost_utmi_linestate	(3 << 18) /* linstate signal status */
#define uhost_utmisrp_bvalid	(1 << 17) /* bvalid status signal */
#define uhost_utmiotg_vbusvalid (1 << 16) /* vbusvalid signal status */

/* General SOC Status Register 0 - USB OTG Flags */
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
#define GRF_UOC0_CON0	(0x00)	/* 0x010c W 0x0000c889 otg control register */
#define GRF_UOC0_CON1	(0x04)	/* 0x0110 W 0x00007333 otg control register */

#define GRF_UOC0_CON2	(0x08)	/* 0x0114 W 0x00000da8 otg control register */

#define GRF_UOC0_CON3	(0x0c)	/* 0x0118 W 0x00000001 otg control register */

/* 0x011c W 0x0000c889 usb host control register */
#define GRF_UOC1_CON0	(0x00)
/* 0x0120 W 0x00007333 usb host control register */
#define GRF_UOC1_CON1	(0x04)

/* 0x0124 W 0x00000da8 otg control register */
#define GRF_UOC1_CON2	(0x08)
#define PHY_VBUSVLD_EXT (0x00 << 0) /* enable / detect D+ pullup resistor */
#define PHY_VBUSVLD_MASK (0x01 << 0)
#define PHY_VBUSVLD_EXTSEL (0x01 << 1) /* 1: Use VBUS_DET input */
#define PHY_VBUSVLD_INTSEL (0x00 << 1) /* 0: Use internal comparator

#define PHY_SFTCON_DIS	(0x00 << 2) /* disable software control of PHY */
#define PHY_SFTCON_ENA	(0x01 << 2) /* enable software control of PHY */
#define PHY_SFTCON_MASK	(0x01 << 2)

/* 0x0128 W 0x00000001 usb host control register */
#define GRF_UOC1_CON3	(0x0c)
#define UTMI_SUSP_ON	(0x01 <<  0) /* suspend */
#define UTMI_SUSP_OFF	(0x00 <<  0) /* normal operation */
#define UTMI_SUSP_MASK	(0x01 <<  0)
#define UTMI_OPM_NORMAL	(0x00 <<  1) /* normal operation */
#define UTMI_OPM_NODRV	(0x01 <<  1) /* no-driving */
#define UTMI_OPM_NOSTUF	(0x02 <<  1) /* disable bit stuffing & NRZI encoding */
#define UTMI_OPM_NOSYNC	(0x03 <<  1) /* normal operation without SYNC or EOP */
#define UTMI_OPM_MASK	(0x03 <<  1)
#define UTMI_XCVR_HS	(0x00 <<  3) /* select HS transceiver */
#define UTMI_XCVR_HS	(0x01 <<  3) /* select FS transceiver */
#define UTMI_XCVR_HS	(0x02 <<  3) /* select LS transceiver */
#define UTMI_XCVR_LOFS	(0x03 <<  3) /* LS over FS transceiver */
#define UTMI_XCVR_MASK	(0x03 <<  3)
#define UTMI_TERM_HS	(0x00 <<  5) /* HS termination enabled */
#define UTMI_TERM_FS	(0x01 <<  5) /* FS termination enabled */
#define UTMI_TERM_MASK	(0x01 <<  5)
/* Valid only for OTG capable interfaces */
#define UTMI_BVIRQ_DIS	(0x00 << 14) /* otg0 bvalid interrupt disable */
#define UTMI_BVIRQ_ENA	(0x01 << 14) /* otg0 bvalid interrupt enable */
#define UTMI_BVIRQ_MASK	(0x01 << 14)
#define UTMI_BVIRQ_PEND (0x01 << 15) /* otg0 bvalid interrupt pending */
#define UTMI_BVIRQ_CLR	(0x01 << 15) /* clear otg0 bvalid irq bit */

#define GRF_UOC2_CON0	(0x00)	/* 0x012c W 0x000030eb hsic phy control register */
#define GRF_UOC2_CON1	(0x04)	/* 0x0130 W 0x00000003 hsic phy control register */
#define GRF_UOC3_CON0	(0x0c)	/* 0x0138 W 0x00000080 hsic controller control register */
#define GRF_UOC3_CON1	(0x10)	/* 0x013c W 0x00000820 hsic controller control register */
#define GRF_HSIC_STAT	(0x14)	/* 0x0140 W 0x00000000 hsic status register */

#define HIWORD_UPDATE(val, mask) \
		((val) | (mask) << 16))

#define
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

struct phy_control *am335x_get_phy_control(struct device *dev);

#endif
