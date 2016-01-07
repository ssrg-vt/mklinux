#
# Makefile for the ipmi drivers.
#

ipmi_si-y := ipmi_si_intf.o ipmi_kcs_sm.o ipmi_smic_sm.o ipmi_bt_sm.o \
	ipmi_raw_sm.o ipmi_xgene_kcs_sm.o\
    apm_ipmi_request.o apm_ipmi_sdr.o apm_ipmi_chassis_request.o \
	apm_ipmi_apps_request.o apm_ipmi_storage_request.o apm_ipmi_oem_request.o \
    apm_ipmi_sensor_event_request.o \
	apm_i2c_access.o apm_sensor_adm1032.o apm_sensor_fan.o apm_sensor_dimm.o \
	apm_sensor_storm.o apm_sensor_vrm.o

obj-$(CONFIG_IPMI_HANDLER) += ipmi_msghandler.o
obj-$(CONFIG_IPMI_DEVICE_INTERFACE) += ipmi_devintf.o
obj-$(CONFIG_IPMI_SI) += ipmi_si.o
obj-$(CONFIG_IPMI_WATCHDOG) += ipmi_watchdog.o
obj-$(CONFIG_IPMI_POWEROFF) += ipmi_poweroff.o

obj-m += power_sensor/
