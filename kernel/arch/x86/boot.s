# Declare constants for the multiboot header.
.set MB_ALIGN,    1<<0                   # align loaded modules on page boundaries
.set MB_MEMINFO,  1<<1                   # provide memory map
.set MB_FLAGS,    MB_ALIGN | MB_MEMINFO  # this is the Multiboot 'flag' field
.set MB_MAGIC,    0x1BADB002             # 'magic number' lets bootloader find the header
.set MB_CHECKSUM, -(MB_MAGIC + MB_FLAGS) # checksum of above, to prove we are multiboot

# Declare a multiboot header that marks the program as a kernel.
.section .multiboot
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECKSUM

# Kernel is mapped at 3GB
.set KERN_OFFSET, 0xC0000000

# Allocate the initial stack.
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.section .bss, "aw", @nobits
	.align 4096
boot_pagedir:
	.skip 4096
boot_pagetab1:
	.skip 4096
# Further page tables may be required if the kernel grows beyond 3 MiB.

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	# Stick the physical address of the first page table in the destination
	# index register
	movl $(boot_pagetab1 - KERN_OFFSET), %edi
	# We're mapping the first 1024 pages (4MB)
	# This covers the first 1MB, which is normally reserved for x86-ey things,
	# plus another 3MB for the kernel
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

	# The page table is used at both page directory entry 0 (virtually from 0x0
	# to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
	# 768 (virtually from 0xC0000000 to 0xC03FFFFF) (thus mapping it in the
	# higher half). The kernel is identity mapped because enabling paging does
	# not change the next instruction, which continues to be physical. The CPU
	# would instead page fault if there was no identity mapping.

	# Map the page table to both virtual addresses 0x00000000 and 0xC0000000
	movl $(boot_pagetab1 - KERN_OFFSET + 0x003), boot_pagedir - KERN_OFFSET + 0
	movl $(boot_pagetab1 - KERN_OFFSET + 0x003), boot_pagedir - KERN_OFFSET + 768 * 4

	# Set cr3 to the address of the boot_pagedir.
	movl $(boot_pagedir - KERN_OFFSET), %ecx
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

	# Unmap the identity mapping as it is now unnecessary.
	#movl $0, boot_pagedir + 0

	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Set up the stack.
	mov $stack_top, %esp

	# Enter the high-level kernel.
	call kmain

	# Infinite loop if the system has nothing more to do.
	cli
1:	hlt
	jmp 1b
