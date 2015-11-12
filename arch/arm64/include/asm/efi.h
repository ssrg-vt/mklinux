#ifndef _ASM_ARM64_EFI_H
#define _ASM_ARM64_EFI_H

#include <asm/io.h>

#ifdef CONFIG_EFI
extern void efi_init(void);
#else
#define efi_init()
#endif

#endif /* _ASM_ARM64_EFI_H */
