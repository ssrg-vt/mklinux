
ccflags-y += -Iarch/x86/vmm -Iinclude/linux

CFLAGS_x86.o := -I.
CFLAGS_svm.o := -I.
CFLAGS_vmx.o := -I.

bhyve-y			+= vmm.o vmm_dev.o vmm_ipi.o vmm_lapic.o vmm_mem.o \
			   vmm_msr.o vmm_stat.o vmm_util.o x86.o vmm_support.o
bhyve-intel-y		+= $(addprefix intel/, ept.o vmcs.o vmx_msr.o vmx.o vtd.o)
bhyve-amd-y		+= $(addprefix amd/, amdv.o)

obj-$(CONFIG_BHYVE)	+= bhyve.o
obj-$(CONFIG_BHYVE_INTEL)	+= bhyve-intel.o
obj-$(CONFIG_BHYVE_AMD)	+= bhyve-amd.o
