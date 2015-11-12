#ifndef _ASM_ARM_EFI_H
#define _ASM_ARM_EFI_H

#include <asm/mach/map.h>

extern int efi_memblock_arm_reserve_range(void);

typedef efi_status_t efi_phys_call_t(u32 memory_map_size,
				     u32 descriptor_size,
				     u32 descriptor_version,
				     efi_memory_desc_t *dsc,
				     efi_set_virtual_address_map_t *f);

extern efi_status_t efi_phys_call(u32, u32, u32, efi_memory_desc_t *,
				  efi_set_virtual_address_map_t *);

#define efi_remap(cookie, size) __arm_ioremap((cookie), (size), MT_MEMORY)
#define efi_ioremap(cookie, size) __arm_ioremap((cookie), (size), MT_DEVICE)
#define efi_unmap(cookie) __arm_iounmap((cookie))
#define efi_iounmap(cookie) __arm_iounmap((cookie))

#endif /* _ASM_ARM_EFI_H */
