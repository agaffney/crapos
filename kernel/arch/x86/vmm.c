#include <arch/x86/vmm.h>
#include <core/kprint.h>
#include <core/vmm.h>

x86_page_dir * PAGE_DIR;
char * NEXT_PHYS_PAGE;

// Pre-allocate the first VMM mapping structs to help bootstrap the page
// allocator
vmm_page initial_pages[4];

void vmm_init() {
	int i;
	// Reuse initial page directory from boot.s to conserve memory
	PAGE_DIR = (x86_page_dir *)&_boot_pagedir;
	// Mark all entries in page directory as writable but not-present
	for (i = 0; i < PAGE_DIR_ENTRIES; i++) {
		PAGE_DIR->page_tables[i] = (x86_page_table *)(0 | PAGE_FLAG_WRITABLE);
	}
	// Map first 4MB of memory at 0 and 3GB
	PAGE_DIR->page_tables[0] = (x86_page_table *)(((uint32_t)&_boot_pagetab1 - KERN_OFFSET) | (PAGE_FLAG_PRESENT + PAGE_FLAG_WRITABLE));
	PAGE_DIR->page_tables[768] = PAGE_DIR->page_tables[0];
	reload_page_dir();
	kdebug("PAGE_DIR->page_tables[768] = 0x%x\n", PAGE_DIR->page_tables[768]);
	// Set the next available physical page to the location of our
	// bootstrap heap
	NEXT_PHYS_PAGE = &_bootstrap_heap_start - KERN_OFFSET;
	kdebug("NEXT_PHYS_PAGE = 0x%x\n", NEXT_PHYS_PAGE);
	// Seed the heap allocator with the physical pages from our bootstrap heap
	uint8_t bootstrap_pages_count = (&_bootstrap_heap_end - &_bootstrap_heap_start) / ARCH_PAGE_SIZE;
	vmm_page * tmp_page;
	for (i = 0; i < bootstrap_pages_count; i++) {
		tmp_page = initial_pages + i;
		tmp_page->phys_addr = arch_vmm_next_phys_page();
		tmp_page->virt_addr = tmp_page->phys_addr + KERN_OFFSET;
		tmp_page->remain = ARCH_PAGE_SIZE;
		kdebug("[%d] tmp_page->phys_addr = 0x%x, tmp_page = 0x%x\n", i, tmp_page->phys_addr, tmp_page);
		vmm_add_free_page(tmp_page);
	}

	// Force allocation of multiple pages (testing)
	for (i = 0; i < 1000; i++) {
		kdebug("allocating chunk %d\n", i);
		uint32_t * foo = kmalloc(sizeof(uint32_t) * 1000, 0);
		foo[42] = 1;
	}
}

void reload_page_dir() {
	kdebug("reloading page dir\n");
	// Put the physical address of the page directory into the CR3 register
	uint32_t val = (uint32_t)PAGE_DIR - KERN_OFFSET;
	asm volatile ("movl %0, %%cr3" :: "r"(val));
}

void * arch_vmm_get_heap_start() {
	return &_bootstrap_heap_start;
}

void * arch_vmm_next_phys_page() {
	void * new_page = NEXT_PHYS_PAGE;
	NEXT_PHYS_PAGE += ARCH_PAGE_SIZE;
	return new_page;
}

void arch_vmm_map_page(vmm_page * page) {
	uint16_t page_dir_idx = get_page_dir_index(page->virt_addr);
	uint16_t page_tbl_idx = get_page_table_index(page->virt_addr);
	kdebug("page->phys_addr=0x%x, page->virt_addr=0x%x, page->remain=%d\n", page->phys_addr, page->virt_addr, page->remain);
	kdebug("page_dir_idx = %d, page_tbl_idx = %d\n", page_dir_idx, page_tbl_idx);
	x86_page_table * page_table = get_page_table(page_dir_idx);
	kdebug("old value = 0x%x\n", page_table->pages[page_tbl_idx]);
	// Allocate a new page table, if needed
	if (page_table == NULL) {
		page_table = kmalloc(sizeof(x86_page_table), KMALLOC_ATOMIC);
		PAGE_DIR->page_tables[page_dir_idx] = (x86_page_table *)((size_t)page_table - KERN_OFFSET + PAGE_FLAG_PRESENT + PAGE_FLAG_WRITABLE);
	}
	page_table->pages[page_tbl_idx] = (void *)( (size_t)page->phys_addr | (PAGE_FLAG_PRESENT + PAGE_FLAG_WRITABLE) );
	kdebug("wrote 0x%x\n", page_table->pages[page_tbl_idx]);
	reload_page_dir();
}

x86_page_table * get_page_table(uint16_t page_dir_idx) {
	return (x86_page_table *)((size_t)PAGE_DIR->page_tables[page_dir_idx] & 0xFFFFF000 + KERN_OFFSET);
}

uint16_t get_page_dir_index(void * addr) {
//	return (uint16_t)((uint32_t)addr / (ARCH_PAGE_SIZE * PAGE_TABLE_ENTRIES));
	return (size_t)addr >> 22;
}

uint16_t get_page_table_index(void * addr) {
//	return (uint16_t)((uint32_t)addr / ARCH_PAGE_SIZE % (PAGE_TABLE_ENTRIES * PAGE_DIR_ENTRIES));
	return (size_t)addr >> 12 & 0x03FF;
}
