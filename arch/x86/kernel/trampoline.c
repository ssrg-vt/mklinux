#include <linux/io.h>
#include <linux/memblock.h>

#include <asm/trampoline.h>
#include <asm/cacheflush.h>
#include <asm/pgtable.h>

extern int mklinux_boot;

unsigned char *x86_trampoline_base;
unsigned char *x86_trampoline_bsp_base;

void __init setup_trampolines(void)
{
	phys_addr_t mem;
	size_t size = PAGE_ALIGN(x86_trampoline_end - x86_trampoline_start);

	/* Has to be in very low memory so we can execute real-mode AP code. */
	mem = memblock_find_in_range(0, 1<<20, size, PAGE_SIZE);
	if (mem == MEMBLOCK_ERROR)
		panic("Cannot allocate trampoline\n");

	x86_trampoline_base = __va(mem);
	memblock_x86_reserve_range(mem, mem + size, "TRAMPOLINE");

	printk(KERN_DEBUG "Base memory trampoline at [%p] %llx size %zu\n",
	       x86_trampoline_base, (unsigned long long)mem, size);

	/* POPCORN -- this is commented out because it breaks clustering.
	 * TODO -- need to fix! */
//	if (!mklinux_boot) {
		memcpy(x86_trampoline_base, x86_trampoline_start, size);
//	} else {
//		printk("Popcorn boot: SMP trampoline will NOT be copied\n");
//	}
}


void __init setup_trampolines_bsp(void)
{
	phys_addr_t mem;
	size_t size = PAGE_ALIGN(x86_trampoline_bsp_end - x86_trampoline_bsp_start);

	/* Has to be in very low memory so we can execute real-mode AP code. */
	mem = memblock_find_in_range(0, 1<<20, size, PAGE_SIZE);
	if (mem == MEMBLOCK_ERROR)
		panic("Cannot allocate trampoline\n");

	x86_trampoline_bsp_base = __va(mem);
	memblock_x86_reserve_range(mem, mem + size, "TRAMPOLINE_BSP");

	printk(KERN_DEBUG "Base memory trampoline BSP at [%p] %llx size %zu\n",
	       x86_trampoline_bsp_base, (unsigned long long)mem, size);

	if (!mklinux_boot) {
		memcpy(x86_trampoline_bsp_base, x86_trampoline_bsp_start, size);

	} else {
		printk("Popcorn boot: BSP trampoline will NOT be copied\n");
	}
}

/*
 * setup_trampolines() gets called very early, to guarantee the
 * availability of low memory.  This is before the proper kernel page
 * tables are set up, so we cannot set page permissions in that
 * function.  Thus, we use an arch_initcall instead.
 */
static int __init configure_trampolines(void)
{
	size_t size = PAGE_ALIGN(x86_trampoline_end - x86_trampoline_start);

	set_memory_x((unsigned long)x86_trampoline_base, size >> PAGE_SHIFT);
	return 0;
}

arch_initcall(configure_trampolines);

static int __init configure_trampolines_bsp(void)
{
	size_t size = PAGE_ALIGN(x86_trampoline_bsp_end - x86_trampoline_bsp_start);

	set_memory_x((unsigned long)x86_trampoline_bsp_base, size >> PAGE_SHIFT);
	return 0;
}

arch_initcall(configure_trampolines_bsp);

