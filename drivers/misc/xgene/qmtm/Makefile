#
# Makefile for APM X-GENE Queue Manager Traffic Manager driver.
#

xgene-qmtm-objs := xgene_qmtm.o \
		xgene_qmtm_main.o \
		xgene_qmtm_util.o \
		xgene_qmtm_msg.o \
		xgene_qmtm_storm.o

ifdef CONFIG_XGENE_QMTM_ERROR
xgene-qmtm-objs += xgene_qmtm_error.o
endif

obj-$(CONFIG_XGENE_QMTM) += xgene-qmtm.o
