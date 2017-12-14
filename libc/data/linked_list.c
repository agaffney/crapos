#include <stddef.h>
#include <core/kprint.h>
#include <core/vmm.h>
#include <data/linked_list.h>

Linked_List * Linked_List_init(Linked_List * list) {
	list->first = NULL;
	list->last = NULL;
	list->length = 0;
	return list;
}

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
	if (list->length == 0 && list->first != NULL) {
		new_item = list->first;
	} else {
		new_item = kmalloc(sizeof(Linked_List_Item), KMALLOC_ZERO);
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
		data = list->first->data;
		Linked_List_delete(list, 0);
	}
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
