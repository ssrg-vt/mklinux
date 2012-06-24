/*
 *
 *	Trampoline.S	Derived from Setup.S by Linus Torvalds
 *
 *	4 Jan 1997 Michael Chastain: changed to gnu as.
 *	15 Sept 2005 Eric Biederman: 64bit PIC support
 *
 *	Entry: CS:IP point to the start of our code, we are 
 *	in real mode with no stack, but the rest of the 
 *	trampoline page to make our stack and everything else
 *	is a mystery.
 *
 *	On entry to trampoline_data, the processor is in real mode
 *	with 16-bit addressing and 16-bit data.  CS has some value
 *	and IP is zero.  Thus, data addresses need to be absolute
 *	(no relocation) and are taken with regard to r_base.
 *
 *	With the addition of trampoline_level4_pgt this code can
 *	now enter a 64bit kernel that lives at arbitrary 64bit
 *	physical addresses.
 *
 *	If you work on this file, check the object module with objdump
 *	--full-contents --reloc to make sure there are no relocation
 *	entries.
 */

#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/pgtable_types.h>
#include <asm/page_types.h>
#include <asm/msr.h>
#include <asm/segment.h>
#include <asm/processor-flags.h>

	.section ".x86_trampoline_bsp","a"
	.balign PAGE_SIZE
	.code16

ENTRY(trampoline_data_bsp)
bsp_base = .
	cli			# We should be safe anyway
	wbinvd
	mov	%cs, %ax	# Code and data in the same place
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %ss


	movl	$0xA5A5A5A5, trampoline_status_bsp - bsp_base
				# write marker for master knows we're running

					# Setup stack
	movw	$(trampoline_stack_bsp_end - bsp_base), %sp

	call	verify_cpu		# Verify the cpu supports long mode
	testl   %eax, %eax		# Check for return code
	jnz	no_longmode_bsp

	mov	%cs, %ax
	movzx	%ax, %esi		# Find the 32bit trampoline location
	shll	$4, %esi

					# Fixup the absolute vectors
	leal	(startup_32_bsp - bsp_base)(%esi), %eax
	movl	%eax, startup_32_vector_bsp - bsp_base
	leal	(startup_64_bsp - bsp_base)(%esi), %eax
	movl	%eax, startup_64_vector_bsp - bsp_base
	leal	(tgdt_bsp - bsp_base)(%esi), %eax
	movl	%eax, (tgdt_bsp + 2 - bsp_base)

	/*
	 * GDT tables in non default location kernel can be beyond 16MB and
	 * lgdt will not be able to load the address as in real mode default
	 * operand size is 16bit. Use lgdtl instead to force operand size
	 * to 32 bit.
	 */

	lidtl	tidt_bsp - bsp_base	# load idt with 0, 0
	lgdtl	tgdt_bsp - bsp_base	# load gdt with whatever is appropriate

	mov	$X86_CR0_PE, %ax	# protected mode (PE) bit
	lmsw	%ax			# into protected mode

	# flush prefetch and jump to startup_32
	ljmpl	*(startup_32_vector_bsp - bsp_base)

	.code32
	.balign 4
startup_32_bsp:

	/* MKLINUX -- at this point, we're in 32-bit mode */

	/* MKLINUX -- this is code from arch/x86/boot/compressed/head_64.S
	 * It's necessary here in order to set up the identity pagetables
	 * prior to entering the kernel. */


	/* MKLINUX -- at this point, the */
/*
 * Calculate the delta between where we were compiled to run
 * at and where we were actually loaded at.  This can only be done
 * with a short local call on x86.  Nothing  else will tell us what
 * address we are running at.  The reserved chunk of the real-mode
 * data at 0x1e4 (defined as a scratch field) are used as the stack
 * for this calculation. Only 4 bytes are needed.
 */

/*
        leal    (0x1e4)(%esi), %esp
        call    1f
1:      popl    %ebp
        subl    $1b, %ebp
*/

	/* MKLINUX -- load where we were loaded into %ebx */

		

 /*
  * Build early 4G boot pagetable
  */
        /* Initialize Page tables to 0 */
	leal    pgtable_bsp - bsp_base + 0(%ebx), %edi        

	xorl    %eax, %eax
        movl    $((4096*6)/4), %ecx
        rep     stosl

        /* Build Level 4 */
        leal    (pgtable_bsp - bsp_base)(%esi), %edi
        leal    0x1007 (%edi), %eax
        movl    %eax, 0(%edi)

        /* Build Level 3 */
        leal    (pgtable_bsp - bsp_base + 0x1000)(%esi), %edi
        leal    0x1007(%edi), %eax
        movl    $4, %ecx
1:      movl    %eax, 0x00(%edi)
        addl    $0x00001000, %eax
        addl    $8, %edi
        decl    %ecx
        jnz     1b

        /* Build Level 2 */
        leal    (pgtable_bsp - bsp_base + 0x2000)(%esi), %edi
        movl    $0x00000183, %eax
        movl    $2048, %ecx
1:      movl    %eax, 0(%edi)
        addl    $0x00200000, %eax
        addl    $8, %edi
        decl    %ecx
        jnz     1b

        /* Enable the boot page tables */
        leal    (pgtable_bsp - bsp_base)(%esi), %eax
        movl    %eax, %cr3

        /* Enable Long mode in EFER (Extended Feature Enable Register) */
        movl    $MSR_EFER, %ecx
        rdmsr
        btsl    $_EFER_LME, %eax
        wrmsr


	/* Enter paged protected Mode, activating Long Mode */
        movl    $(X86_CR0_PG | X86_CR0_PE), %eax /* Enable Paging and Protected mode */
        movl    %eax, %cr0


/*

	movl	$__KERNEL_DS, %eax	# Initialize the %ds segment register
	movl	%eax, %ds

	movl	$X86_CR4_PAE, %eax
	movl	%eax, %cr4		# Enable PAE mode

					# Setup trampoline 4 level pagetables
	leal	(trampoline_level4_pgt - r_base)(%esi), %eax
	movl	%eax, %cr3

	movl	$MSR_EFER, %ecx
	movl	$(1 << _EFER_LME), %eax	# Enable Long Mode
	xorl	%edx, %edx
	wrmsr

	# Enable paging and in turn activate Long Mode
	# Enable protected mode
	movl	$(X86_CR0_PG | X86_CR0_PE), %eax
	movl	%eax, %cr0
*/

	/*
	 * At this point we're in long mode but in 32bit compatibility mode
	 * with EFER.LME = 1, CS.L = 0, CS.D = 1 (and in turn
	 * EFER.LMA = 1). Now we want to jump in 64bit mode, to do that we use
	 * the new gdt/idt that has __KERNEL_CS with CS.L = 1.
	 */
	ljmp	*(startup_64_vector_bsp - bsp_base)(%esi)

	.code64
	.balign 4
startup_64_bsp:
	# Now jump into the kernel using virtual addresses
	movq	$secondary_startup_64, %rax
	jmp	*%rax

	.code16
no_longmode_bsp:
	hlt
	jmp no_longmode_bsp
#include "verify_cpu.S"

	.balign 4
	# Careful these need to be in the same 64K segment as the above;
tidt_bsp:
	.word	0			# idt limit = 0
	.word	0, 0			# idt base = 0L

	# Duplicate the global descriptor table
	# so the kernel can live anywhere
	.balign 4
tgdt_bsp:
	.short	tgdt_bsp_end - tgdt_bsp		# gdt limit
	.long	tgdt_bsp - bsp_base
	.short 0
	.quad	0x00cf9b000000ffff	# __KERNEL32_CS
	.quad	0x00af9b000000ffff	# __KERNEL_CS
	.quad	0x00cf93000000ffff	# __KERNEL_DS
tgdt_bsp_end:

	.balign 4
startup_32_vector_bsp:
	.long	startup_32_bsp - bsp_base
	.word	__KERNEL32_CS, 0

	.balign 4
startup_64_vector_bsp:
	.long	startup_64_bsp - bsp_base
	.word	__KERNEL_CS, 0

	.balign 4
ENTRY(trampoline_status_bsp)
	.long	0

	.balign 4
ENTRY(trampoline_location)
	.quad   0

#ENTRY(trampoline_level4_pgt)
#        .quad   level3_ident_pgt - __START_KERNEL_map + _KERNPG_TABLE
#        .fill   510,8,0
#        .quad   level3_kernel_pgt - __START_KERNEL_map + _KERNPG_TABLE

trampoline_stack_bsp:
	.fill 512,8,0
trampoline_stack_bsp_end:


ENTRY(trampoline_bsp_end)

/*
 * Space for page tables (not in .bss so not zeroed)
 */

#        .section ".pgtable_bsp","a",@nobits
        .balign 4096
pgtable_bsp:
        .fill 6*4096, 1, 0


