#
# Makefile for the linux kernel.
#

CPPFLAGS_vmlinux.lds	:= -DTEXT_OFFSET=$(TEXT_OFFSET)
AFLAGS_head.o		:= -DTEXT_OFFSET=$(TEXT_OFFSET)
CFLAGS_efi-stub.o	:= -DTEXT_OFFSET=$(TEXT_OFFSET) \
			   -I$(src)/../../../scripts/dtc/libfdt

# Object file lists.
arm64-obj-y		:= cputable.o debug-monitors.o entry.o irq.o fpsimd.o	\
			   entry-fpsimd.o process.o ptrace.o setup.o signal.o	\
			   sys.o stacktrace.o time.o traps.o io.o vdso.o	\
			   hyp-stub.o psci.o

arm64-obj-$(CONFIG_AARCH32_EL0)	+= sys32.o kuser32.o signal32.o 	\
					   sys_compat.o
arm64-obj-$(CONFIG_ARM64_ILP32)		+= sys_ilp32.o
arm64-obj-$(CONFIG_MODULES)		+= arm64ksyms.o module.o
arm64-obj-$(CONFIG_SMP)			+= smp.o smp_spin_table.o cpu_ops.o
arm64-obj-$(CONFIG_HW_PERF_EVENTS)	+= perf_event.o
arm64-obj-$(CONFIG_HAVE_HW_BREAKPOINT)+= hw_breakpoint.o
arm64-obj-$(CONFIG_EARLY_PRINTK)	+= early_printk.o
arm64-obj-$(CONFIG_ARM64_CPU_TOPOLOGY)	+= topology.o
arm64-obj-$(CONFIG_EFI_STUB)		+= efi-stub.o efi-entry.o
arm64-obj-$(CONFIG_EFI)			+= efi.o
arm64-obj-$(CONFIG_ARM64_CPU_SUSPEND)	+= sleep.o suspend.o

obj-$(CONFIG_PCI)			+= pci.o
obj-$(CONFIG_PCI_MSI)			+= msi_bitmap.o
obj-y					+= $(arm64-obj-y) vdso/
obj-$(CONFIG_ARM64_ILP32)		+= vdso-ilp32/
obj-m					+= $(arm64-obj-m)
head-y					:= head.o
extra-y					:= $(head-y) vmlinux.lds

obj-$(CONFIG_POPCORN)			+= popcorn_cpuinfo_arch.o process_server.o

# vDSO - this must be built first to generate the symbol offsets
$(call objectify,$(arm64-obj-y)): $(obj)/vdso/vdso-offsets.h
$(obj)/vdso/vdso-offsets.h: $(obj)/vdso

# vDSO - this must be built first to generate the symbol offsets
$(call objectify,$(arm64-obj-y)): $(obj)/vdso-ilp32/vdso-ilp32-offsets.h
$(obj)/vdso-ilp32/vdso-ilp32-offsets.h: $(obj)/vdso-ilp32
