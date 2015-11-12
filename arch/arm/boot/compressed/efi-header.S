@ Copyright (C) 2013 Linaro Ltd;  <roy.franz@linaro.org>
@
@ This file contains the PE/COFF header that is part of the
@ EFI stub.
@

	.org	0x3c
	@
	@ The PE header can be anywhere in the file, but for
	@ simplicity we keep it together with the MSDOS header
	@ The offset to the PE/COFF header needs to be at offset
	@ 0x3C in the MSDOS header.
	@ The only 2 fields of the MSDOS header that are used are this
	@ PE/COFF offset, and the "MZ" bytes at offset 0x0.
	@
	.long	pe_header			@ Offset to the PE header.

      .align 3
pe_header:
	.ascii	"PE"
	.short 	0

coff_header:
	.short	0x01c2				@ ARM or Thumb
	.short	2				@ nr_sections
	.long	0 				@ TimeDateStamp
	.long	0				@ PointerToSymbolTable
	.long	1				@ NumberOfSymbols
	.short	section_table - optional_header	@ SizeOfOptionalHeader
	.short	0x306				@ Characteristics.
						@ IMAGE_FILE_32BIT_MACHINE |
						@ IMAGE_FILE_DEBUG_STRIPPED |
						@ IMAGE_FILE_EXECUTABLE_IMAGE |
						@ IMAGE_FILE_LINE_NUMS_STRIPPED

optional_header:
	.short	0x10b				@ PE32 format
	.byte	0x02				@ MajorLinkerVersion
	.byte	0x14				@ MinorLinkerVersion

	.long	_edata - efi_stub_entry		@ SizeOfCode

	.long	0				@ SizeOfInitializedData
	.long	0				@ SizeOfUninitializedData

	.long	efi_stub_entry			@ AddressOfEntryPoint
	.long	efi_stub_entry			@ BaseOfCode
	.long	0				@ data

extra_header_fields:
	.long	0				@ ImageBase
	.long	0x20				@ SectionAlignment
	.long	0x20				@ FileAlignment
	.short	0				@ MajorOperatingSystemVersion
	.short	0				@ MinorOperatingSystemVersion
	.short	0				@ MajorImageVersion
	.short	0				@ MinorImageVersion
	.short	0				@ MajorSubsystemVersion
	.short	0				@ MinorSubsystemVersion
	.long	0				@ Win32VersionValue

	.long	_edata				@ SizeOfImage

	@ Everything before the entry point is considered part of the header
	.long	efi_stub_entry			@ SizeOfHeaders
	.long	0				@ CheckSum
	.short	0xa				@ Subsystem (EFI application)
	.short	0				@ DllCharacteristics
	.long	0				@ SizeOfStackReserve
	.long	0				@ SizeOfStackCommit
	.long	0				@ SizeOfHeapReserve
	.long	0				@ SizeOfHeapCommit
	.long	0				@ LoaderFlags
	.long	0x0				@ NumberOfRvaAndSizes

	# Section table
section_table:

	#
	# The EFI application loader requires a relocation section
	# because EFI applications must be relocatable.  This is a
	# dummy section as far as we are concerned.
	#
	.ascii	".reloc"
	.byte	0
	.byte	0			@ end of 0 padding of section name
	.long	0
	.long	0
	.long	0			@ SizeOfRawData
	.long	0			@ PointerToRawData
	.long	0			@ PointerToRelocations
	.long	0			@ PointerToLineNumbers
	.short	0			@ NumberOfRelocations
	.short	0			@ NumberOfLineNumbers
	.long	0x42100040		@ Characteristics (section flags)


	.ascii	".text"
	.byte	0
	.byte	0
	.byte	0        		@ end of 0 padding of section name
	.long	_edata - efi_stub_entry		@ VirtualSize
	.long	efi_stub_entry			@ VirtualAddress
	.long	_edata - efi_stub_entry		@ SizeOfRawData
	.long	efi_stub_entry			@ PointerToRawData

	.long	0		@ PointerToRelocations (0 for executables)
	.long	0		@ PointerToLineNumbers (0 for executables)
	.short	0		@ NumberOfRelocations  (0 for executables)
	.short	0		@ NumberOfLineNumbers  (0 for executables)
	.long	0xe0500020	@ Characteristics (section flags)
