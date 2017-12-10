#include <stddef.h>
#include <core/kprint.h>
#include <core/vmm.h>
#include <data/linked_list.h>

void * Linked_List_get(Linked_List * list, uint32_t index) {
	uint32_t count;
	Linked_List_Item * item = list->first;
	for (count = 0; count < index; count++) {
		item = item->next;
	}
	return item->data;
}

void Linked_List_push(Linked_List * list, void * data) {
	Linked_List_Item * new_item;
	// Use pre-allocated empty first item if present, otherwise allocate
	// memory
	kprint("list->length = %d, list->first = 0x%x\n", list->length, list->first);
	if (list->length == 0 && list->first != NULL) {
		new_item = list->first;
	} else {
		new_item = kmalloc(sizeof(Linked_List_Item));
	}
	new_item->data = data;
	if (list->first == NULL) {
		list->first = new_item;
	}
	if (list->last != NULL) {
		list->last->next = new_item;
	}
	list->last = new_item;
	list->length++;
	kprint("new_item = 0x%x, list->last = 0x%x, list->length = %d\n", new_item, list->last, list->length);
}

void Linked_List_delete(Linked_List * list, size_t idx) {
	if (idx > (list->length - 1)) {
		return;
	}
	size_t count;
	Linked_List_Item * item = list->first;
	Linked_List_Item * prev_item = NULL;
	for (count = 0; count < idx; count++) {
		prev_item = item;
		item = item->next;
	}
	if (prev_item != NULL) {
		prev_item->next = item->next;
	}
	if (idx == (list->length - 1)) {
		list->last = prev_item;
	}
	if (idx == 0) {
		list->first = item->next;
	}
	list->length--;
}

void * Linked_List_shift(Linked_List * list) {
	void * data = NULL;
	if (list->length > 0) {
		kprint("shift(): list->first=0x%x\n", list->first);
		data = list->first->data;
		Linked_List_delete(list, 0);
	}
	kprint("shift(): data=0x%x, list->length=%d\n", data, list->length);
	return data;
}

void * Linked_List_pop(Linked_List * list) {
	void * data = NULL;
	if (list->length > 0) {
		data = list->last->data;
		Linked_List_delete(list, list->length - 1);
	}
	return data;
}
