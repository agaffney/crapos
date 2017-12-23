#include <stddef.h>
#include <core/kprint.h>
#include <core/vmm.h>
#include <libk/data/linked_list.h>

void * KMALLOC_NEXT_ADDR;
vmm_page * KMALLOC_CUR_PAGE;

// Pre-allocate initial list and first item to bootstrap us before we have any
// free memory pages available
Linked_List_Item _vmm_free_pages_item;
Linked_List VMM_FREE_PAGES = {
	.first = &_vmm_free_pages_item,
	.last = NULL,
	.length = 0,
};

void * kmalloc(size_t len, uint8_t flags) {
	if (KMALLOC_CUR_PAGE == NULL) {
		KMALLOC_CUR_PAGE = Linked_List_shift(&VMM_FREE_PAGES);
		KMALLOC_CUR_PAGE->virt_addr = arch_vmm_get_heap_start();
		KMALLOC_NEXT_ADDR = KMALLOC_CUR_PAGE->virt_addr;
	}
	if (KMALLOC_CUR_PAGE->remain < len) {
		KMALLOC_NEXT_ADDR = KMALLOC_CUR_PAGE->virt_addr + PAGE_SIZE;
		KMALLOC_CUR_PAGE = Linked_List_shift(&VMM_FREE_PAGES);
		KMALLOC_CUR_PAGE->virt_addr = KMALLOC_NEXT_ADDR;
		KMALLOC_CUR_PAGE->remain = PAGE_SIZE;
		arch_vmm_map_page(KMALLOC_CUR_PAGE);
		if (VMM_FREE_PAGES.length <= VMM_MIN_FREE_PAGES && !(flags & KMALLOC_ATOMIC)) {
			int i;
			vmm_page * new_page;
			// Add 4 more free pages
			for (i = 0; i < 4; i++) {
				new_page = kmalloc(sizeof(vmm_page), KMALLOC_ZERO & KMALLOC_ATOMIC);
				new_page->phys_addr = arch_vmm_next_phys_page();
				new_page->virt_addr = NULL;
				new_page->remain = PAGE_SIZE;
				Linked_List_push(&VMM_FREE_PAGES, new_page);
			}
		}
	}
	void * addr = KMALLOC_NEXT_ADDR;
	KMALLOC_NEXT_ADDR += len;
	KMALLOC_CUR_PAGE->remain -= len;
	if (flags & KMALLOC_ZERO) {
		int i;
		for (i = 0; i < len; i++) {
			*(char *)((size_t)addr + i) = '\0';
		}
	}
	return addr;
}

void vmm_add_free_page(vmm_page * page) {
	Linked_List_push(&VMM_FREE_PAGES, page);
}
