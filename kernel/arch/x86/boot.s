# Declare constants for the multiboot header.
.set MB_FLAG_ALIGN,   1<<0                             # align loaded modules on page boundaries
.set MB_FLAG_MEMINFO, 1<<1                             # provide memory map
.set MB_FLAGS,        MB_FLAG_ALIGN | MB_FLAG_MEMINFO  # this is the Multiboot 'flag' field
.set MB_MAGIC,        0x1BADB002                       # 'magic number' lets bootloader find the header
.set MB_CHECKSUM,     -(MB_MAGIC + MB_FLAGS)           # checksum of above, to prove we are multiboot

# Declare a multiboot header that marks the program as a kernel.
.section .multiboot
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECKSUM

# Kernel is mapped at 3GB + 1MB
.set KERN_OFFSET, 0xC0000000

# Allocate the initial stack.
.section .bootstrap_stack, "aw", @nobits
bootstrap_stack_bottom:
.skip 16384 # 16 KiB
bootstrap_stack_top:

# Allocate the initial heap
.section .bootstrap_heap, "aw", @nobits
.global _bootstrap_heap_start
_bootstrap_heap_start:
.skip 16384 # 16 KiB
.global _bootstrap_heap_end
_bootstrap_heap_end:

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.section .bss, "aw", @nobits
	.align 4096
.global _boot_pagedir
_boot_pagedir:
	.skip 4096
.global _boot_pagetab1
_boot_pagetab1:
	.skip 4096
# Further page tables may be required if the kernel grows beyond 3 MiB.

# Global variables for multiboot info
.section .data
.global _multiboot_info
_multiboot_info:
.long 0
.global _multiboot_magic_value
_multiboot_magic_value:
.long 0

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	# Disable interrupts
	cli

	# Stick the physical address of the first page table in the destination
	# index register
	movl $(_boot_pagetab1 - KERN_OFFSET), %edi
	# We're mapping up to the first 1024 pages (4MB)
	# This covers the first 1MB, which is normally reserved for x86-ey things,
	# plus up to another 3MB for the kernel. This also makes it easier to get
	# at the multiboot info, which can end up *after* the kernel image in memory.
	movl $0, %esi
	movl $1024, %ecx

1:
	# Copy current page start address into EDX
	movl %esi, %edx
	# Set bits for present/writable
	orl $0x003, %edx
	# Write to page table entry
	movl %edx, (%edi)
	# Increment source index register by page size (4K)
	addl $4096, %esi
	# Increment destination index register by page table entry size (4 bytes)
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop 1b

2:
	# The page table is used at both page directory entry 0 (virtually from 0x0
	# to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
	# 768 (virtually from 0xC0000000 to 0xC03FFFFF) (thus mapping it in the
	# higher half). The kernel is identity mapped because enabling paging does
	# not change the next instruction, which continues to be physical. The CPU
	# would instead page fault if there was no identity mapping.

	# Map the page table to both virtual addresses 0x00000000 and 0xC0000000
	movl $(_boot_pagetab1 - KERN_OFFSET + 0x003), _boot_pagedir - KERN_OFFSET + 0
	movl $(_boot_pagetab1 - KERN_OFFSET + 0x003), _boot_pagedir - KERN_OFFSET + 768 * 4

	# Set cr3 to the address of _boot_pagedir
	movl $(_boot_pagedir - KERN_OFFSET), %ecx
	movl %ecx, %cr3

	# Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	# Jump to higher half with an absolute jump.
	lea _start_higher_half, %ecx
	jmp *%ecx

_start_higher_half:
	# At this point, paging is fully set up and enabled.

	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Set up the stack pointer
	mov $bootstrap_stack_top, %esp

	# Store the pointer to the multiboot info and the magic value
	movl %ebx, _multiboot_info
	movl %eax, _multiboot_magic_value

	# Enter the high-level kernel.
	call kmain

	# Infinite loop if the system has nothing more to do.
	cli
1:	hlt
	jmp 1b
