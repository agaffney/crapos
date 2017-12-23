#pragma once

#include <stdint.h>
#include <stddef.h>

// It's necessary to declare these before defining them to break the circular
// dependency between the two structs
struct _Linked_List;
struct _Linked_List_Item;

struct _Linked_List {
	struct _Linked_List_Item * first;
	struct _Linked_List_Item * last;
	uint32_t length;
};

typedef struct _Linked_List Linked_List;

struct _Linked_List_Item {
	struct _Linked_List_Item * next;
	Linked_List * head;
	void * data;
};

typedef struct _Linked_List_Item Linked_List_Item;

void * Linked_List_get(Linked_List * list, uint32_t index);
void Linked_List_push(Linked_List * list, void * item);
void * Linked_List_pop(Linked_List * list);
void * Linked_List_shift(Linked_List * list);
void Linked_List_delete(Linked_List * list, size_t idx);
