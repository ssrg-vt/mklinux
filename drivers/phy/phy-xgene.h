#ifndef __PHY_XGENE_INTERNAL__
#define __PHY_XGENE_INTERNAL__
#include <linux/acpi.h>
#include <linux/efi.h>

/* PHY with Ref CMU */
#define XGENE_PHY_DTS			"apm,xgene-phy"
#define XGENE_PHY_ACPI			0

/* PHY with Ref CMU located outside (external) of the PHY */
#define XGENE_PHY_EXT_DTS		"apm,xgene-phy-ext"
#define XGENE_PHY_EXT_ACPI		1

#define SERDES_PIPE_INDIRECT_OFFSET	0x10000
#define SERDES_PLL2_INDIRECT_OFFSET	0x30000
#define SERDES_INDIRECT2_OFFSET		0x30400
#define SERDES_LANE_X4_STRIDE		0x30000

#define SDS0_CMU_STATUS0		0x2c
#define SDS1_CMU_STATUS0		0x64

enum param_type {
	POST_LINK_UP_SSD = 0, /* ssd drives detected after link up */
	POST_LINK_UP_HDD = 1, /* HDD drives detected after link up */
	PRE_HARDRESET = 2,    /* Before COMINIT sequence */
	POST_HARDRESET = 3,   /* After COMINIT sequence */
	POST_LINK_UP_PHY_RESET = 4, /* PHY reset after link up */
};

enum disk_type {
	DISK_SSD = 0,
	DISK_HDD = 1
};

static int xgene_ahci_is_preB0(void)
{
	#define MIDR_EL1_REV_MASK		0x0000000f 
	#define REVIDR_EL1_MINOR_REV_MASK	0x00000007 

#ifndef CONFIG_ARCH_MSLIM
	u32 val;

	asm volatile("mrs %0, midr_el1" : "=r" (val));
        val &= MIDR_EL1_REV_MASK;
	return val == 0 ? 1 : 0;
#else
	return 0; /* Assume B0 */
#endif
}
#endif /*__PHY_XGENE_INTERNAL__*/

