/*
 * linux/arch/arm/boot/compressed/efi-stub.c
 *
 * Copyright (C) 2013 Linaro Ltd;  <roy.franz@linaro.org>
 *
 * This file implements the EFI boot stub for the ARM kernel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/efi.h>
#include <libfdt.h>
#include "efi-stub.h"

/* EFI function call wrappers.  These are not required for
 * ARM, but wrappers are required for X86 to convert between
 * ABIs.  These wrappers are provided to allow code sharing
 * between X86 and ARM.  Since these wrappers directly invoke the
 * EFI function pointer, the function pointer type must be properly
 * defined, which is not the case for X86  One advantage of this is
 * it allows for type checking of arguments, which is not
 * possible with the X86 wrappers.
 */
#define efi_call_phys0(f)			f()
#define efi_call_phys1(f, a1)			f(a1)
#define efi_call_phys2(f, a1, a2)		f(a1, a2)
#define efi_call_phys3(f, a1, a2, a3)		f(a1, a2, a3)
#define efi_call_phys4(f, a1, a2, a3, a4)	f(a1, a2, a3, a4)
#define efi_call_phys5(f, a1, a2, a3, a4, a5)	f(a1, a2, a3, a4, a5)

/* The maximum uncompressed kernel size is 32 MBytes, so we will reserve
 * that for the decompressed kernel.  We have no easy way to tell what
 * the actuall size of code + data the uncompressed kernel will use.
 */
#define MAX_UNCOMP_KERNEL_SIZE	0x02000000

/* The kernel zImage should be located between 32 Mbytes
 * and 128 MBytes from the base of DRAM.  The min
 * address leaves space for a maximal size uncompressed image,
 * and the max address is due to how the zImage decompressor
 * picks a destination address.
 */
#define ZIMAGE_OFFSET_LIMIT	0x08000000
#define MIN_ZIMAGE_OFFSET	MAX_UNCOMP_KERNEL_SIZE

#define PRINTK_PREFIX		"EFIstub: "

struct fdt_region {
	u64 base;
	u64 size;
};


/* Include shared EFI stub code */
#include "../../../../drivers/firmware/efi/efi-stub-helper.c"

static int relocate_kernel(efi_system_table_t *sys_table,
			   unsigned long *zimage_addr,
			   unsigned long zimage_size,
			   unsigned long min_addr, unsigned long max_addr)
{
	/* Get current address of kernel. */
	unsigned long cur_zimage_addr = *zimage_addr;
	unsigned long new_addr = 0;

	efi_status_t status;

	if (!zimage_addr || !zimage_size)
		return EFI_INVALID_PARAMETER;

	if (cur_zimage_addr > min_addr
	    && (cur_zimage_addr + zimage_size) < max_addr) {
		/* We don't need to do anything, as kernel is at an
		 * acceptable address already.
		 */
		return EFI_SUCCESS;
	}
	/*
	 * The EFI firmware loader could have placed the kernel image
	 * anywhere in memory, but the kernel has restrictions on the
	 * min and max physical address it can run at.
	 */
	status = efi_low_alloc(sys_table, zimage_size, 0,
			   &new_addr, min_addr);
	if (status != EFI_SUCCESS) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: Failed to allocate usable memory for kernel.\n");
		return status;
	}

	if (new_addr > (max_addr - zimage_size)) {
		efi_free(sys_table, zimage_size, new_addr);
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: Failed to allocate usable memory for kernel.\n");
		return EFI_INVALID_PARAMETER;
	}

	/* We know source/dest won't overlap since both memory ranges
	 * have been allocated by UEFI, so we can safely use memcpy.
	 */
	memcpy((void *)new_addr, (void *)(unsigned long)cur_zimage_addr,
	       zimage_size);

	/* Return the load address  */
	*zimage_addr = new_addr;

	return status;
}


/* Convert the unicode UEFI command line to ASCII to pass to kernel.
 * Size of memory allocated return in *cmd_line_len.
 * Returns NULL on error.
 */
static char *convert_cmdline_to_ascii(efi_system_table_t *sys_table,
				      efi_loaded_image_t *image,
				      unsigned long *cmd_line_len,
				      u32 max_addr)
{
	u16 *s2;
	u8 *s1 = NULL;
	unsigned long cmdline_addr = 0;
	int load_options_size = image->load_options_size / 2; /* ASCII */
	void *options = (u16 *)image->load_options;
	int options_size = 0;
	int status;
	int i;
	u16 zero = 0;

	if (options) {
		s2 = options;
		while (*s2 && *s2 != '\n' && options_size < load_options_size) {
			s2++;
			options_size++;
		}
	}

	if (options_size == 0) {
		/* No command line options, so return empty string*/
		options_size = 1;
		options = &zero;
	}

	options_size++;  /* NUL termination */

	status = efi_high_alloc(sys_table, options_size, 0,
			    &cmdline_addr, max_addr);
	if (status != EFI_SUCCESS)
		return NULL;

	s1 = (u8 *)(unsigned long)cmdline_addr;
	s2 = (u16 *)options;

	for (i = 0; i < options_size - 1; i++)
		*s1++ = *s2++;

	*s1 = '\0';

	*cmd_line_len = options_size;
	return (char *)(unsigned long)cmdline_addr;
}


static u32 update_fdt(efi_system_table_t *sys_table, void *orig_fdt, void *fdt,
		      int new_fdt_size, char *cmdline_ptr, u64 initrd_addr,
		      u64 initrd_size, efi_memory_desc_t *memory_map,
		      int map_size, int desc_size)
{
	int node;
	int status;
	unsigned long fdt_val;

	status = fdt_open_into(orig_fdt, fdt, new_fdt_size);
	if (status != 0)
		goto fdt_set_fail;

	node = fdt_subnode_offset(fdt, 0, "chosen");
	if (node < 0) {
		node = fdt_add_subnode(fdt, 0, "chosen");
		if (node < 0) {
			status = node; /* node is error code when negative */
			goto fdt_set_fail;
		}
	}

	if ((cmdline_ptr != NULL) && (strlen(cmdline_ptr) > 0)) {
		status = fdt_setprop(fdt, node, "bootargs", cmdline_ptr,
				     strlen(cmdline_ptr) + 1);
		if (status)
			goto fdt_set_fail;
	}

	/* Set intird address/end in device tree, if present */
	if (initrd_size != 0) {
		u64 initrd_image_end;
		u64 initrd_image_start = cpu_to_fdt64(initrd_addr);
		status = fdt_setprop(fdt, node, "linux,initrd-start",
				     &initrd_image_start, sizeof(u64));
		if (status)
			goto fdt_set_fail;
		initrd_image_end = cpu_to_fdt64(initrd_addr + initrd_size);
		status = fdt_setprop(fdt, node, "linux,initrd-end",
				     &initrd_image_end, sizeof(u64));
		if (status)
			goto fdt_set_fail;
	}

	/* Add FDT entries for EFI runtime services in chosen node. */
	node = fdt_subnode_offset(fdt, 0, "chosen");
	fdt_val = cpu_to_fdt32((unsigned long)sys_table);
	status = fdt_setprop(fdt, node, "efi-system-table",
			     &fdt_val, sizeof(fdt_val));
	if (status)
		goto fdt_set_fail;

	fdt_val = cpu_to_fdt32(desc_size);
	status = fdt_setprop(fdt, node, "efi-mmap-desc-size",
			     &fdt_val, sizeof(fdt_val));
	if (status)
		goto fdt_set_fail;

	fdt_val = cpu_to_fdt32(map_size);
	status = fdt_setprop(fdt, node, "efi-runtime-mmap-size",
			     &fdt_val, sizeof(fdt_val));
	if (status)
		goto fdt_set_fail;

	fdt_val = cpu_to_fdt32((unsigned long)memory_map);
	status = fdt_setprop(fdt, node, "efi-runtime-mmap",
			     &fdt_val, sizeof(fdt_val));
	if (status)
		goto fdt_set_fail;

	return EFI_SUCCESS;

fdt_set_fail:
	if (status == -FDT_ERR_NOSPACE)
		return EFI_BUFFER_TOO_SMALL;

	return EFI_LOAD_ERROR;
}



int efi_entry(void *handle, efi_system_table_t *sys_table,
	      unsigned long *zimage_addr)
{
	efi_loaded_image_t *image;
	int status;
	unsigned long nr_pages;
	const struct fdt_region *region;

	void *fdt;
	int err;
	int node;
	unsigned long zimage_size = 0;
	unsigned long dram_base;
	/* addr/point and size pairs for memory management*/
	u64 initrd_addr;
	u64 initrd_size = 0;
	u64 fdt_addr;
	u64 fdt_size = 0;
	u64 kernel_reserve_addr;
	u64 kernel_reserve_size = 0;
	char *cmdline_ptr;
	unsigned long cmdline_size = 0;

	unsigned long map_size, desc_size;
	unsigned long mmap_key;
	efi_memory_desc_t *memory_map;

	unsigned long new_fdt_size;
	unsigned long new_fdt_addr;

	efi_guid_t proto = LOADED_IMAGE_PROTOCOL_GUID;

	/* Check if we were booted by the EFI firmware */
	if (sys_table->hdr.signature != EFI_SYSTEM_TABLE_SIGNATURE)
		goto fail;

	efi_printk(sys_table, PRINTK_PREFIX"Booting Linux using EFI stub.\n");


	/* get the command line from EFI, using the LOADED_IMAGE protocol */
	status = efi_call_phys3(sys_table->boottime->handle_protocol,
				handle, &proto, (void *)&image);
	if (status != EFI_SUCCESS) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: Failed to get handle for LOADED_IMAGE_PROTOCOL\n");
		goto fail;
	}

	/* We are going to copy this into device tree, so we don't care where in
	 * memory it is.
	 */
	cmdline_ptr = convert_cmdline_to_ascii(sys_table, image,
					       &cmdline_size, 0xFFFFFFFF);
	if (!cmdline_ptr) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: converting command line to ascii failed.\n");
		goto fail;
	}

	/* We first load the device tree, as we need to get the base address of
	 * DRAM from the device tree.  The zImage, device tree, and initrd
	 * have address restrictions that are relative to the base of DRAM.
	 */
	status = handle_cmdline_files(sys_table, image, cmdline_ptr, "dtb=",
				      0xffffffff, &fdt_addr, &fdt_size);
	if (status != EFI_SUCCESS) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: Unable to load device tree blob.\n");
		goto fail_free_cmdline;
	}

	err = fdt_check_header((void *)(unsigned long)fdt_addr);
	if (err != 0) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: device tree header not valid\n");
		goto fail_free_fdt;
	}
	if (fdt_totalsize((void *)(unsigned long)fdt_addr) > fdt_size) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: Incomplete device tree.\n");
		goto fail_free_fdt;

	}


	/* Look up the base of DRAM from the device tree.*/
	fdt = (void *)(u32)fdt_addr;
	node = fdt_subnode_offset(fdt, 0, "memory");
	region = fdt_getprop(fdt, node, "reg", NULL);
	if (region) {
		dram_base = fdt64_to_cpu(region->base);
	} else {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: no 'memory' node in device tree.\n");
		goto fail_free_fdt;
	}

	/* Reserve memory for the uncompressed kernel image. */
	kernel_reserve_addr = dram_base;
	kernel_reserve_size = MAX_UNCOMP_KERNEL_SIZE;
	nr_pages = round_up(kernel_reserve_size, EFI_PAGE_SIZE) / EFI_PAGE_SIZE;
	status = efi_call_phys4(sys_table->boottime->allocate_pages,
				EFI_ALLOCATE_ADDRESS, EFI_LOADER_DATA,
				nr_pages, &kernel_reserve_addr);
	if (status != EFI_SUCCESS) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: unable to allocate memory for uncompressed kernel.\n");
		goto fail_free_fdt;
	}

	/* Relocate the zImage, if required. */
	zimage_size = image->image_size;
	status = relocate_kernel(sys_table, zimage_addr, zimage_size,
				 dram_base + MIN_ZIMAGE_OFFSET,
				 dram_base + ZIMAGE_OFFSET_LIMIT);
	if (status != EFI_SUCCESS) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: Failed to relocate kernel\n");
		goto fail_free_kernel_reserve;
	}

	status = handle_cmdline_files(sys_table, image, cmdline_ptr, "initrd=",
				      dram_base + ZIMAGE_OFFSET_LIMIT,
				      &initrd_addr, &initrd_size);
	if (status != EFI_SUCCESS) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: Unable to load initrd\n");
		goto fail_free_zimage;
	}

	/* Estimate size of new FDT, and allocate memory for it. We
	 * will allocate a bigger buffer if this ends up being too
	 * small, so a rough guess is OK here.*/
	new_fdt_size = fdt_size + cmdline_size + 0x200;

fdt_alloc_retry:
	status = efi_high_alloc(sys_table, new_fdt_size, 0, &new_fdt_addr,
			    dram_base + ZIMAGE_OFFSET_LIMIT);
	if (status != EFI_SUCCESS) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: Unable to allocate memory for new device tree.\n");
		goto fail_free_initrd;
	}

	/* Now that we have done our final memory allocation (and free)
	 * we can get the memory map key needed
	 * forexit_boot_services.*/
	status = efi_get_memory_map(sys_table, &memory_map, &map_size,
				    &desc_size, &mmap_key);
	if (status != EFI_SUCCESS)
		goto fail_free_new_fdt;

	status = update_fdt(sys_table,
			    fdt, (void *)new_fdt_addr, new_fdt_size,
			    cmdline_ptr,
			    initrd_addr, initrd_size,
			    memory_map, map_size, desc_size);

	if (status != EFI_SUCCESS) {
		if (status == EFI_BUFFER_TOO_SMALL) {
			/* We need to allocate more space for the new
			 * device tree, so free existing buffer that is
			 * too small.  Also free memory map, as we will need
			 * to get new one that reflects the free/alloc we do
			 * on the device tree buffer. */
			efi_free(sys_table, new_fdt_size, new_fdt_addr);
			efi_call_phys1(sys_table->boottime->free_pool,
				       memory_map);
			new_fdt_size += new_fdt_size/4;
			goto fdt_alloc_retry;
		}
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: Unable to constuct new device tree.\n");
		goto fail_free_initrd;
	}

	/* Now we are ready to exit_boot_services.*/
	status = efi_call_phys2(sys_table->boottime->exit_boot_services,
				handle, mmap_key);

	if (status != EFI_SUCCESS) {
		efi_printk(sys_table, PRINTK_PREFIX"ERROR: exit boot services failed.\n");
		goto fail_free_mmap;
	}


	/* Now we need to return the FDT address to the calling
	 * assembly to this can be used as part of normal boot.
	 */
	return new_fdt_addr;

fail_free_mmap:
	efi_call_phys1(sys_table->boottime->free_pool, memory_map);

fail_free_new_fdt:
	efi_free(sys_table, new_fdt_size, new_fdt_addr);

fail_free_initrd:
	efi_free(sys_table, initrd_size, initrd_addr);

fail_free_zimage:
	efi_free(sys_table, zimage_size, *zimage_addr);

fail_free_kernel_reserve:
	efi_free(sys_table, kernel_reserve_addr, kernel_reserve_size);

fail_free_fdt:
	efi_free(sys_table, fdt_size, fdt_addr);

fail_free_cmdline:
	efi_free(sys_table, cmdline_size, (u32)cmdline_ptr);

fail:
	return EFI_STUB_ERROR;
}
