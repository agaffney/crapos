#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define VMM_MIN_FREE_PAGES 2

#define KMALLOC_ATOMIC 1

typedef struct {
	void * virt_addr;
	void * phys_addr;
	uint16_t remain;
} vmm_page;

void vmm_add_free_page(vmm_page * page);

void * kmalloc(size_t, uint8_t);

// These functions should be defined in the arch-specific VMM code
extern void * arch_vmm_get_heap_start();
extern void arch_vmm_map_page(vmm_page * page);
extern void * arch_vmm_next_phys_page();
