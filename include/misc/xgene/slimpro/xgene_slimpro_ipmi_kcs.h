#ifndef __XGENE_SLIMPRO_IPMI_KCS_H__
#define __XGENE_SLIMPRO_IPMI_KCS_H__

void slimpro_xgene_kcs_wr(u8 reg_offset, u8 *data, u8 len);
int slimpro_xgene_kcs_rd(u8 reg_offset, u8 *data, const int len);

#endif

