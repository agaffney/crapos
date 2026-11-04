#include <stddef.h>
#include <stdint.h>

#include <data/linked_list.h>
#include "harness.h"

/* Build a list holding the addresses of a, b and c, in that order. */
#define LIST_OF_THREE(list, a, b, c) \
	Linked_List list = {0}; \
	int a = 1, b = 2, c = 3; \
	Linked_List_push(&list, &a); \
	Linked_List_push(&list, &b); \
	Linked_List_push(&list, &c)

/* Linked_List_init() resets all fields, whatever they held before. */
TEST(list_init_clears_fields)
{
	Linked_List list;
	list.first = (Linked_List_Item *)&list;
	list.last = (Linked_List_Item *)&list;
	list.length = 99;
	ASSERT_TRUE(Linked_List_init(&list) == &list);
	ASSERT_NULL(list.first);
	ASSERT_NULL(list.last);
	ASSERT_EQ_INT(list.length, 0);
}

/* Pushing onto an empty list makes the new item both first and last. */
TEST(list_push_to_empty)
{
	Linked_List list = {0};
	int a = 1;
	Linked_List_push(&list, &a);
	ASSERT_EQ_INT(list.length, 1);
	ASSERT_TRUE(list.first == list.last);
	ASSERT_TRUE(list.first->data == &a);
	ASSERT_NULL(list.first->next);
}

/* Pushed items are appended and can be read back in order with get(). */
TEST(list_push_appends_in_order)
{
	LIST_OF_THREE(list, a, b, c);
	ASSERT_EQ_INT(list.length, 3);
	ASSERT_TRUE(Linked_List_get(&list, 0) == &a);
	ASSERT_TRUE(Linked_List_get(&list, 1) == &b);
	ASSERT_TRUE(Linked_List_get(&list, 2) == &c);
	ASSERT_TRUE(list.last->data == &c);
}

/*
 * Known bug: get() has no bounds check, so an index past the end walks off
 * the list and dereferences NULL. delete() checks its index; get() should
 * return NULL.
 */
XFAIL_TEST(list_get_past_end_returns_null)
{
	LIST_OF_THREE(list, a, b, c);
	ASSERT_NULL(Linked_List_get(&list, 3));
}

/*
 * Known bug (or dead field): Linked_List_Item has a 'head' pointer that
 * push() never sets. This assumes it is meant to point back at the owning
 * list; if it's unused, remove the field and this test instead.
 */
XFAIL_TEST(list_push_sets_item_head)
{
	LIST_OF_THREE(list, a, b, c);
	ASSERT_TRUE(list.first->head == &list);
	ASSERT_TRUE(list.last->head == &list);
}

/* shift() removes and returns the first item. */
TEST(list_shift_removes_first)
{
	LIST_OF_THREE(list, a, b, c);
	ASSERT_TRUE(Linked_List_shift(&list) == &a);
	ASSERT_EQ_INT(list.length, 2);
	ASSERT_TRUE(Linked_List_get(&list, 0) == &b);
	ASSERT_TRUE(list.last->data == &c);
}

/* pop() removes and returns the last item and moves 'last' back. */
TEST(list_pop_removes_last)
{
	LIST_OF_THREE(list, a, b, c);
	ASSERT_TRUE(Linked_List_pop(&list) == &c);
	ASSERT_EQ_INT(list.length, 2);
	ASSERT_TRUE(list.last->data == &b);
	ASSERT_NULL(list.last->next);
}

/* shift() and pop() on an empty list return NULL and change nothing. */
TEST(list_shift_and_pop_on_empty)
{
	Linked_List list = {0};
	ASSERT_NULL(Linked_List_shift(&list));
	ASSERT_NULL(Linked_List_pop(&list));
	ASSERT_EQ_INT(list.length, 0);
}

/* Removing the only item leaves a list that is empty and reusable. */
TEST(list_single_item_shift_empties_list)
{
	Linked_List list = {0};
	int a = 1, b = 2;
	Linked_List_push(&list, &a);
	ASSERT_TRUE(Linked_List_shift(&list) == &a);
	ASSERT_EQ_INT(list.length, 0);
	ASSERT_NULL(list.first);
	ASSERT_NULL(list.last);
	Linked_List_push(&list, &b);
	ASSERT_TRUE(Linked_List_get(&list, 0) == &b);
}

/* Same as above, emptying the list with pop() instead. */
TEST(list_single_item_pop_empties_list)
{
	Linked_List list = {0};
	int a = 1;
	Linked_List_push(&list, &a);
	ASSERT_TRUE(Linked_List_pop(&list) == &a);
	ASSERT_EQ_INT(list.length, 0);
	ASSERT_NULL(list.first);
	ASSERT_NULL(list.last);
}

/* delete() at index 0 drops the head. */
TEST(list_delete_first)
{
	LIST_OF_THREE(list, a, b, c);
	Linked_List_delete(&list, 0);
	ASSERT_EQ_INT(list.length, 2);
	ASSERT_TRUE(Linked_List_get(&list, 0) == &b);
	ASSERT_TRUE(Linked_List_get(&list, 1) == &c);
}

/* delete() in the middle relinks the neighbors. */
TEST(list_delete_middle)
{
	LIST_OF_THREE(list, a, b, c);
	Linked_List_delete(&list, 1);
	ASSERT_EQ_INT(list.length, 2);
	ASSERT_TRUE(Linked_List_get(&list, 0) == &a);
	ASSERT_TRUE(Linked_List_get(&list, 1) == &c);
	ASSERT_TRUE(list.last->data == &c);
}

/* delete() at the last index drops the tail and updates 'last'. */
TEST(list_delete_last)
{
	LIST_OF_THREE(list, a, b, c);
	Linked_List_delete(&list, 2);
	ASSERT_EQ_INT(list.length, 2);
	ASSERT_TRUE(list.last->data == &b);
	ASSERT_NULL(list.last->next);
}

/* delete() with an index past the end is ignored. */
TEST(list_delete_out_of_range_is_ignored)
{
	LIST_OF_THREE(list, a, b, c);
	Linked_List_delete(&list, 3);
	ASSERT_EQ_INT(list.length, 3);
	ASSERT_TRUE(Linked_List_get(&list, 2) == &c);
}

/*
 * Known bug: the range check computes length - 1 on an unsigned value, which
 * wraps to a huge number for an empty list, so delete() goes on to
 * dereference the NULL first item. It should be a no-op.
 */
XFAIL_TEST(list_delete_on_empty_is_a_noop)
{
	Linked_List list = {0};
	Linked_List_delete(&list, 0);
	ASSERT_EQ_INT(list.length, 0);
}

/*
 * The VMM bootstraps its free-page list with a preallocated, empty item
 * (first set, length 0). The first push must use that item, not allocate.
 */
TEST(list_push_reuses_preallocated_first_item)
{
	Linked_List_Item preallocated = {0};
	Linked_List list = {.first = &preallocated, .last = NULL, .length = 0};
	int a = 1, b = 2;

	Linked_List_push(&list, &a);
	ASSERT_TRUE(list.first == &preallocated);
	ASSERT_TRUE(list.last == &preallocated);
	ASSERT_TRUE(preallocated.data == &a);
	ASSERT_EQ_INT(list.length, 1);

	Linked_List_push(&list, &b);
	ASSERT_TRUE(list.last != &preallocated);
	ASSERT_TRUE(preallocated.next == list.last);
	ASSERT_EQ_INT(list.length, 2);
}

/*
 * Known bug (latent): when push() reuses the preallocated item it doesn't
 * clear its 'next' pointer. The kernel's item is static, so it is always
 * zero there, but a dirty item would leave a bogus link in the list.
 */
XFAIL_TEST(list_push_clears_next_of_reused_item)
{
	int junk;
	Linked_List_Item other = {0};
	Linked_List_Item preallocated = {.next = &other};
	Linked_List list = {.first = &preallocated, .last = NULL, .length = 0};

	Linked_List_push(&list, &junk);
	ASSERT_NULL(preallocated.next);
}
