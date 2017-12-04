#include <arch/x86/vmm.h>

x86_page_dir * PAGE_DIR;

void vmm_init() {
	// Reuse initial page directory from boot.s to conserve memory
	PAGE_DIR = (x86_page_dir *)&boot_pagedir;
	// Map first 4MB of memory at 0 and 3GB
	PAGE_DIR->page_tables[0] = (x86_page_table *)(((uint32_t)&boot_pagetab1 - KERN_OFFSET) | (PAGE_FLAG_PRESENT + PAGE_FLAG_WRITE));
	PAGE_DIR->page_tables[768] = PAGE_DIR->page_tables[0];
	reload_page_dir();
}

void reload_page_dir() {
	uint32_t val = (uint32_t)PAGE_DIR - KERN_OFFSET;
	asm volatile ("movl %0, %%cr3" :: "r"(val));
}
