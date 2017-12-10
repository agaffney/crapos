#include <stddef.h>
#include <core/kprint.h>
#include <core/vmm.h>
#include <data/linked_list.h>

void * KMALLOC_NEXT_ADDR;

// Pre-allocate initial list and first item to bootstrap us before we have any
// free memory pages available
Linked_List_Item _vmm_free_pages_item;
Linked_List VMM_FREE_PAGES = {
	.first = &_vmm_free_pages_item,
	.last = NULL,
	.length = 0,
};

// TODO: implement an actual allocator that can detect page boundaries
void * kmalloc(size_t len) {
	if (KMALLOC_NEXT_ADDR == NULL) {
		vmm_page * first_page = Linked_List_shift(&VMM_FREE_PAGES);
//		vmm_page * first_page = Linked_List_get(&VMM_FREE_PAGES, 0);
		KMALLOC_NEXT_ADDR = first_page->virt_addr;
		kprint("kmalloc(): first_page->virt_addr=0x%x\n", first_page->virt_addr);
	}
	void * addr = KMALLOC_NEXT_ADDR;
	KMALLOC_NEXT_ADDR += len;
	kprint("kmalloc(): len=%d, addr=0x%x, next_addr=0x%x\n", len, addr, KMALLOC_NEXT_ADDR);
	return addr;
}

void vmm_add_free_page(vmm_page * page) {
	Linked_List_push(&VMM_FREE_PAGES, page);
	int i;
	for (i = 0; i < VMM_FREE_PAGES.length; i++) {
		kprint("%d - 0x%x, phys_addr = 0x%x\n", i, Linked_List_get(&VMM_FREE_PAGES, i), ((vmm_page *)Linked_List_get(&VMM_FREE_PAGES, i))->phys_addr);
	}
}
