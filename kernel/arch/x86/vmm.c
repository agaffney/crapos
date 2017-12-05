#include <arch/x86/vmm.h>

x86_page_dir * PAGE_DIR;
void * NEXT_PHYS_PAGE;

void vmm_init() {
	// Reuse initial page directory from boot.s to conserve memory
	PAGE_DIR = (x86_page_dir *)&boot_pagedir;
	// Map first 4MB of memory at 0 and 3GB
	PAGE_DIR->page_tables[0] = (x86_page_table *)(((uint32_t)&boot_pagetab1 - KERN_OFFSET) | (PAGE_FLAG_PRESENT + PAGE_FLAG_WRITE));
	PAGE_DIR->page_tables[768] = PAGE_DIR->page_tables[0];
	reload_page_dir();
	// Set the next available physical page to the location of the
	// _kernel_end symbol
	NEXT_PHYS_PAGE = &_kernel_end - KERN_OFFSET;
	void * foo = map_page();
	char buf[30];
	sprintf(buf, "page addr = %x", foo);
	kprint(buf);
	kprint_newline();
}

void reload_page_dir() {
	uint32_t val = (uint32_t)PAGE_DIR - KERN_OFFSET;
	asm volatile ("movl %0, %%cr3" :: "r"(val));
}

void * get_next_phys_page() {
	void * new_page = NEXT_PHYS_PAGE;
	NEXT_PHYS_PAGE += PAGE_SIZE;
	return new_page;
}

void * map_page() {
	void * new_page = get_next_phys_page();
	uint16_t page_dir_idx = get_page_dir_index(new_page);
	uint16_t page_tbl_idx = get_page_table_index(new_page);
	char buf[20];
	sprintf(buf, "0x%x, 0x%x", page_dir_idx, page_tbl_idx);
	kprint(buf);
	kprint_newline();

	return new_page;
}

uint16_t get_page_dir_index(void * addr) {
	return (uint16_t)((uint32_t)addr / (PAGE_SIZE * PAGE_TABLE_SIZE));
}

uint16_t get_page_table_index(void * addr) {
	return (uint16_t)((uint32_t)addr / PAGE_SIZE % (PAGE_TABLE_SIZE * PAGE_DIR_SIZE));
}
