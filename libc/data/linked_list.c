#include <stddef.h>
#include <core/kprint.h>
#include <data/linked_list.h>

void * Linked_List_get(Linked_List * list, uint32_t index) {
	uint32_t count;
	Linked_List_Item * item = list->first;
	for (count = 0; count < index; count++) {
		item = item->next;
	}
	return item->item;
}

void Linked_List_push(Linked_List * list, void * item) {
	Linked_List_Item * new_item;
	// Use pre-allocated empty first item if present, otherwise allocate
	// memory
	kprint("list->length = %d, list->first = 0x%x\n", list->length, list->first);
	if (list->length == 0 && list->first != NULL) {
		new_item = list->first;
	} else {
		new_item = kmalloc(sizeof(Linked_List_Item));
	}
	kprint("new_item = 0x%x, list->last = 0x%x, list->length = %d\n", new_item, list->last, list->length);
	new_item->item = item;
	if (list->last != NULL) {
		list->last->next = new_item;
	}
	list->last = new_item;
	list->length++;
}
