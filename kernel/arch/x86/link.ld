ENTRY (_start)

SECTIONS
{
	/* The kernel will live at 3GB + 1MB in the virtual address space, */
	/* which will be mapped to 1MB in the physical address space. */
	/* Note that we page-align the sections. */
	. = 0xC0100000;
	/* Add a symbol that indicates the start address of the kernel. */
	_kernel_start = .;
	.text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000)
	{
		*(.multiboot)
		*(.text)
	}
	.rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
	/*
	 * I have no idea why, but these symbols need to be in their own section, or
	 * we end up with weird offsets that break things
	 */
	.rodata.register_func ALIGN(4K) : AT (ADDR (.rodata.register_func) - 0xC0000000)
	{
		_register_func_vfs_start = .;
		KEEP(*(SORT(.register_func.vfs.*)))
		_register_func_vfs_end = .;
	}
	.data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
	{
		*(.data)
	}
	.bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
		*(.bootstrap_heap)
	}
	/* Add a symbol that indicates the end address of the kernel. */
	_kernel_end = .;
}
