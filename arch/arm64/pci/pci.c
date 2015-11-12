#include <linux/acpi.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>

/**
 * raw_pci_read - Platform-specific PCI config space access.
 *
 * Default empty implementation.  Replace with an architecture-specific setup
 * routine, if necessary.
 */
int __weak raw_pci_read(unsigned int domain, unsigned int bus,
			unsigned int devfn, int reg, int len, u32 *val)
{
	return -EINVAL;
}

int __weak raw_pci_write(unsigned int domain, unsigned int bus,
			unsigned int devfn, int reg, int len, u32 val)
{
	return -EINVAL;
}

#ifdef CONFIG_ACPI
/* Root bridge scanning */
struct  pci_bus __weak *pci_acpi_scan_root(struct acpi_pci_root *root)
{
	return NULL;
}
#endif

void __init pci_acpi_crs_quirks(void)
{
	return;
}
