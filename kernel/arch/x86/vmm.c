#include <arch/x86/vmm.h>
#include <core/kprint.h>
#include <core/vmm.h>

x86_page_dir * PAGE_DIR;
char * NEXT_PHYS_PAGE;

// Pre-allocate the first VMM mapping struct to help bootstrap the page
// allocator
vmm_page initial_page;

void vmm_init() {
	int i;
	// Reuse initial page directory from boot.s to conserve memory
	PAGE_DIR = (x86_page_dir *)&_boot_pagedir;
	// Mark all entries in page directory as writable but not-present
	for (i = 0; i < PAGE_TABLE_ENTRIES; i++) {
		PAGE_DIR->page_tables[i] = (x86_page_table *)(0 | PAGE_FLAG_WRITABLE);
	}
	// Map first 4MB of memory at 0 and 3GB
	PAGE_DIR->page_tables[0] = (x86_page_table *)(((uint32_t)&_boot_pagetab1 - KERN_OFFSET) | (PAGE_FLAG_PRESENT + PAGE_FLAG_WRITABLE));
	PAGE_DIR->page_tables[768] = PAGE_DIR->page_tables[0];
	reload_page_dir();
	// Set the next available physical page to the location of our
	// bootstrap heap
	NEXT_PHYS_PAGE = &_bootstrap_heap_start - KERN_OFFSET;
	kprint("NEXT_PHYS_PAGE = 0x%x\n", NEXT_PHYS_PAGE);
	// Seed the heap allocator with the first available physical page
	initial_page.phys_addr = get_next_phys_page();
	kprint("initial_page.phys_addr = 0x%x, &initial_page = 0x%x\n", initial_page.phys_addr, &initial_page);
	vmm_add_free_page(&initial_page);
	// Now that we have a page available, we can allocate memory for the rest
	// of the pages in our bootstrap heap
	uint8_t bootstrap_pages_count = (&_bootstrap_heap_end - &_bootstrap_heap_start) / ARCH_PAGE_SIZE;
	vmm_page * tmp_page;
	for (i = 1; i < bootstrap_pages_count; i++) {
/*
		tmp_page = kmalloc(sizeof(vmm_page));
		tmp_page->phys_addr = get_next_phys_page();
		vmm_add_free_page(tmp_page);
*/
		kprint("adding bootstrap page %d\n", i);
	}

}

void reload_page_dir() {
	uint32_t val = (uint32_t)PAGE_DIR - KERN_OFFSET;
	asm volatile ("movl %0, %%cr3" :: "r"(val));
}

void * get_next_phys_page() {
	void * new_page = NEXT_PHYS_PAGE;
	NEXT_PHYS_PAGE += ARCH_PAGE_SIZE;
	return new_page;
}

void * map_page() {
	void * new_page = get_next_phys_page();
	uint16_t page_dir_idx = get_page_dir_index(new_page);
	uint16_t page_tbl_idx = get_page_table_index(new_page);
	kprint("0x%x, 0x%x\n", page_dir_idx, page_tbl_idx);

	return new_page;
}

uint16_t get_page_dir_index(void * addr) {
	return (uint16_t)((uint32_t)addr / (ARCH_PAGE_SIZE * PAGE_TABLE_ENTRIES));
}

uint16_t get_page_table_index(void * addr) {
	return (uint16_t)((uint32_t)addr / ARCH_PAGE_SIZE % (PAGE_TABLE_ENTRIES * PAGE_DIR_ENTRIES));
}
