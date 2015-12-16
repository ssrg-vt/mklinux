#
# Makefile for APM X-GENE Ethernet driver.
#

xgene-enet-objs := \
		apm_enet_common.o	\
		apm_enet_mac.o		\
		apm_enet_access.o	\
		apm_enet_misc.o		\
		apm_enet_tools.o	\
		apm_xgenet_mac.o	

ifdef CONFIG_XGENE_CLE
xgene-enet-objs += apm_cle_cfg.o
endif

ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
xgene-enet-objs += apm_enet_ifo.o
endif

obj-$(CONFIG_XGENE_NET) += xgene-enet.o
