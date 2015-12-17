#
# Makefile for APM X-GENE Potenza/MSLIM Communication Interface (MCI).
#

mci-core-objs := xgene_mci_core.o \
			xgene_mci_sysfs.o

obj-$(CONFIG_XGENE_MCI) += mci-core.o
obj-$(CONFIG_ARCH_XGENE) += xgene_mci_potenza.o
obj-$(CONFIG_ARCH_MSLIM) += xgene_mci_mslim.o
obj-$(CONFIG_XGENE_MCI_TEST) += xgene_mci_test.o
