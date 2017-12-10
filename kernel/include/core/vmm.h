#pragma once

#define PAGE_SIZE 4096

typedef struct {
	void * virt_addr;
	void * phys_addr;
} vmm_page;

void vmm_add_free_page(vmm_page * page);
