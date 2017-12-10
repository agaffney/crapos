#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

typedef struct {
	void * virt_addr;
	void * phys_addr;
	uint16_t remain;
} vmm_page;

void vmm_add_free_page(vmm_page * page);

// This function should be defined in the arch-specific VMM code
void arch_vmm_map_page(vmm_page * page);

void * kmalloc(size_t);
